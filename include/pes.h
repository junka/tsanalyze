#ifndef _PES_H_
#define _PES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ps.h"
#include "types.h"
#include "table.h"


#if 0
enum PES_scrambling_control{
	PES_non_scrambled = 0x00,
	PES_user_defined = 0x01,
	
};
#endif

enum trick_mode_control {
	mode_fast_forward = 0x0,
	mode_slow_motion = 0x1,
	mode_freeze_frame = 0x2,
	mode_fast_reserve = 0x3,
	mode_slow_reserve = 0x4,
};

typedef struct
{
	uint8_t flag : 4;
	uint8_t PTS_DTS_1 : 3;
	uint8_t mark_bit1 : 1;
	uint16_t PTS_DTS_2 : 15;
	uint16_t mark_bit2 : 1;
	uint16_t PTS_DTS_3 : 15;
	uint16_t mark_bit3 : 1;
} PTS_DTS;

typedef struct
{
	uint64_t reserved : 2;
	uint64_t ESCR_base_1 : 3;
	uint64_t mark_bit_1 : 1;
	uint64_t ESCR_base_2 : 15;
	uint64_t mark_bit_2 : 1;
	uint64_t ESCR_base_3 : 15;
	uint64_t mark_bit_3 : 1;
	uint64_t ESCR_extension : 9;
	uint64_t mark_bit_4 : 1;
} __attribute__((packed)) ESCR;

typedef struct
{
	uint32_t mark_bit_1 : 1;
	uint32_t ES_rate : 22;
	uint32_t mark_bit_2 : 1;
} __attribute__((packed)) ES_rate;

typedef struct
{
	EXT_STD_C11
	union
	{
		struct
		{
			uint8_t DSM_trick_mode_control : 3;
			uint8_t rep_cntrl : 5;
		};
		struct
		{
			uint8_t DSM_trick_mode_control1 : 3;
			uint8_t field_id : 2;
			uint8_t intra_slice_refresh : 1;
			uint8_t frequency_truncation : 2;
		};
		struct
		{
			uint8_t DSM_trick_mode_control2 : 3;
			uint8_t field_id1 : 2;
			uint8_t reserved : 3;
		};
		struct
		{
			uint8_t DSM_trick_mode_control3 : 3;
			uint8_t reserved1 : 5;
		};
	};
} DSM_trick_mode;

typedef struct
{
	uint8_t mark_bit : 1;
	uint8_t additional_copy_info : 7;
} additional_copy;

typedef struct
{
	uint8_t marker_bit : 1;
	uint8_t program_packet_sequence_counter : 7;
	uint8_t marker_bit1 : 1;
	uint8_t MPEG1_MPEG2_identifier : 1;
	uint8_t original_stuff_length : 6;
} program_packet_sequence_counter;

typedef struct
{
	uint16_t zero_one : 2;
	uint16_t PSTD_buffer_scale : 1;
	uint16_t PSTD_buffer_size : 13;
} PSTD_buffer;

typedef struct
{
	uint8_t PES_private_data_flag : 1;
	uint8_t pack_header_field_flag : 1;
	uint8_t program_packet_sequence_counter_flag : 1;
	uint8_t P_STD_buffer_flag : 1;
	uint8_t reserved : 3;
	uint8_t PES_extension_flag_2 : 1;
	uint8_t PES_private_data[16];
	uint8_t pack_head_length;
	// pack_header pack_head;
	program_packet_sequence_counter ppsc;
	PSTD_buffer pstd_buffer;
	uint8_t marker_bit : 1;
	uint8_t PES_extension_field_length : 7;
} PES_extension;

typedef struct
{
	uint8_t reserved : 2;
	uint8_t PES_scrambling_control : 2;
	uint8_t PES_priority : 1;
	uint8_t data_alignment_indicator : 1;
	uint8_t copyright : 1;
	uint8_t original_or_copy : 1;
	uint8_t PTS_DTS_flags : 2;		 /*00 none ,01 forbiden, 10 PTS present, 11 both*/
	uint8_t ESCR_flag : 1;			 /* 1 ESCR base and extension fields are present */
	uint8_t ES_rate_flag : 1;		 /* 1 ES_rate field is present in the PES packet header*/
	uint8_t DSM_trick_mode_flag : 1; /*1indicates 8-bit trick mode field*/
	uint8_t additional_copy_info_flag : 1;
	uint8_t PES_CRC_flag : 1;
	uint8_t PES_extension_flag : 1;
	uint8_t PES_header_data_length;
	PTS_DTS pts;
	PTS_DTS dts;
	ESCR escr;
	ES_rate rate;
	DSM_trick_mode trick_mode;
	additional_copy copy_info;
	uint16_t previous_PES_packet_CRC;
	PES_extension extension;
} non_ps;

typedef int (*pes_data_callback)(uint16_t pid, uint8_t *buf, int len, void *);

typedef struct
{
	uint16_t pid;
	StreamType_E type;
	uint32_t packet_start_code_prefix : 24; /* should be PES_PACKET_START */
	uint32_t stream_id : 8;
	uint16_t PES_packet_length;
	union
	{
		non_ps packet_data;
		uint8_t *PES_packet_data_byte;
		/* user defined for private_stream_1, private_stream_2, ECM_stream, EMM_stream */
		// uint8_t *padding_byte;
	};
	void *private;
	pes_data_callback cb;
} pes_t;


#define PES_PACKET_START 0x000001

/* see Table 2-18 in iso13818-1 */
enum stream_id {
	stream_id_program_stream_end = 0xB9,
	stream_id_pack_start = 0xBA,
	stream_id_system_start = 0xBB,

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


int parse_pes_packet(uint16_t pid, uint8_t *pkt, uint16_t len);

void register_pes_ops(uint16_t pid, uint8_t stream_type);

void unregister_pes_ops(void);

void register_pes_data_callback(uint16_t pid, uint8_t stream_type, pes_data_callback cb, uint8_t tag);

void *pes_private_alloc(uint8_t tag);

void dump_pes_infos(void);

#ifdef __cplusplus
}
#endif

#endif /*_PES_H_*/
