#include <stdint.h>
#include <stdio.h>

#include "types.h"
#include "ts.h"
#include "table.h"
#include "pes.h"
#include "subtitle.h"

#define CHECK_OFFSET(v) off += v; if (off >= 8) {off %= 8; blen += 1; pdata += 1;}

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
    uint8_t* code = data;
    bool end = false;
    while (!end) {
        uint8_t next_bits = TS_READ16_BITS(pdata, 4, off);
        CHECK_OFFSET(4);
        if (next_bits != 0) {
            *code++ = next_bits;
        } else {
            uint8_t switch1 = TS_READ8_BITS(pdata, 1, off);
            CHECK_OFFSET(1);
            if (switch1 == 0) {
                next_bits = TS_READ16_BITS(pdata, 3, off);
                CHECK_OFFSET(3);
                if (next_bits != 0) {
                    for (int n = 0; n < next_bits + 2; n ++) {
                        *code ++ = next_bits;
                    }
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
                    for (int n = 0; n < next_bits + 4; n ++) {
                        *code++ = value;
                    }
                } else {
                    uint8_t switch3 = TS_READ16_BITS(pdata, 2, off);
                    CHECK_OFFSET(2);
                    if (switch3 == 2) {
                        next_bits = TS_READ16_BITS(pdata, 4, off);
                        CHECK_OFFSET(4);
                        uint8_t value = TS_READ16_BITS(pdata, 4, off);
                        CHECK_OFFSET(4);
                        for (int n = 0; n < next_bits + 9; n ++) {
                            *code++ = value;
                        }
                    } else if (switch3 == 3) {
                        next_bits = TS_READ16_BITS(pdata, 8, off);
                        blen += 1;
                        pdata += 1;
                        uint8_t value = TS_READ16_BITS(pdata, 4, off);
                        CHECK_OFFSET(4);
                        for (int n = 0; n < next_bits + 25; n ++) {
                            *code++ = value;
                        }
                    } else if (switch3 == 0) {
                        *code ++ = 0;
                    } else {
                        *code ++ = 0;
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
    bool end = false;
    while (!end) {
        uint8_t next_bits = TS_READ8(pdata);
        if (next_bits != 0) {
            *code++ = next_bits;
            blen += 1;
            pdata ++;
        } else {
            blen += 1;
            pdata += 1;
            uint8_t switch1 = TS_READ8_BITS(pdata, 1, 0);
            if (switch1 == 0) {
                next_bits = TS_READ8_BITS(pdata, 7, 1);
                if (next_bits != 0) {
                    //run_length_1-127, number of 0x00
                    for (int n = 0; n < next_bits; n ++) {
                        *code++ = 0;
                    }
                } else {
                    // end of string signal
                    end = true;
                }
                blen += 1;
                pdata += 1;
            } else {
                // run_length_3-127
                next_bits = TS_READ8_BITS(pdata, 7, 1);
                blen += 1;
                pdata += 1;
                uint8_t value = TS_READ8(pdata);
                for (int n = 0; n < next_bits; n++) {
                    *code++ = value;
                }
                blen += 1;
                pdata += 1;
            }
        }
    }
    *dec_len = code - data;
    return blen;
}
#define ONE_LINE_OBJECTS (1024)
int read_pixel_data_sub_block(uint8_t *pbuf, int len, struct data_sub_block *block)
{
    uint8_t *pdata = pbuf;
    int slen = 0;

    block->data_type = TS_READ8(pdata);
    pdata += 1;
    slen += 1;
    // printf("data sub block type 0x%x, len %d\n", block->data_type, len);
    if (block->data_type == 0x10) {
        block->data = calloc(1, ONE_LINE_OBJECTS);
        slen += read_2bit_pixel_code_string(pdata, len - 1, block->data,
                                                &block->data_len);
    } else if (block->data_type == 0x11) {
        block->data =  calloc(1, ONE_LINE_OBJECTS);
        slen += read_4bit_pixel_code_string(pdata, len - 1, block->data,
                                                &block->data_len);
    } else if (block->data_type == 0x12) {
        block->data =  calloc(1, ONE_LINE_OBJECTS);
        slen += read_8bit_pixel_code_string(pdata, len - 1, block->data,
                                                &block->data_len);
    } else if (block->data_type == 0x20) {
        block->data = malloc(2);
        for (int j = 0; j < 2; j++) {
            block->data[j] = TS_READ8(pdata);
            pdata += 1;
        }
        slen += 2;
    } else if (block->data_type == 0x21) {
        block->data = malloc(4);
        for (int j = 0; j < 4; j++) {
            block->data[j] = TS_READ8(pdata);
            pdata += 1;
        }
        slen += 4;
    } else if (block->data_type == 0x22) {
        block->data = malloc(16);
        for (int j = 0; j < 16; j++) {
            block->data[j] = TS_READ8(pdata);
            pdata += 1;
        }
        slen += 16;
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
            seg->segment.data = pcs;
            pcs->page_time_out = TS_READ8(pdata);
            pdata += 1;
            slen += 1;
            pcs->page_version_number = TS_READ8_BITS(pdata, 4, 0);
            pcs->page_state = TS_READ8_BITS(pdata, 2, 4);
            pdata += 1;
            slen += 1;
            pcs->region = calloc(((seg->segment.header.segment_length - 2) / 6), sizeof(struct region));
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
                rcs->object = realloc(rcs->object, (i+1) * sizeof(struct object));
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
            break;
        case CLUT_definition_segment:
            cds = calloc(1, sizeof(*cds));
            seg->segment.data = cds;
            cds->CLUT_id = TS_READ8(pdata);
            pdata += 1;
            slen += 1;
            cds->CLUT_version_number = TS_READ8_BITS(pdata, 4, 0);
            pdata += 1;
            slen += 1;
            i = 0;
            while (slen < seg->segment.header.segment_length) {
                cds->cluts = realloc(cds->cluts, (i + 1) * sizeof(struct clut));
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
            }
            break;
        case object_data_segment:
            ods = calloc(1, sizeof(*ods));
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
                while (top_len < ods->top_field_data_block_length) {
                    ods->top_sub_block = realloc(ods->top_sub_block, (i + 1) * sizeof(struct data_sub_block));
                    top_len += read_pixel_data_sub_block(pdata + top_len, ods->top_field_data_block_length - top_len, ods->top_sub_block + i);
                    if (ods->top_sub_block[i].data_type != 0xf0) {
                        /* skip end of line */
                        i ++;
                    }
                }
                pdata += top_len;
                slen += top_len;
                assert(top_len == ods->top_field_data_block_length);
                ods->num_top = i;
                int bottom_len = 0;
                i = 0;
                while (bottom_len < ods->bottom_field_data_block_length) {
                    ods->bottom_sub_block = realloc(ods->bottom_sub_block, (i + 1)*sizeof(struct data_sub_block));
                    bottom_len += read_pixel_data_sub_block(pdata + bottom_len, ods->bottom_field_data_block_length - bottom_len, ods->bottom_sub_block + i);
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
                for (int j = 0; j < ods->progressive_block.compressed_data_block_length; j ++) {
                    ods->progressive_block.compressed_bitmap_data_byte[j] = TS_READ8(pdata);
                    pdata += 1;
                    slen += 1;
                }
            }
            break;
        case disparity_signalling_segment:
            dss = calloc(1, sizeof(*dss));
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
                for (int n = 0; n < dss->disparity_shift_update_sequence.division_period_count; n++) {
                    dss->disparity_shift_update_sequence.preiod[n].interval_count = TS_READ8(pdata);
                    pdata += 1;
                    dss->disparity_shift_update_sequence.preiod[n].disparity_shift_update_integer_part = TS_READ8(pdata);
                    pdata += 1;
                    slen += 2;
                }
            }
            while (slen < seg->segment.header.segment_length) {
                dss->disparity_regions = realloc(dss->disparity_regions, (i + 1) * sizeof(struct disparity_region));
                dss->disparity_regions[i].region_id = TS_READ8(pdata);
                pdata += 1;
                slen += 1;
                dss->disparity_regions[i].disparity_shift_update_sequence_region_flag = TS_READ8_BITS(pdata, 1, 0);
                dss->disparity_regions[i].number_of_subregions_minus_1 = TS_READ8_BITS(pdata, 1, 6);
                dss->disparity_regions[i].subregions = calloc(dss->disparity_regions[i].number_of_subregions_minus_1 + 1, sizeof(struct sub_region));
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
                acs->cluts = realloc(acs->cluts, (i + 1) * sizeof(struct aclut));
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
            printf("have unsupport segment type, we should not go here\n");
            slen += seg->segment.header.segment_length;
            break;
    }
    return slen;
}

int parse_subtitle(uint16_t pid, uint8_t *pbuf, int len)
{
    uint8_t *pdata = pbuf;
    int seg_len = len;
    struct subtitle_pes_data subtitle;
    list_head_init(&subtitle.seg_list);
    subtitle.data_identifier = TS_READ8(pdata);
    pdata += 1;
    subtitle.subtitle_stream_id = TS_READ8(pdata);
    pdata += 1;
    seg_len -= 2;
    if (subtitle.data_identifier != 0x20 ||
        subtitle.subtitle_stream_id != 0x00) {
        printf("PID 0x%x not a valid DVB subtitle 0x%x, 0x%x\n", pid,
             subtitle.data_identifier, subtitle.subtitle_stream_id);
        return -1;
    }
    uint8_t next_bits = TS_READ8(pdata);
    while(next_bits == SUBTITLE_SYNC_BYTE && seg_len) {
        struct segment_node *seg = calloc(1, sizeof(struct segment_node));
        int slen = parse_subtitle_segment_header(pdata, seg_len, &seg->segment.header);
        assert(slen == 6);
        seg_len -= slen;
        pdata += slen;
        slen = parse_sub_segment(pdata, seg->segment.header.segment_length, seg->segment.header.segment_type, seg);
        assert(slen == seg->segment.header.segment_length);
        seg_len -= slen;
        pdata += slen;
        list_add(&subtitle.seg_list, &seg->node);
        
        next_bits = TS_READ8(pdata);
    }

    return 0;
}
