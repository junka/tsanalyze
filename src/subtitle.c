#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "types.h"
#include "ts.h"
#include "table.h"
#include "pes.h"
#include "subtitle.h"
#include "utils.h"

#define CHECK_OFFSET(v) off += v; if (off >= 8) {off %= 8; blen += 1; pdata += 1;}

static uint8_t default_2_to_4_bit_map_table[4] = { 0, 0x7, 0x8, 0xF};
static uint8_t default_2_to_8_bit_map_table[4] = { 0, 0x77, 0x88, 0xFF};
static uint8_t default_4_to_8_bit_map_table[16] = {
    0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
};

struct RGBA {
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t T;
};

#if 0
static struct RGBA default_256_cluts[] = {
    
};

static struct RGBA default_16_cluts[] = {
    {.T=0xFF},{.R=0xFF},{.G=0xFF},{.R=0xFF, .G=0xFF},{.B=0xFF},{.R=0xFF, .B=0xFF},{.G=0xFF, .B=0xFF},{.R=0xFF,.G=0xFF, .B=0xFF},
    {.T=0},{.R=0x7F},{.G=0x7F},{.R=0x7F, .G=0x7F},{.B=0x7F},{.R=0x7F, .B=0x7F},{.G=0x7F, .B=0x7F},{.R=0x7F,.G=0x7F, .B=0x7F}
};

static struct RGBA default_4_cluts[] = {
    {.T=0xFF},
    {.R=0xFF, .G=0xFF, .B=0xFF, .T=0},
    {.T=0,},
    {.R=0x7F, .G=0x7F, .B=0x7F, .T=0}
};
#endif

int read_2bit_pixel_code_string(uint8_t *pbuf, int len, uint8_t* data, int *dec_len)
{
    int blen = 0, off = 0;
    uint8_t *pdata = pbuf;
    uint8_t* code = data;
    bool end = false;
    while (!end) {
        uint8_t next_bits = TS_READ8_BITS(pbuf, 2, off);
        if (next_bits != 0) {
            CHECK_OFFSET(2);
            *code++ = next_bits;
        } else {
            CHECK_OFFSET(2);
            uint8_t switch1 = TS_READ8_BITS(pdata, 1, off);
            CHECK_OFFSET(1);
            if (switch1 == 1) {
                next_bits = TS_READ8_BITS(pbuf, 3, off);
                CHECK_OFFSET(3);
                uint8_t value = TS_READ8_BITS(pdata, 2, off);
                CHECK_OFFSET(3);
                for (int n = 0; n < next_bits + 3; n ++) {
                    *code ++ = value;
                }
            } else {
                uint8_t switch2 = TS_READ8_BITS(pdata, 1, off);
                CHECK_OFFSET(1);
                if (switch2 == 0) {
                    uint8_t switch3 = TS_READ8_BITS(pdata, 2, off);
                    CHECK_OFFSET(2);
                    if (switch3 == 2) {
                        next_bits = TS_READ8_BITS(pbuf, 4, off);
                        CHECK_OFFSET(4);
                        uint8_t value = TS_READ8_BITS(pdata, 2, off);
                        CHECK_OFFSET(2);
                        for (int n = 0; n < next_bits + 12; n ++) {
                            *code ++ = value;
                        }
                    } else if (switch3 == 3) {
                        assert(off == 0);
                        next_bits = TS_READ8(pdata);
                        blen += 1;
                        pdata ++;
                        uint8_t value = TS_READ8_BITS(pdata, 2, off);
                        CHECK_OFFSET(2);
                        for (int n = 0; n < next_bits + 29; n ++) {
                            *code ++ = value;
                        }
                    }
                }
            }
        }
    }
    /* byte alignment */
    if (off) {
        blen += 1;
        pdata += 1;
    }
    *dec_len = code - data;
    return blen;
}

int read_4bit_pixel_code_string(uint8_t *pbuf, int len, uint8_t* data, int *dec_len)
{
    int blen = 0, off = 0;
    uint8_t *pdata = pbuf;
    uint8_t* code = data + *dec_len;
    static uint8_t encode[256];
    static int encode_len = 0;
    bool end = false;
    while (!end) {
        uint8_t next_bits = TS_READ16_BITS(pdata, 4, off);
        CHECK_OFFSET(4);
        if (next_bits != 0) {
            encode[encode_len++] = next_bits;
            *code++ = next_bits;
        } else {
            if (encode_len > 2) {
                *code++ = 0;
                *code++ = encode_len;
                for (int n = 0; n < encode_len; n++) {
                    /* absolute mode */
                    *code++ = encode[n];
                }
                /* word wrap */
                for (int n = 0; n < (4 - encode_len%4)%4; n++) {
                    *code++ = 0;
                }
                encode_len = 0;
            } else if (encode_len > 0) {
                for (int n = 0; n < encode_len; n++) {
                    *code++ = 1;
                    *code++ = encode[n];
                }
                encode_len = 0;
            }

            uint8_t switch1 = TS_READ8_BITS(pdata, 1, off);
            CHECK_OFFSET(1);
            if (switch1 == 0) {
                next_bits = TS_READ16_BITS(pdata, 3, off);
                CHECK_OFFSET(3);
                if (next_bits != 0) {
                    /* encode */
                    *code ++ = next_bits+2;
                    *code ++ = 0;
                } else {
                    end = true;
                }
            } else {
                uint8_t switch2 = TS_READ8_BITS(pdata, 1, off);
                CHECK_OFFSET(1);
                if (switch2 == 0) {
                    next_bits = TS_READ16_BITS(pdata, 2, off);
                    CHECK_OFFSET(2);
                    uint8_t value = TS_READ16_BITS(pdata, 4, off);
                    CHECK_OFFSET(4);
                    *code++ = next_bits + 4;
                    *code++ = value;
                } else {
                    uint8_t switch3 = TS_READ16_BITS(pdata, 2, off);
                    CHECK_OFFSET(2);
                    if (switch3 == 2) {
                        next_bits = TS_READ16_BITS(pdata, 4, off);
                        CHECK_OFFSET(4);
                        uint8_t value = TS_READ16_BITS(pdata, 4, off);
                        CHECK_OFFSET(4);
                        *code++ = next_bits + 9;
                        *code++ = value;
                    } else if (switch3 == 3) {
                        next_bits = TS_READ16_BITS(pdata, 8, off);
                        blen += 1;
                        pdata += 1;
                        uint8_t value = TS_READ16_BITS(pdata, 4, off);
                        CHECK_OFFSET(4);
                        *code++ = next_bits + 25;
                        *code++ = value;
                    } else if (switch3 == 0) {
                        *code ++ = 1;
                        *code ++ = 0;
                    } else {
                        *code ++ = 2;
                        *code ++ = 0;
                    }
                }
            }
        }
    }
    /* byte alignment */
    if (off) {
        blen += 1;
        pdata += 1;
    }
    *dec_len = code - data;
    return blen;
}

int read_8bit_pixel_code_string(uint8_t *pbuf, int len, uint8_t* data, int *dec_len)
{
    int blen = 0;
    uint8_t *pdata = pbuf;
    uint8_t* code = data;
    static uint8_t encode[256];
    static int encode_len = 0;
    bool end = false;
    while (!end) {
        uint8_t next_bits = TS_READ8(pdata);
        if (next_bits != 0) {
            encode[encode_len++] = next_bits;
            blen += 1;
            pdata ++;
        } else {
            if (encode_len > 2) {
                *code++ = 0;
                *code++ = encode_len;
                for (int n = 0; n < encode_len; n++) {
                    /* absolute mode */
                    *code++ = encode[n];
                }
                /* word wrap */
                for (int n = 0; n < (4 - encode_len%4)%4; n++) {
                    *code++ = 0;
                }
                encode_len = 0;
            } else if (encode_len > 0) {
                for (int n = 0; n < encode_len; n++) {
                    *code++ = 1;
                    *code++ = encode[n];
                }
                encode_len = 0;
            }

            blen += 1;
            pdata += 1;
            uint8_t switch1 = TS_READ8_BITS(pdata, 1, 0);
            if (switch1 == 0) {
                next_bits = TS_READ8_BITS(pdata, 7, 1);
                if (next_bits != 0) {
                    //run_length_1-127, number of 0x00
                    if (next_bits > 2) {
                        *code = next_bits;
                        *code++ = 0;
                    } else {
                        for (int n = 0; n < encode_len; n++) {
                            *code++ = 1;
                            *code++ = 0;
                        }
                    }
                } else {
                    // end of string signal
                    end = true;
                }
                blen += 1;
                pdata += 1;
            } else {
                // run_length_3-127 must >= 3
                next_bits = TS_READ8_BITS(pdata, 7, 1);
                blen += 1;
                pdata += 1;
                uint8_t value = TS_READ8(pdata);
                *code++ = next_bits;
                *code++ = value;
                blen += 1;
                pdata += 1;
            }
        }
    }
    *dec_len = code - data;
    return blen;
}
#define ONE_LINE_OBJECTS (1024)
int read_pixel_data_sub_block(uint8_t *pbuf, int len, uint8_t *enc_data, int *enc_len)
{
    uint8_t *pdata = pbuf;
    int slen = 0;
    uint8_t *to_data = enc_data+ *enc_len;
    
    uint8_t *mt24 = default_2_to_4_bit_map_table;
    uint8_t *mt28 = default_2_to_8_bit_map_table;
    uint8_t *mt48 = default_4_to_8_bit_map_table;
    uint8_t data_type = TS_READ8(pdata);
    pdata += 1;
    slen += 1;
    // printf("data sub block type 0x%x, len %d\n", data_type, len);
    if (data_type == 0x10) {
        slen += read_2bit_pixel_code_string(pdata, len - 1, enc_data, enc_len);
    } else if (data_type == 0x11) {
        slen += read_4bit_pixel_code_string(pdata, len - 1, enc_data, enc_len);
    } else if (data_type == 0x12) {
        slen += read_8bit_pixel_code_string(pdata, len - 1, enc_data, enc_len);
    } else if (data_type == 0x20) {
        mt24 = calloc(1, 4);
        if (!mt24) {
            return ENOMEM;
        }
        mt24[0] = TS_READ8_BITS(pdata, 4, 0);
        mt24[1] = TS_READ8_BITS(pdata, 4, 4);
        pdata += 1;
        mt24[2] = TS_READ8_BITS(pdata, 4, 0);
        mt24[3] = TS_READ8_BITS(pdata, 4, 4);
        pdata += 1;
        slen += 2;
    } else if (data_type == 0x21) {
        mt28 = calloc(1, 4);
        if (!mt28) {
            return ENOMEM;
        }
        for (int j = 0; j < 4; j++) {
            mt28[j] = TS_READ8(pdata);
            pdata += 1;
        }
        slen += 4;
    } else if (data_type == 0x22) {
        mt48 = calloc(1, 16);
        if (!mt48) {
            return ENOMEM;
        }
        for (int j = 0; j < 16; j++) {
            mt48[j] = TS_READ8(pdata);
            pdata += 1;
        }
        slen += 16;
    } else if (data_type == 0xf0) {
        /* end of line */
        *to_data++ = 0;
        *to_data++ = 0;
        *enc_len += 2;
    }
    return slen;
}


int parse_subtitle_segment_header(uint8_t *pbuf, int len, struct segment_header *h)
{
    uint8_t *pdata = pbuf;
    int hlen = 0;
    h->sync_byte = TS_READ8(pdata);
    assert(h->sync_byte == 0x0F);
    if (h->sync_byte != 0x0F) {
        printf("Must be sync error, drop parsing 0x%x\n", h->sync_byte);
        return -1;
    }
    hlen += 1;
    pdata += 1;
    h->segment_type = TS_READ8(pdata);
    hlen += 1;
    pdata += 1;
    h->page_id = TS_READ16(pdata);
    hlen += 2;
    pdata += 2;
    h->segment_length = TS_READ16(pdata);
    hlen += 2;
    pdata += 2;
    //printf("segment_type %d, page id %d, segment_length %d\n", h->segment_type, h->page_id, h->segment_length);
    return hlen;
}

int parse_sub_segment(uint8_t *pbuf, int len, uint8_t type, struct segment_node *seg)
{
    int slen = 0, i = 0;
    uint8_t *pdata = pbuf;
    struct display_definition_segment *dds;
    struct page_composition_segment *pcs;
    struct region_composition_segment *rcs;
    struct CLUT_definition_segment *cds;
    struct object_data_segment *ods;
    struct disparity_signalling_segment *dss;
    struct alternative_CLUT_segment *acs;
    switch (type) {
        case display_definition_segment:
            dds = calloc(1, sizeof(*dds));
            if (!dds) {
                return ENOMEM;
            }
            seg->segment.data = dds;
            dds->dds_version_number = TS_READ8_BITS(pdata, 4, 0);
            dds->display_window_flag = TS_READ8_BITS(pdata, 1, 4);
            pdata += 1;
            slen += 1;
            dds->display_width = TS_READ16(pdata);
            pdata += 2;
            slen += 2;
            dds->display_height = TS_READ16(pdata);
            pdata += 2;
            slen += 2;
            if (dds->display_window_flag) {
                dds->window.horizontal_position_minimum = TS_READ16(pdata);
                pdata += 2;
                dds->window.horizontal_position_maximum = TS_READ16(pdata);
                pdata += 2;
                dds->window.vertical_position_minimum = TS_READ16(pdata);
                pdata += 2;
                dds->window.vertical_position_maximum = TS_READ16(pdata);
                pdata += 2;
                slen += 8;
            }
            break;
        case page_composition_segment:
            pcs = calloc(1, sizeof(*pcs));
            if (!pcs) {
                return ENOMEM;
            }
            seg->segment.data = pcs;
            pcs->page_time_out = TS_READ8(pdata);
            pdata += 1;
            slen += 1;
            pcs->page_version_number = TS_READ8_BITS(pdata, 4, 0);
            pcs->page_state = TS_READ8_BITS(pdata, 2, 4);
            pdata += 1;
            slen += 1;
            pcs->region = calloc(((seg->segment.header.segment_length - 2) / 6), sizeof(struct region));
            if (!pcs->region) {
                return ENOMEM;
            }
            while (slen < seg->segment.header.segment_length) {
                pcs->region[i].region_id = TS_READ8(pdata);
                /* skip a reserved byte */
                pdata += 2;
                slen += 2;
                pcs->region[i].region_horizontal_address = TS_READ16(pdata);
                pdata += 2;
                slen += 2;
                pcs->region[i].region_vertical_address = TS_READ16(pdata);
                pdata += 2;
                slen += 2;
                i ++;
            }
            pcs->n_region = i;
            break;
        case region_composition_segment:
            rcs = calloc(1, sizeof(*rcs));
            if (!rcs) {
                return ENOMEM;
            }
            seg->segment.data = rcs;
            rcs->region_id = TS_READ8(pdata);
            pdata += 1;
            slen += 1;
            rcs->region_version_number = TS_READ8_BITS(pdata, 4, 0);
            rcs->region_fill_flag = TS_READ8_BITS(pdata, 1, 4);
            pdata += 1;
            slen += 1;
            rcs->region_width = TS_READ16(pdata);
            pdata += 2;
            slen += 2;
            rcs->region_height = TS_READ16(pdata);
            pdata += 2;
            slen += 2;
            rcs->region_level_of_compatibility = TS_READ8_BITS(pdata, 3, 0);
            rcs->region_depth = TS_READ8_BITS(pdata, 3, 3);
            pdata += 1;
            slen += 1;
            rcs->CLUT_id = TS_READ8(pdata);
            pdata += 1;
            slen += 1;
            rcs->region_8bit_pixel_code = TS_READ8(pdata);
            pdata += 1;
            slen += 1;
            rcs->region_4bit_pixel_code = TS_READ8_BITS(pdata, 4, 0);
            rcs->region_2bit_pixel_code = TS_READ8_BITS(pdata, 2, 4);
            pdata += 1;
            slen += 1;
            i = 0;
            while (slen < seg->segment.header.segment_length) {
				struct object *obj = (struct object *)realloc(rcs->object, (i + 1) * sizeof(struct object));
				if (!obj) {
					return ENOMEM;
				}
				rcs->object = obj;
                rcs->object[i].object_id = TS_READ16(pdata);
                pdata += 2;
                slen += 2;
                rcs->object[i].object_type = TS_READ16_BITS(pdata, 2, 0);
                rcs->object[i].object_provider_flag = TS_READ16_BITS(pdata, 2, 2);
                rcs->object[i].object_horizontal_position = TS_READ16_BITS(pdata, 12, 4);
                pdata += 2;
                slen += 2;
                rcs->object[i].object_vertical_position = TS_READ16_BITS(pdata, 12, 4);
                pdata += 2;
                slen += 2;
                if (rcs->object[i].object_type ==0x01 || rcs->object[i].object_type == 0x02) {
                    rcs->object[i].foreground_pixel_code = TS_READ8(pdata);
                    pdata += 1;
                    rcs->object[i].background_pixel_code = TS_READ8(pdata);
                    pdata += 1;
                    slen += 2;
                }
                i ++;
            }
            rcs->n_objs = i;
            break;
        case CLUT_definition_segment:
            cds = calloc(1, sizeof(*cds));
            if (!cds) {
                return ENOMEM;
            }
            seg->segment.data = cds;
            cds->CLUT_id = TS_READ8(pdata);
            pdata += 1;
            slen += 1;
            cds->CLUT_version_number = TS_READ8_BITS(pdata, 4, 0);
            pdata += 1;
            slen += 1;
            i = 0;
            while (slen < seg->segment.header.segment_length) {
                struct clut *cluts = (struct clut*)realloc(cds->cluts, (i + 1) * sizeof(struct clut));
                if (!cluts) {
                    return ENOMEM;
                }
                cds->cluts = cluts;
                cds->cluts[i].entry_id = TS_READ8(pdata);
                pdata += 1;
                slen += 1;
                cds->cluts[i].entry_2bit_CLUT_flag = TS_READ8_BITS(pdata, 1, 0);
                cds->cluts[i].entry_4bit_CLUT_flag = TS_READ8_BITS(pdata, 1, 1);
                cds->cluts[i].entry_8bit_CLUT_flag = TS_READ8_BITS(pdata, 1, 2);
                cds->cluts[i].full_range_flag = TS_READ8_BITS(pdata, 1, 7);
                pdata += 1;
                slen += 1;
                if (cds->cluts[i].full_range_flag) {
                    cds->cluts[i].Y = TS_READ8(pdata);
                    pdata += 1;
                    cds->cluts[i].Cr = TS_READ8(pdata);
                    pdata += 1;
                    cds->cluts[i].Cb = TS_READ8(pdata);
                    pdata += 1;
                    cds->cluts[i].T = TS_READ8(pdata);
                    pdata += 1;
                    slen += 4;
                } else {
                    cds->cluts[i].Y = TS_READ16_BITS(pdata, 6, 0);
                    cds->cluts[i].Cr = TS_READ16_BITS(pdata, 4, 6);
                    cds->cluts[i].Cb = TS_READ16_BITS(pdata, 4, 10);
                    cds->cluts[i].T = TS_READ16_BITS(pdata, 2, 14);
                    pdata += 2;
                    slen += 2;
                }
                i++;
            }
            cds->n_cluts = i;
            break;
        case object_data_segment:
            ods = calloc(1, sizeof(*ods));
            if (!ods) {
                return ENOMEM;
            }
            seg->segment.data = ods;
            ods->object_id = TS_READ16(pdata);
            pdata += 2;
            slen += 2;
            ods->object_version_number = TS_READ8_BITS(pdata, 4, 0);
            ods->object_coding_method = TS_READ8_BITS(pdata, 2, 4);
            ods->non_modifying_colour_flag = TS_READ8_BITS(pdata, 1, 6);
            pdata += 1;
            slen += 1;
            if (ods->object_coding_method == 0) {
                ods->top_field_data_block_length = TS_READ16(pdata);
                pdata += 2;
                slen += 2;
                ods->bottom_field_data_block_length = TS_READ16(pdata);
                pdata += 2;
                slen += 2;
                i = 0;
                int top_len = 0;
                if (ods->top_field_data_block_length) {
                    ods->data_top = calloc(1, ods->top_field_data_block_length * 4);
                    if (!ods->data_top) {
                        return ENOMEM;
                    }
                }
                while (top_len < ods->top_field_data_block_length) {
                    top_len += read_pixel_data_sub_block(pdata + top_len, ods->top_field_data_block_length - top_len, ods->data_top, &ods->top_dec_len);
                    // if (ods->top_sub_block[i].data_type != 0xf0) {
                    i ++;
                }
                // printf("ori len %d, dec_len %d\n", ods->top_field_data_block_length, dec_top_len);

                pdata += top_len;
                slen += top_len;
                assert(top_len == ods->top_field_data_block_length);
                ods->num_top = i;
                int bottom_len = 0;
                i = 0;
                if (ods->bottom_field_data_block_length) {
                    ods->data_bottom = calloc(1, ods->bottom_field_data_block_length * 4);
                    if (!ods->data_bottom) {
                        return ENOMEM;
                    }
                }
                while (bottom_len < ods->bottom_field_data_block_length) {
                    bottom_len += read_pixel_data_sub_block(pdata + bottom_len, ods->bottom_field_data_block_length - bottom_len, ods->data_bottom, &ods->bottom_dec_len);
                    i ++;
                }
                assert(bottom_len == ods->bottom_field_data_block_length);
                pdata += bottom_len;
                slen += bottom_len;
                ods->num_bottom = i;
                assert(seg->segment.header.segment_length - 7 - top_len - bottom_len <= 1);
                /* fast way */
                if ((top_len + bottom_len) % 2 == 0) {
                    slen += 1;
                    pdata += 1;
                }
            } else if (ods->object_coding_method == 1) {
                ods->number_of_codes = TS_READ8(pdata);
                pdata += 1;
                slen += 1;
                ods->character_code = calloc(ods->number_of_codes, 1);
                if (!ods->character_code) {
                    return ENOMEM;
                }
                for (int j = 0; j < ods->number_of_codes; j ++) {
                    ods->character_code[j] = TS_READ8(pdata);
                }
            } else if (ods->object_coding_method == 2) {
                /* progressive_pixel_block */
                ods->progressive_block.bitmap_width = TS_READ16(pdata);
                pdata += 2;
                slen += 2;
                ods->progressive_block.bitmap_height = TS_READ16(pdata);
                pdata += 2;
                slen += 2;
                ods->progressive_block.compressed_data_block_length = TS_READ16(pdata);
                pdata += 2;
                slen += 2;
                ods->progressive_block.compressed_bitmap_data_byte = malloc(ods->progressive_block.compressed_data_block_length);
				if (!ods->progressive_block.compressed_bitmap_data_byte) {
					return ENOMEM;
				}
                for (int j = 0; j < ods->progressive_block.compressed_data_block_length; j ++) {
                    ods->progressive_block.compressed_bitmap_data_byte[j] = TS_READ8(pdata);
                    pdata += 1;
                    slen += 1;
                }
            }
            break;
        case disparity_signalling_segment:
            dss = calloc(1, sizeof(*dss));
            if (!dss) {
                return ENOMEM;
            }
            seg->segment.data = dss;
            dss->dss_version_number = TS_READ8_BITS(pdata, 4, 0);
            dss->disparity_shift_update_sequence_page_flag = TS_READ8_BITS(pdata, 1, 4);
            slen += 1;
            pdata += 1;
            dss->page_default_disparity_shift = TS_READ8(pdata);
            slen += 1;
            pdata += 1;
            if (dss->disparity_shift_update_sequence_page_flag == 1) {
                dss->disparity_shift_update_sequence.disparity_shift_update_sequence_length = TS_READ8(pdata);
                pdata += 1;
                slen += 1;
                dss->disparity_shift_update_sequence.interval_duration = TS_READ32_BITS(pdata, 24, 0);
                dss->disparity_shift_update_sequence.division_period_count = TS_READ32_BITS(pdata, 8, 24);
                pdata += 4;
                slen += 4;
                dss->disparity_shift_update_sequence.preiod = calloc(dss->disparity_shift_update_sequence.division_period_count, sizeof(struct division_period));
                if (!dss->disparity_shift_update_sequence.preiod) {
                    return ENOMEM;
                }
                for (int n = 0; n < dss->disparity_shift_update_sequence.division_period_count; n++) {
                    dss->disparity_shift_update_sequence.preiod[n].interval_count = TS_READ8(pdata);
                    pdata += 1;
                    dss->disparity_shift_update_sequence.preiod[n].disparity_shift_update_integer_part = TS_READ8(pdata);
                    pdata += 1;
                    slen += 2;
                }
            }
            while (slen < seg->segment.header.segment_length) {
                struct disparity_region *regions = (struct disparity_region *)realloc(dss->disparity_regions, (i + 1) * sizeof(struct disparity_region));
				if (!regions) {
                    return ENOMEM;
                }
				dss->disparity_regions = regions;
                dss->disparity_regions[i].region_id = TS_READ8(pdata);
                pdata += 1;
                slen += 1;
                dss->disparity_regions[i].disparity_shift_update_sequence_region_flag = TS_READ8_BITS(pdata, 1, 0);
                dss->disparity_regions[i].number_of_subregions_minus_1 = TS_READ8_BITS(pdata, 1, 6);
                dss->disparity_regions[i].subregions = calloc(dss->disparity_regions[i].number_of_subregions_minus_1 + 1, sizeof(struct sub_region));
                if (!dss->disparity_regions[i].subregions) {
                    return ENOMEM;
                }
                for (int n = 0; n < dss->disparity_regions[i].number_of_subregions_minus_1 + 1; n ++) {
                    if (dss->disparity_regions[i].number_of_subregions_minus_1) {
                        dss->disparity_regions[i].subregions[n].horizontal_position = TS_READ16(pdata);
                        pdata += 2;
                        dss->disparity_regions[i].subregions[n].width = TS_READ16(pdata);
                        pdata += 2;
                        slen += 4;
                    }
                    dss->disparity_regions[i].subregions[n].disparity_shift_integer_part = TS_READ8(pdata);
                    pdata += 1;
                    slen += 1;
                    dss->disparity_regions[i].subregions[n].disparity_shift_fractional_part = TS_READ8_BITS(pdata, 4, 0);
                    pdata += 1;
                    slen += 1;
                    if (dss->disparity_shift_update_sequence_page_flag == 1) {
                        dss->disparity_regions[i].subregions[n].sub_sequence.disparity_shift_update_sequence_length = TS_READ8(pdata);
                        pdata += 1;
                        slen += 1;
                        dss->disparity_regions[i].subregions[n].sub_sequence.interval_duration = TS_READ32_BITS(pdata, 24, 0);
                        dss->disparity_regions[i].subregions[n].sub_sequence.division_period_count = TS_READ32_BITS(pdata, 8, 24);
                        pdata += 4;
                        slen += 4;
                        dss->disparity_regions[i].subregions[n].sub_sequence.preiod = calloc(dss->disparity_regions[i].subregions[n].sub_sequence.division_period_count, sizeof(struct division_period));
                        if (!dss->disparity_regions[i].subregions[n].sub_sequence.preiod) {
                            return ENOMEM;
                        }
                        for (int k = 0; k < dss->disparity_shift_update_sequence.division_period_count; k++) {
                            dss->disparity_regions[i].subregions[n].sub_sequence.preiod[k].interval_count = TS_READ8(pdata);
                            pdata += 1;
                            dss->disparity_regions[i].subregions[n].sub_sequence.preiod[k].disparity_shift_update_integer_part = TS_READ8(pdata);
                            pdata += 1;
                            slen += 2;
                        }
                    }
                }
                i ++;
            }
            dss->n_regions = i;
            break;
        case alternative_CLUT_segment:
            acs = calloc(1, sizeof(*acs));
            if (!acs) {
                return ENOMEM;
            }
            seg->segment.data = acs;
            acs->CLUT_id = TS_READ8(pdata);
            pdata += 1;
            slen += 1;
            acs->CLUT_version_number = TS_READ8_BITS(pdata, 4, 0);
            pdata += 1;
            slen += 1;
            acs->CLUT_parameters.CLUT_entry_max_number = TS_READ8_BITS(pdata, 2, 0);
            acs->CLUT_parameters.colour_component_type = TS_READ8_BITS(pdata, 2, 2);
            acs->CLUT_parameters.output_bit_depth = TS_READ8_BITS(pdata, 3, 4);
            pdata += 1;
            acs->CLUT_parameters.dynamic_range_and_colour_gamut = TS_READ8(pdata);
            pdata += 1;
            slen += 2;
            i = 0;
            while (slen - 4 < seg->segment.header.segment_length) {
                struct aclut *cluts = (struct aclut*)realloc(acs->cluts, (i + 1) * sizeof(struct aclut));
                if (!cluts) {
                    return ENOMEM;
                }
                acs->cluts = cluts;
                if (acs->CLUT_parameters.output_bit_depth == 1) {
                    acs->cluts[i].luma = TS_READ8(pdata);
                    pdata += 1;
                    acs->cluts[i].chroma1 = TS_READ8(pdata);
                    pdata += 1;
                    acs->cluts[i].chroma2 = TS_READ8(pdata);
                    pdata += 1;
                    acs->cluts[i].T = TS_READ8(pdata);
                    pdata += 1;
                    slen += 4;
                } else {
                    acs->cluts[i].luma = TS_READ16_BITS(pdata, 10, 0);
                    pdata += 1;
                    acs->cluts[i].chroma1 = TS_READ16_BITS(pdata, 10, 2);
                    pdata += 1;
                    acs->cluts[i].chroma2 = TS_READ16_BITS(pdata, 10, 4);
                    pdata += 1;
                    acs->cluts[i].T = TS_READ16_BITS(pdata, 10, 2);
                    pdata += 2;
                    slen += 5;
                }
                i ++;
            }
            acs->n_cluts = i;
            break;
        case end_of_display_set_segment:
            /* only header here */
            // printf("end of display\n");
            break;
        default:
            printf("have unsupported segment type, we should not go here\n");
            slen += seg->segment.header.segment_length;
            break;
    }
    return slen;
}

int parse_subtitle(uint16_t pid, uint8_t *pbuf, int len, void *arg)
{
    uint8_t *pdata = pbuf;
    int seg_len = len;
    struct subtitle_pes_data *subtitle = (struct subtitle_pes_data *)arg;
    if (!list_empty(&subtitle->seg_list)) {
        free_subtitles(subtitle);
        list_head_init(&subtitle->seg_list);
    }
    subtitle->data_identifier = TS_READ8(pdata);
    pdata += 1;
    subtitle->subtitle_stream_id = TS_READ8(pdata);
    pdata += 1;
    seg_len -= 2;
    if (subtitle->data_identifier != 0x20 ||
        subtitle->subtitle_stream_id != 0x00) {
        printf("PID 0x%x not a valid DVB subtitle 0x%x, 0x%x\n", pid,
             subtitle->data_identifier, subtitle->subtitle_stream_id);
        return -1;
    }
    uint8_t next_bits = TS_READ8(pdata);
    while(next_bits == SUBTITLE_SYNC_BYTE && seg_len) {
        struct segment_node *seg = calloc(1, sizeof(struct segment_node));
        if (!seg) {
            return ENOMEM;
        }
        int slen = parse_subtitle_segment_header(pdata, seg_len, &seg->segment.header);
        assert(slen == 6);
        seg_len -= slen;
        pdata += slen;
        slen = parse_sub_segment(pdata, seg->segment.header.segment_length, seg->segment.header.segment_type, seg);
        assert(slen == seg->segment.header.segment_length);
        seg_len -= slen;
        pdata += slen;
        list_add_tail(&subtitle->seg_list, &seg->node);
        
        next_bits = TS_READ8(pdata);
    }

    return 0;
}




#pragma pack(push,2)

struct bmp_header {
    //bmp file header
    uint16_t file_type;
    uint32_t file_size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;

    //bmp info head
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};
#pragma pack(pop)


enum bmp_compression {
    BI_RGB = 0,
    BI_RLE8 = 1,
    BI_RLE4 = 2,
};

uint8_t* alloc_bmp_head(uint8_t bits, int len, int w, int h)
{
    struct bmp_header *bmp_p = malloc(54);
	if (!bmp_p) {
		return NULL;
	}

    bmp_p->file_type = 0x4D42;
    bmp_p->file_size = 54+len;
    bmp_p->reserved1 = 0x0;
    bmp_p->reserved2 = 0x0;
    bmp_p->offset = 0x36;

    //bmp info head
    bmp_p->biSize = 0x28;
    bmp_p->biWidth = w;
    bmp_p->biHeight = -h;
    bmp_p->biPlanes = 1;
    if (bits == 4 || bits == 8) {
        bmp_p->biBitCount = 8;
        bmp_p->biCompression = BI_RLE8;
        bmp_p->biSizeImage = len;
        bmp_p->biClrUsed = 1 << bits;
        bmp_p->biClrImportant = 1 << bits;
    }
    bmp_p->biXPelsPerMeter = 0x60;
    bmp_p->biYPelsPerMeter = 0x60;
    
    return (uint8_t *)bmp_p;
}

const char *subtitle_get_type(uint8_t type)
{
    const char *seg_types[] = {
        [page_composition_segment] = "page_composition_segment",
        [region_composition_segment] = "region_composition_segment",
        [CLUT_definition_segment] = "CLUT_definition_segment",
        [object_data_segment] = "object_data_segment",
        [display_definition_segment] = "display_definition_segment",
        [disparity_signalling_segment] = "disparity_signalling_segment",
        [alternative_CLUT_segment] = "alternative_CLUT_segment",
        /* 0x17 - 0x7F reserved */
        [end_of_display_set_segment] = "end_of_display_set_segment",
    };
    return seg_types[type];
}

void YCbCr_to_RGB(uint8_t y, uint8_t cb, uint8_t cr, uint8_t *r, uint8_t *g, uint8_t *b)
{
    double Y = (double) y;
    double Cb = (double) cb;
    double Cr = (double) cr;

    int R = (int) (Y + 1.40200 * (Cr - 0x80));
    int G = (int) (Y - 0.34414 * (Cb - 0x80) - 0.71414 * (Cr - 0x80));
    int B = (int) (Y + 1.77200 * (Cb - 0x80));

    *r = MAX(0, MIN(255, R));
    *g = MAX(0, MIN(255, G));
    *b = MAX(0, MIN(255, B));
}

void dump_subtitle_bitmap(const char* title, uint8_t* cluts, uint8_t *data, int len, int width, int height, int depth)
{
    char name[128];
    snprintf(name, 128,  "%s.bmp", title);
    FILE * fd = fopen(name, "w");

    uint8_t *file_header = alloc_bmp_head(depth, len, width, height);
    //write file header
    fwrite(file_header, 54, 1, fd);
    //write CLUTS
    fwrite(cluts, 1<<depth, 4, fd);
    //write RLE encoded ods
    fwrite(data, len, 1, fd);


    fclose(fd);
    free(file_header);
}

void dump_subtitles(struct subtitle_pes_data *sub)
{
    struct segment_node *seg = NULL;
    rout(2, "subtitle", NULL);
    list_for_each(&sub->seg_list, seg, node) {
        rout(3, "segment_type", "%s", subtitle_get_type(seg->segment.header.segment_type));
        rout(3, "page_id", "%d", seg->segment.header.page_id);
        rout(3, "segment_length", "%d", seg->segment.header.segment_length);
        if (seg->segment.header.segment_type == CLUT_definition_segment) {
            struct CLUT_definition_segment *cds = seg->segment.data;
            rout(4, "CLUT_id", "%d", cds->CLUT_id);
            rout(4, "CLUT_version_number", "%d", cds->CLUT_version_number);
            for (int i = 0; i < cds->n_cluts; i ++) {
                rout(5, "entry_id", "%d", cds->cluts[i].entry_id);
                if (cds->cluts[i].entry_2bit_CLUT_flag) {
                    rout(5, "clut_bit_depth", "2");
                }
                if (cds->cluts[i].entry_4bit_CLUT_flag) {
                    rout(5, "clut_bit_depth", "4");
                }
                if (cds->cluts[i].entry_8bit_CLUT_flag) {
                    rout(5, "clut_bit_depth", "8");
                }
                rout(5, "color", "(Y %d Cr %d Cb %d T %d)", cds->cluts[i].Y, cds->cluts[i].Cr, cds->cluts[i].Cb,  cds->cluts[i].T);
            }
        } else if (seg->segment.header.segment_type == region_composition_segment) {
            struct region_composition_segment *rcs = seg->segment.data;
            rout(4, "region_id", "%d", rcs->region_id);
            rout(4, "region_version_number", "%d", rcs->region_version_number);
            rout(4, "region_fill_flag", "%d", rcs->region_fill_flag);
            rout(4, "region_width", "%d", rcs->region_width);
            rout(4, "region_height", "%d", rcs->region_height);
            rout(4, "region_level_of_compatibility", "%d", rcs->region_level_of_compatibility);
            rout(4, "region_depth", "%d", rcs->region_depth);
            rout(4, "CLUT_id", "%d", rcs->CLUT_id);
            rout(4, "region_8bit_pixel_code", "0x%x", rcs->region_8bit_pixel_code);
            rout(4, "region_4bit_pixel_code", "0x%x", rcs->region_4bit_pixel_code);
            rout(4, "region_2bit_pixel_code", "0x%x", rcs->region_2bit_pixel_code);
            rout(4, "n_objs", "%d", rcs->n_objs);
            for (int i = 0; i < rcs->n_objs; i++) {
                rout(5, "object_id", "%d", rcs->object[i].object_id);
                rout(5, "object_type", "%d", rcs->object[i].object_type);
                rout(5, "object_horizontal_position", "%d", rcs->object[i].object_horizontal_position);
                rout(5, "object_vertical_position", "%d", rcs->object[i].object_vertical_position);
                rout(5, "foreground_pixel_code", "%d", rcs->object[i].foreground_pixel_code);
                rout(5, "background_pixel_code", "%d", rcs->object[i].background_pixel_code);
            }
        } else if (seg->segment.header.segment_type == object_data_segment) {
            struct object_data_segment *ods = seg->segment.data;
            rout(4, "object_id", "%d", ods->object_id);
            rout(4, "object_version_number", "%d", ods->object_version_number);
            rout(4, "object_coding_method", "%d", ods->object_coding_method);
            rout(4, "non_modifying_colour_flag", "%d", ods->non_modifying_colour_flag);
            if (ods->object_coding_method == 0) {
                rout(5, "top_field_data_block_length", "%d", ods->top_field_data_block_length);
                rout(5, "bottom_field_data_block_length", "%d", ods->bottom_field_data_block_length);
                rout(5, "num_top", "%d", ods->num_top);
                // dump_subtitle_bitmap("topxxx", NULL, ods->data_top, ods->top_dec_len, 720, 36, 4);
                rout(5, "num_bottom", "%d", ods->num_bottom);
            } else if (ods->object_coding_method == 1) {
                rout(5, "number_of_codes", "%d", ods->number_of_codes);
                for (int i = 0; i < ods->number_of_codes; i ++) {
                    rout(6, "code", "0x%x", ods->character_code[i]);
                }
            } else if (ods->object_coding_method == 2) {
                rout(5, "bitmap_width", "%d", ods->progressive_block.bitmap_width);
                rout(5, "bitmap_height", "%d", ods->progressive_block.bitmap_height);
                rout(5, "compressed_data_block_length", "%d", ods->progressive_block.compressed_data_block_length);
            }
        } else if (seg->segment.header.segment_type == page_composition_segment) {
            struct page_composition_segment *pcs = seg->segment.data;
            rout(4, "page_time_out", "%d", pcs->page_time_out);
            rout(4, "page_version_number", "%d", pcs->page_version_number);
            rout(4, "page_state", "%d", pcs->page_state);
            rout(4, "n_region", "%d", pcs->n_region);
            for (int i = 0; i < pcs->n_region; i++) {
                rout(5, "region_id", "%d", pcs->region[i].region_id);
                rout(5, "region_horizontal_address", "%d", pcs->region[i].region_horizontal_address);
                rout(5, "region_vertical_address", "%d", pcs->region[i].region_vertical_address);
            }
        }
    }
}


void free_subtitles(struct subtitle_pes_data *sub)
{
    struct segment_node *seg = NULL, *next = NULL;
    list_for_each_safe(&sub->seg_list, seg, next, node) {
        list_del(&(seg->node));
        if (seg->segment.header.segment_type == page_composition_segment) {
            struct page_composition_segment *pcs = seg->segment.data;
            if (pcs->region) {
                free (pcs->region);
            }
        } else if (seg->segment.header.segment_type == region_composition_segment) {
            struct region_composition_segment *rcs = seg->segment.data;
            if (rcs->object) {
                free(rcs->object);
            }
        } else if (seg->segment.header.segment_type == CLUT_definition_segment) {
            struct CLUT_definition_segment *cds = seg->segment.data;
            if (cds->cluts) {
                free(cds->cluts);
            }
        } else if (seg->segment.header.segment_type == object_data_segment) {
            struct object_data_segment *ods = seg->segment.data;
            if (ods->object_coding_method == 0) {
                if (ods->data_top) {
                    free(ods->data_top);
                }
                if (ods->data_bottom) {
                    free(ods->data_bottom);
                }
                // if (ods->top_sub_block) {
                //     for (int i =0; i < ods->num_top; i ++) {
                //         if (ods->top_sub_block[i].data) {
                //             free(ods->top_sub_block[i].data);
                //         }
                //     }
                //     free(ods->top_sub_block);
                // }
                // if (ods->bottom_sub_block) {
                //     for (int i =0; i < ods->num_bottom; i ++) {
                //         if (ods->bottom_sub_block[i].data)
                //             free(ods->bottom_sub_block[i].data);
                //     }
                //     free(ods->bottom_sub_block);
                // }
        } else if (ods->object_coding_method == 1) {
                if (ods->character_code) {
                    free(ods->character_code);
                }
            } else if (ods->object_coding_method == 2) {
                if (ods->progressive_block.compressed_bitmap_data_byte) {
                    free(ods->progressive_block.compressed_bitmap_data_byte);
                }
            }

        } else if (seg->segment.header.segment_type == disparity_signalling_segment) {
            struct disparity_signalling_segment *dss = seg->segment.data;
            if (dss->disparity_regions) {
                for (int i = 0; i < dss->n_regions; i++) {
                    if (dss->disparity_regions[i].subregions) {
                        free(dss->disparity_regions[i].subregions);
                    }
                }
                free(dss->disparity_regions);
            }
        } else if (seg->segment.header.segment_type == alternative_CLUT_segment) {
            struct alternative_CLUT_segment *acs = seg->segment.data;
            if (acs->cluts)
                free(acs->cluts);
        }
        free (seg->segment.data);
        free (seg);
    }
}
