#ifndef _TABLE_H_
#define _TABLE_H_


/* define ts structure ,see ISO/IEC13818-1 */

#ifdef __cplusplus
extern "C"{
#endif

/* table id */
#define PAT_TABLE_ID	0x0
#define PMT_TABLE_ID	0x2

/* stream type */
#define STREAM_TYPE_VIDEO_11172	0x01
#define STREAM_TYEP_H262	0x02
#define STREAM_TYEP_AUDIO_11172	0x03
#define STREAM_TYEP_AUDIO_13818	0x04
#define STREAM_TYEP_H222	0x05
#define STREAM_TYEP_H222_PES	0x06
#define STREAM_TYEP_MHEG	0x07


/* INFO int PAT */
struct program_list{
	uint16_t program_number;
	uint16_t reserved:3;
	uint16_t program_map_PID:13;
	struct program_list * next;
};

typedef struct{
	uint8_t table_id;	/* 0x00 */
	uint16 section_syntax_indicator:1;
	uint16_t z:1;
	uint16_t reserved:2;
	uint16_t section_length:12;
	uint16_t transport_stream_id;
	uint8_t reserved1:2;
	uint8_t version_number:5;
	uint8_t current_next_indicator:1;
	uint8_t section_number;
	uint8_t last_section_number;
	struct program_list *list;
	uint32_t crc32;
}pat;

/* INFO int PMT */
struct descriptor{
	
};

struct stream_info{
	uint8_t stream_type;
	uint16_t reserved:3;
	uint16_t elementary_PID:13;
	uint16_t reserved1:4;
	uint16_t ES_info_length:12;
	struct descriptor * descriptor_list;
};

typedef struct{
	uint8_t table_id;	/* 0x02 */
	uint16 section_syntax_indicator:1;
	uint16_t z:1;
	uint16_t reserved:2;
	uint16_t section_length:12;
	uint16_t program_number;
	uint8_t reserved1:2;
	uint8_t version_number:5;
	uint8_t current_next_indicator:1;
	uint8_t section_number;
	uint8_t last_section_number;
	uint16_t reserved2:3;
	uint16_t PCR_PID:13;
	uint16_t reserved3:4;
	uint16_t program_info_length:12;
	struct descriptor * desriptor_list;
	struct stream_info* stream_list;
	uint32_t crc32;
}pmt;



#ifdef __cplusplus
}
#endif

#endif
