#ifndef _SUBTITLE_H_
#define _SUBTITLE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


enum Segment_Type {
    page_composition_segment = 0x10,
    region_composition_segment = 0x11,
    CLUT_definition_segment = 0x12,
    object_data_segment = 0x13,
    display_definition_segment = 0x14,
    disparity_signalling_segment = 0x15,
    alternative_CLUT_segment = 0x16,
    /* 0x17 - 0x7F reserved */
    end_of_display_set_segment = 0x80,
    /* 0x81 - 0xEF private data */
    /* 0xFF stuffing */
};

struct segment_header {
    uint8_t sync_byte;
    uint8_t segment_type;
    uint16_t page_id;
    uint16_t segment_length;
    uint8_t data[0];
};

/* Table 8-1 in EN 300743 */
struct display_definition_segment {
    uint8_t dds_version_number : 4;
    uint8_t display_window_flag: 1;
    uint8_t reserved : 3;
    uint16_t display_width;
    uint16_t display_height;

    struct display_window {
        uint16_t horizontal_position_minimum;
        uint16_t horizontal_position_maximum;
        uint16_t vertical_position_minimum;
        uint16_t vertical_position_maximum;
    } window;
};

/* Table 9 in EN 300743 */
struct page_composition_segment {
    uint8_t page_time_out;
    uint8_t page_version_number: 4;
    uint8_t page_state : 2;
    uint8_t reserved : 2;

    int n_region;
    struct region {
        uint8_t region_id;
        uint8_t reserved;
        uint16_t region_horizontal_address;
        uint16_t region_vertical_address;
    } *region;
};

/* Table 11 in EN 300743 */
struct region_composition_segment {
    uint8_t region_id;
    uint8_t region_version_number: 4;
    uint8_t region_fill_flag :1;
    uint8_t reserved : 3;
    uint16_t region_width;
    uint16_t region_height;
    uint8_t region_level_of_compatibility: 3;
    uint8_t region_depth : 3;
    uint8_t reserved1: 2;
    uint8_t CLUT_id;
    uint8_t region_8bit_pixel_code;
    uint8_t region_4bit_pixel_code: 4;
    uint8_t region_2bit_pixel_code: 2;
    uint8_t reserved2: 2;

    struct object {
        uint16_t object_id;
        uint16_t object_type:2;
        uint16_t object_provider_flag:2;
        uint16_t object_horizontal_position:12;
        uint16_t reserved:4;
        uint16_t object_vertical_position:12;
        uint8_t foreground_pixel_code;
        uint8_t background_pixel_code;
    } *object;
};

struct CLUT_definition_segment {
    uint8_t CLUT_id;
    uint8_t CLUT_version_number: 4;
    uint8_t reserved : 4;

    struct clut {
        uint8_t entry_id;
        uint8_t entry_2bit_CLUT_flag:1;
        uint8_t entry_4bit_CLUT_flag:1;
        uint8_t entry_8bit_CLUT_flag:1;
        uint8_t reserved:4;
        uint8_t full_range_flag : 1;
        uint8_t Y;
        uint8_t Cr;
        uint8_t Cb;
        uint8_t T;
    } *cluts;
};

struct data_sub_block {
    uint8_t data_type;
    int data_len;
    uint8_t *data;
};

struct progressive_pixel_block {
    uint16_t bitmap_width;
    uint16_t bitmap_height;
    uint16_t compressed_data_block_length;
    uint8_t *compressed_bitmap_data_byte;
};

struct object_data_segment {
    uint8_t object_id;
    uint8_t object_version_number: 4;
    uint8_t object_coding_method : 2;
    uint8_t non_modifying_colour_flag : 1;
    uint8_t reserved : 1;

    //TODO
    union {
        struct {
            uint16_t top_field_data_block_length;
            uint16_t bottom_field_data_block_length;
            int num_top;
            struct data_sub_block *top_sub_block;
            int num_bottom;
            struct data_sub_block *bottom_sub_block;
        };
        struct {
            uint8_t number_of_codes;
            uint16_t* character_code;
        };

        struct progressive_pixel_block progressive_block;
    };
};

struct disparity_shift_update_sequence {
    uint8_t disparity_shift_update_sequence_length;
    uint32_t interval_duration :24;
    uint32_t division_period_count:8;

    struct division_period {
        uint8_t interval_count;
        uint8_t disparity_shift_update_integer_part;
    } *preiod;
};


struct disparity_signalling_segment {
    uint8_t dss_version_number: 4;
    uint8_t disparity_shift_update_sequence_page_flag: 1;
    uint8_t reserved:3;
    uint8_t page_default_disparity_shift;
    /* when disparity_shift_update_sequence_page_flag == 1*/
    struct disparity_shift_update_sequence disparity_shift_update_sequence;

    int n_regions;
    struct disparity_region {
        uint8_t region_id;
        uint8_t disparity_shift_update_sequence_region_flag :1 ;
        uint8_t reserved1 : 5;
        uint8_t number_of_subregions_minus_1: 2;
        struct sub_region {
            uint16_t horizontal_position;
            uint16_t width;
            uint8_t disparity_shift_integer_part;
            uint8_t disparity_shift_fractional_part: 4;
            uint8_t reserved2: 4;
            struct disparity_shift_update_sequence sub_sequence;
        } *subregions;
    }* disparity_regions;
};


struct alternative_CLUT_segment {
    uint8_t CLUT_id;
    uint8_t CLUT_version_number: 4;
    uint8_t reserved_zero_future_use: 4;
    struct {
        uint8_t CLUT_entry_max_number: 2;
        uint8_t colour_component_type: 2;
        uint8_t output_bit_depth: 3;
        uint8_t reserved_zero_future_use :1;
        uint8_t dynamic_range_and_colour_gamut;
    } CLUT_parameters;

    int n_cluts;
    struct aclut {
        uint16_t luma;
        uint16_t chroma1;
        uint16_t chroma2;
        uint16_t T;
    } *cluts;
};

#define SUBTITLE_SYNC_BYTE  (0x0F)
#define end_of_PES_data_field_marker (0xFF)

struct segment_node {
    struct list_node node;
    uint8_t type;
    struct {
        struct segment_header header;
        void * data;
    } segment;
};

/* en 300743 Table 3 */
struct subtitle_pes_data {
    uint8_t data_identifier; /* For DVB subtitle shall be coded with the value 0x20.*/
    uint8_t subtitle_stream_id; /* DVB subtitling shall be identified by the value 0x00 */

    struct list_head seg_list;
};

int parse_subtitle(uint16_t pid, uint8_t *pbuf, int len);

#ifdef __cplusplus
}
#endif

#endif /*_SUBTITLE_H_*/
