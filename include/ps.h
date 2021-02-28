#ifndef _PS_H_
#define _PS_H_

#include <stdint.h>
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PACK_START 0x000001BA
#define SYSTEM_START 0x000001BB

#define PES_PACKET_START 0x000001

#define PROGRAM_END 0x000001B9

/* see Table 2-18 in iso13818-1 */
enum stream_id{
	stream_id_program_stream_map = 0xBC,
	stream_id_private_stream_1 = 0xBD,
	stream_id_padding_stream = 0xBE,
	stream_id_private_stream_2 = 0xBF,

	/* 0xC0 - 0xDF   ISO/IEC 13818-3 or ISO/IEC 11172-3 or 
	ISO/IEC 13818-7 or ISO/IEC 14496-3 audio stream number x xxxx */
	stream_id_13818_audio_stream_0 = 0xC0,
	stream_id_13818_audio_stream_1 = 0xC1,
	stream_id_13818_audio_stream_2 = 0xC2,
	stream_id_13818_audio_stream_3 = 0xC3,
	stream_id_13818_audio_stream_4 = 0xC4,
	stream_id_13818_audio_stream_5 = 0xC5,
	stream_id_13818_audio_stream_6 = 0xC6,
	stream_id_13818_audio_stream_7 = 0xC7,
	stream_id_13818_audio_stream_8 = 0xC8,
	stream_id_13818_audio_stream_9 = 0xC9,
	stream_id_13818_audio_stream_a = 0xCA,
	stream_id_13818_audio_stream_b = 0xCB,
	stream_id_13818_audio_stream_c = 0xCC,
	stream_id_13818_audio_stream_d = 0xCD,
	stream_id_13818_audio_stream_e = 0xCE,
	stream_id_13818_audio_stream_f = 0xCF,
	stream_id_13818_audio_stream_10 = 0xD0,
	stream_id_13818_audio_stream_11 = 0xD1,
	stream_id_13818_audio_stream_12 = 0xD2,
	stream_id_13818_audio_stream_13 = 0xD3,
	stream_id_13818_audio_stream_14 = 0xD4,
	stream_id_13818_audio_stream_15 = 0xD5,
	stream_id_13818_audio_stream_16 = 0xD6,
	stream_id_13818_audio_stream_17 = 0xD7,
	stream_id_13818_audio_stream_18 = 0xD8,
	stream_id_13818_audio_stream_19 = 0xD9,
	stream_id_13818_audio_stream_1a = 0xDA,
	stream_id_13818_audio_stream_1b = 0xDB,
	stream_id_13818_audio_stream_1c = 0xDC,
	stream_id_13818_audio_stream_1d = 0xDD,
	stream_id_13818_audio_stream_1e = 0xDE,
	stream_id_13818_audio_stream_1f = 0xDF,

	/* 0xE0 - 0xEF   ITU-T Rec. H.262 | ISO/IEC 13818-2 or 
	ISO/IEC 11172-2 or ISO/IEC 14496-2 video stream number xxxx */
	stream_id_H262_14496_video_stream_0 = 0xE0,
	stream_id_H262_14496_video_stream_1 = 0xE1,
	stream_id_H262_14496_video_stream_2 = 0xE2,
	stream_id_H262_14496_video_stream_3 = 0xE3,
	stream_id_H262_14496_video_stream_4 = 0xE4,
	stream_id_H262_14496_video_stream_5 = 0xE5,
	stream_id_H262_14496_video_stream_6 = 0xE6,
	stream_id_H262_14496_video_stream_7 = 0xE7,
	stream_id_H262_14496_video_stream_8 = 0xE8,
	stream_id_H262_14496_video_stream_9 = 0xE9,
	stream_id_H262_14496_video_stream_a = 0xEA,
	stream_id_H262_14496_video_stream_b = 0xEB,
	stream_id_H262_14496_video_stream_c = 0xEC,
	stream_id_H262_14496_video_stream_d = 0xED,
	stream_id_H262_14496_video_stream_e = 0xEE,
	stream_id_H262_14496_video_stream_f = 0xEF,
	stream_id_ECM_stream = 0xF0,
	stream_id_EMM_stream = 0xF1,
	stream_id_H222_DSMCC_stream = 0xF2,
	stream_id_13522_stream = 0xF3,
	stream_id_H222_typeA_stream = 0xF4,
	stream_id_H222_typeB_stream = 0xF5,
	stream_id_H222_typeC_stream = 0xF6,
	stream_id_H222_typeD_stream = 0xF7,
	stream_id_H222_typeE_stream = 0xF8,
	stream_id_ancillary_stream = 0xF9,
	stream_id_SL_packetized_stream = 0xFA,
	stream_id_FlexMux_stream = 0xFB,
	/* reserved data stream 0xFC - 0xFE */
	stream_id_program_stream_directory = 0xFF,

};


typedef struct
{
	uint8_t stream_id;
	uint16_t one : 2;
	uint16_t PSTD_buffer_bound_scale : 1;
	uint16_t PSTD_buffer_size_bound : 13;
}__attribute__((packed)) estd;

/* see Table 2-34 in iso13818-1 */
typedef struct
{
	uint32_t system_header_start_code;
	uint16_t header_length;
	uint32_t marker_bit : 1;
	uint32_t rate_bound : 22;
	uint32_t marker_bit1 : 1;
	uint32_t audio_bound : 6;
	uint32_t fixed_flag : 1;
	uint32_t CSPS_flag : 1;
	uint8_t system_audio_lock_flag : 1;
	uint8_t system_video_lock_flag : 1;
	uint8_t marker_bit2 : 1;
	uint8_t video_bound : 5;
	uint8_t packet_rate_restriction_flag : 1;
	uint8_t reserved_bits : 7;
	estd es;
} system_header;

/* see Table 2-33 in iso13818-1 */
typedef struct
{
	uint32_t pack_start_code;

	union {
		uint64_t zero_one : 2;
		uint64_t system_clock_reference_base1 : 3;
		uint64_t marker_bit1 : 1;
		uint64_t system_clock_reference_base2 : 15;
		uint64_t marker_bit2 : 1;
		uint64_t system_clock_reference_base3 : 15;
		uint64_t marker_bit3 : 1;
		uint64_t system_clock_reference_extension : 9;
		uint64_t marker_bit4 : 1;
		uint64_t program_mux_rate_h : 16;

		uint64_t md;
	};

	uint8_t program_mux_rate_l : 6;
	uint8_t marker_bit5 : 1;
	uint8_t marker_bit6 : 1;

	uint8_t reserved : 3;
	uint8_t pack_stuffing_length : 5;
	uint8_t *stuffing_byte;
} pack_header;

typedef struct {
	uint8_t stream_type;
	uint8_t elementary_stream_id;
	uint16_t elementary_stream_info_length;
	struct list_head list; /*es descriptor list*/
	struct list_node n;
} es_map;

/* description of the elementary streams in the Program Stream
  see Table 2-35 in iso13818-1 */
typedef struct
{
	uint32_t packet_start_code_prefix : 24;  //should be PES_START_PREFIX 0x000001
	uint32_t map_stream_id : 8;
	uint16_t program_stream_map_length;
	uint8_t current_next_indicator : 1;
	uint8_t reserved : 2;
	uint8_t program_stream_map_version : 5;
	uint8_t reserved1 : 7;
	uint8_t marker_bit : 1;
	uint16_t program_stream_info_length;
	struct list_head list;  //ps descriptor list
	uint16_t elementary_stream_map_length;
	struct list_head h; /*ES es_map list*/
	uint32_t crc_32;
} ps_map;

typedef struct
{
	uint8_t packet_stream_id;
	uint16_t PES_header_position_offset_sign : 1;
	uint16_t PES_header_position_offset1 : 14;
	uint16_t marker_bit1 : 1;
	uint16_t PES_header_position_offset2 : 15;
	uint16_t marker_bit2 : 1;
	uint16_t PES_header_position_offset3 : 15;
	uint16_t marker_bit3 : 1;
	uint16_t reference_offset;
	uint8_t marker_bit4 : 1;
	uint8_t reserved : 3;
	uint8_t PTS1 : 3;
	uint8_t marker_bit5 : 1;
	uint16_t PTS2 : 15;
	uint16_t marker_bit6 : 1;
	uint16_t PTS3 : 15;
	uint16_t marker_bit7 : 1;
	uint16_t bytes_to_read1 : 15;
	uint16_t marker_bit8 : 1;
	uint8_t bytes_to_read2;
	uint8_t marker_bit9 : 1;
	uint8_t intra_coded_indicator : 1;
	uint8_t coding_parameters_indicator : 2;
	uint8_t reserved1 : 4;
} access_unit;

typedef struct
{
	uint32_t packet_start_code_prefix : 24;
	uint32_t directory_stream_id : 8;
	uint16_t PES_packet_length;
	uint16_t number_of_access_units : 15;
	uint16_t marker_bit : 1;
	uint16_t prev_directory_offset1 : 15;
	uint16_t marker_bit1 : 1;
	uint16_t prev_directory_offset2 : 15;
	uint16_t marker_bit2 : 1;
	uint16_t prev_directory_offset3 : 15;
	uint16_t marker_bit3 : 1;
	uint16_t prev_directory_offset4 : 15;
	uint16_t marker_bit4 : 1;
	uint16_t prev_directory_offset5 : 15;
	uint16_t marker_bit5 : 1;
	uint16_t prev_directory_offset6 : 15;
	uint16_t marker_bit6 : 1;
	access_unit * units;
} directory_PES_packet;

typedef struct
{
	pack_header head;
	// PES_packet;
} pack;

typedef struct
{
	pack pack;
	uint32_t MPEG_program_end_code;
} MPEG2_PS;

#ifdef __cplusplus
}
#endif

#endif /*_PS_H_*/
