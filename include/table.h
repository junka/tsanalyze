#ifndef _TABLE_H_
#define _TABLE_H_

/* define ts structure ,see ISO/IEC13818-1 */

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"
#include "descriptor.h"
#include "statistics.h"

/* table id */
typedef enum {
	/* ISO/IEC 13818-1, ITU T-REC H.222.0 */
	PAT_TID = 0x00,  /* program_association_section */
	CAT_TID = 0x01,  /* conditional_access_section */
	PMT_TID = 0x02,  /* TS_program_map_section */
	TSDT_TID = 0x03, /* TS_description_section */
	SDT_TID = 0x04,  /* ISO_IEC_14496_scene_description_section */
	ODT_TID = 0x05,  /* ISO_IEC_14496_object_descriptor_section */

	/* 0x06 - 0x09: ITU-T Rec. H.222.0 | ISO/IEC 13818-1 reserved */

	/* 0x0A - 0x0D: ISO/IEC 13818-6 */
	DSMCC_MULTIPROTOCOL_TID = 0x0A, /* Multiprotocol */
	DSMCC_MSG_HEADER_TID = 0x0B,	/* DSM-CC Messages Header (U-N) */
	DSMCC_DESCR_LOOP_TID = 0x0C,	/* DSM-CC Descriptors Loop */
	DSMCC_TBD_TID = 0x0D,			/* TBD */

	/* 0x0E - 0x37: ITU-T Rec. H.222.0 | ISO/IEC 13818-1 reserved */

	/* 0x38 - 0x3F: Defined in ISO/IEC 13818-6 */
	DSMCC_DL_MESSAGE_TID = 0x3B, /* DSM-CC Download Message */
	DSMCC_DL_DATA_TID = 0x3C,	/* DSM-CC Download Data */
	DSMCC_DL_EVENT_TID = 0x3D,   /* DSM-CC Download Event */

	/* 0x40 - 0x7F: ETSI EN 300 468 V1.9.1 (2009-03) */
	NIT_ACTUAL_TID = 0x40,		   /* network_information_section - actual_network */
	NIT_OTHER_TID = 0x41,		   /* network_information_section - other_network */
	SDT_ACTUAL_TID = 0x42,		   /* service_description_section - actual_transport_stream */
	SDT_OTHER_TID = 0x46,		   /* service_description_section - other_transport_stream */
	BAT_TID = 0x4A,				   /* bouquet_association_section */
	EIT_ACTUAL_TID = 0x4E,		   /* event_information_section - actual_transport_stream, present/following */
	EIT_OTHER_TID = 0x4F,		   /* event_information_section - other_transport_stream, present/following */
	EIT_ACTUAL_SCHED_0_TID = 0x50, /* event_information_section - actual_transport_stream, schedule */
	EIT_ACTUAL_SCHED_1_TID = 0x51, /* event_information_section - actual_transport_stream, schedule */
	EIT_ACTUAL_SCHED_2_TID = 0x52, /* event_information_section - actual_transport_stream, schedule */
	EIT_ACTUAL_SCHED_3_TID = 0x53, /* event_information_section - actual_transport_stream, schedule */
	EIT_ACTUAL_SCHED_4_TID = 0x54, /* event_information_section - actual_transport_stream, schedule */
	EIT_ACTUAL_SCHED_5_TID = 0x55, /* event_information_section - actual_transport_stream, schedule */
	EIT_ACTUAL_SCHED_6_TID = 0x56, /* event_information_section - actual_transport_stream, schedule */
	EIT_ACTUAL_SCHED_7_TID = 0x57, /* event_information_section - actual_transport_stream, schedule */
	EIT_ACTUAL_SCHED_8_TID = 0x58, /* event_information_section - actual_transport_stream, schedule */
	EIT_ACTUAL_SCHED_9_TID = 0x59, /* event_information_section - actual_transport_stream, schedule */
	EIT_ACTUAL_SCHED_A_TID = 0x5A, /* event_information_section - actual_transport_stream, schedule */
	EIT_ACTUAL_SCHED_B_TID = 0x5B, /* event_information_section - actual_transport_stream, schedule */
	EIT_ACTUAL_SCHED_C_TID = 0x5C, /* event_information_section - actual_transport_stream, schedule */
	EIT_ACTUAL_SCHED_D_TID = 0x5D, /* event_information_section - actual_transport_stream, schedule */
	EIT_ACTUAL_SCHED_E_TID = 0x5E, /* event_information_section - actual_transport_stream, schedule */
	EIT_ACTUAL_SCHED_F_TID = 0x5F, /* event_information_section - actual_transport_stream, schedule */
	EIT_OTHER_SCHED_0_TID = 0x60,  /* event_information_section - other_transport_stream, schedule */
	EIT_OTHER_SCHED_1_TID = 0x61,  /* event_information_section - other_transport_stream, schedule */
	EIT_OTHER_SCHED_2_TID = 0x62,  /* event_information_section - other_transport_stream, schedule */
	EIT_OTHER_SCHED_3_TID = 0x63,  /* event_information_section - other_transport_stream, schedule */
	EIT_OTHER_SCHED_4_TID = 0x64,  /* event_information_section - other_transport_stream, schedule */
	EIT_OTHER_SCHED_5_TID = 0x65,  /* event_information_section - other_transport_stream, schedule */
	EIT_OTHER_SCHED_6_TID = 0x66,  /* event_information_section - other_transport_stream, schedule */
	EIT_OTHER_SCHED_7_TID = 0x67,  /* event_information_section - other_transport_stream, schedule */
	EIT_OTHER_SCHED_8_TID = 0x68,  /* event_information_section - other_transport_stream, schedule */
	EIT_OTHER_SCHED_9_TID = 0x69,  /* event_information_section - other_transport_stream, schedule */
	EIT_OTHER_SCHED_A_TID = 0x6A,  /* event_information_section - other_transport_stream, schedule */
	EIT_OTHER_SCHED_B_TID = 0x6B,  /* event_information_section - other_transport_stream, schedule */
	EIT_OTHER_SCHED_C_TID = 0x6C,  /* event_information_section - other_transport_stream, schedule */
	EIT_OTHER_SCHED_D_TID = 0x6D,  /* event_information_section - other_transport_stream, schedule */
	EIT_OTHER_SCHED_E_TID = 0x6E,  /* event_information_section - other_transport_stream, schedule */
	EIT_OTHER_SCHED_F_TID = 0x6F,  /* event_information_section - other_transport_stream, schedule */
	TDT_TID = 0x70,				   /* time_date_section */
	RST_TID = 0x71,				   /* running_status_section */
	ST_TID = 0x72,				   /* stuffing_section */
	TOT_TID = 0x73,				   /* time_offset_section */
	AIT_TID = 0x74,				   /* application_information_section (TS 102 812) */
	CT_TID = 0x75,				   /* container_section (TS 102 323) */
	RCT_TID = 0x76,				   /* related_content_section (TS 102 323) */
	CIT_TID = 0x77,				   /* content_identifier_section (TS 102 323) */
	MPE_FEC_TID = 0x78,			   /* mpe_fec_section (EN 301 192) */
	RNT_TID = 0x79,				   /* resolution_notification_section (TS 102 323) */
	MPE_IFEC_TID = 0x7A,		   /*MPE-IFEC section (ETSI TS 102 772 [51])*/
	DIT_TID = 0x7E,				   /* discontinuity_information_section */
	SIT_TID = 0x7F,				   /* selection_information_section */

	/* 0x80 - 0x8F: ETSI ETR 289 ed.1 (1996-10) */
	CAMT_ECM_0_TID = 0x80,
	CAMT_ECM_1_TID = 0x81,
	CAMT_PRIVATE_0_TID = 0x82,
	CAMT_PRIVATE_1_TID = 0x83,
	CAMT_PRIVATE_2_TID = 0x84,
	CAMT_PRIVATE_3_TID = 0x85,
	CAMT_PRIVATE_4_TID = 0x86,
	CAMT_PRIVATE_5_TID = 0x87,
	CAMT_PRIVATE_6_TID = 0x88,
	CAMT_PRIVATE_7_TID = 0x89,
	CAMT_PRIVATE_8_TID = 0x8A,
	CAMT_PRIVATE_9_TID = 0x8B,
	CAMT_PRIVATE_A_TID = 0x8C,
	CAMT_PRIVATE_B_TID = 0x8D,
	CAMT_PRIVATE_C_TID = 0x8E,
	CAMT_PRIVATE_D_TID = 0x8F,

	/* 0x90 - 0xFE: PRIVATE */

	/* 0xFF: ISO RESERVED */
	RESERVED_TID = 0xFF,
} TID_E;

/* stream type */
typedef enum {
	STEAM_TYPE_RESERVED = 0x00,
	STEAM_TYPE_MPEG1_VIDEO = 0x01,
	STEAM_TYPE_MPEG2_VIDEO = 0x02,
	STEAM_TYPE_MPEG1_AUDIO = 0x03,
	STEAM_TYPE_MPEG2_AUDIO = 0x04,
	STEAM_TYPE_MPEG2_SECTIONS = 0x05,
	STEAM_TYPE_MPEG2_PES = 0x06,
	STEAM_TYPE_MHEG = 0x07,
	STEAM_TYPE_DSM_CC = 0x08,
	STEAM_TYPE_TREC_H_222_1 = 0x09,
	STEAM_TYPE_13818_6_A = 0x0A,
	STEAM_TYPE_13818_6_B = 0x0B,
	STEAM_TYPE_13818_6_C = 0x0C,
	STEAM_TYPE_13818_6_D = 0x0D,
	STEAM_TYPE_AUXILIARY = 0x0E,
	STEAM_TYPE_ADTS_AUDIO = 0x0F,
	STEAM_TYPE_MPEG4_VIDEO = 0x10,
	STEAM_TYPE_MPEG4_AUDIO = 0x11,
	STEAM_TYPE_MPEG4_PES = 0x12,
	STEAM_TYPE_MPEG4_SECTIONS = 0x13,
	STEAM_TYPE_SYNC_DOWNLOAD_PROT = 0x14,
	/* 0x15 - 0x7F: ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Reserved */
	/* 0x80 - 0xFF: User Private */
} StreamType_E;

/* INFO int PAT */
struct program_node
{
	uint16_t program_number;
	uint16_t reserved : 3;
	uint16_t program_map_PID : 13;
	struct list_node n;
};

typedef struct
{
	uint8_t table_id; /* 0x00 */
	uint16_t section_syntax_indicator : 1;
	uint16_t z : 1;
	uint16_t reserved : 2;
	uint16_t section_length : 12;
	uint16_t transport_stream_id;
	uint8_t reserved1 : 2;
	uint8_t version_number : 5;
	uint8_t current_next_indicator : 1;
	uint8_t section_number;
	uint8_t last_section_number;
	uint64_t program_bitmap[1024];
	struct list_head h;
	uint32_t crc32;
} pat_t;

/* INFO int CAT */
typedef struct
{
	uint8_t table_id; /* 0x00 */
	uint16_t section_syntax_indicator : 1;
	uint16_t z : 1;
	uint16_t reserved : 2;
	uint16_t section_length : 12;
	uint16_t transport_stream_id;
	uint8_t reserved1 : 2;
	uint8_t version_number : 5;
	uint8_t current_next_indicator : 1;
	uint8_t section_number;
	uint8_t last_section_number;
	// struct descriptor *list; /*may have multicrypt CA descriptor here*/
	struct list_head list;
	uint32_t crc32;
} cat_t;

/* INFO int PMT */
struct es_node
{
	uint8_t stream_type;
	uint16_t reserved : 3;
	uint16_t elementary_PID : 13;
	uint16_t reserved1 : 4;
	uint16_t ES_info_length : 12;
	// struct descriptor * descriptor_list;
	struct list_head list;
	struct list_node n;
};

typedef struct
{
	uint8_t table_id; /* 0x02 */
	uint16_t section_syntax_indicator : 1;
	uint16_t z : 1;
	uint16_t reserved : 2;
	uint16_t section_length : 12;
	uint16_t program_number;
	uint8_t reserved1 : 2;
	uint8_t version_number : 5;
	uint8_t current_next_indicator : 1;
	uint8_t section_number;
	uint8_t last_section_number;
	uint16_t reserved2 : 3;
	uint16_t PCR_PID : 13;
	uint16_t reserved3 : 4;
	uint16_t program_info_length : 12;
	// struct descriptor * desriptor_list;
	struct list_head list;
	struct list_head h;
	uint32_t crc32;
} pmt_t;

/*infos int nit*/
struct transport_stream_node
{
	uint16_t transport_stream_id;
	uint16_t original_network_id;
	uint16_t reserved_future_use : 4;
	uint16_t transport_descriptors_length : 12;
	struct list_head list;
	struct list_node n;
};

typedef struct
{
	uint8_t table_id; /* 0x40,0x41 */
	uint16_t section_syntax_indicator : 1;
	uint16_t reserved_future_use : 1;
	uint16_t reserved : 2;
	uint16_t section_length : 12;
	uint16_t network_id;
	uint8_t reserved1 : 2;
	uint8_t version_number : 5;
	uint8_t current_next_indicator : 1;
	uint8_t section_number;
	uint8_t last_section_number;
	uint16_t reserved2 : 4;
	uint16_t network_descriptors_length : 12;
	struct list_head list;
	// struct descriptor * network_desriptor_list;
	uint16_t reserved3 : 4;
	uint16_t transport_stream_loop_length : 12;
	struct list_head h;
	uint32_t crc32;
} nit_t;

/*infos in bat*/
typedef struct
{
	uint8_t table_id; /* 0x4A */
	uint16_t section_syntax_indicator : 1;
	uint16_t reserved_future_use : 1;
	uint16_t reserved : 2;
	uint16_t section_length : 12;
	uint16_t bouquet_id;
	uint8_t reserved1 : 2;
	uint8_t version_number : 5;
	uint8_t current_next_indicator : 1;
	uint8_t section_number;
	uint8_t last_section_number;
	uint16_t reserved2 : 4;
	uint16_t bouquet_descriptors_length : 12;
	// struct descriptor * bouquet_desriptor_list;
	struct list_head list;
	uint16_t reserved3 : 4;
	uint16_t transport_stream_loop_length : 12;
	struct list_head h;
	uint32_t crc32;
} bat_t;

/*infos in SDT*/
struct service_node
{
	uint16_t service_id;
	uint8_t reserved_future_use : 6;
	uint8_t EIT_schedule_flag : 1;
	uint8_t EIT_present_following_flag : 1;
	uint16_t running_status : 3;
	uint16_t free_CA_mode : 1;
	uint16_t descriptors_loop_length : 12;
	// struct descriptor * service_desriptor_list;
	struct list_head list;
	struct list_node n;
};

typedef struct
{
	uint8_t table_id; /* 0x42,0x46 */
	uint16_t section_syntax_indicator : 1;
	uint16_t reserved_future_use : 1;
	uint16_t reserved : 2;
	uint16_t section_length : 12;
	uint16_t transport_stream_id;
	uint8_t reserved1 : 2;
	uint8_t version_number : 5;
	uint8_t current_next_indicator : 1;
	uint64_t section_bitmap[4];
	uint8_t section_number;
	uint8_t last_section_number;
	uint16_t original_network_id;
	uint8_t reserved2;
	struct list_head h;
	uint32_t crc32;
} sdt_t;

struct event_node
{
	uint16_t event_id;
	uint64_t start_time : 40;
	uint64_t duration : 24;
	uint16_t running_status : 3;
	uint16_t free_CA_mode : 1;
	uint16_t descriptors_loop_length : 12;
	struct descriptor *event_desriptor_list;
	struct list_node n;
};

typedef struct
{
	uint8_t table_id; /* 0x4E,0x4F,0x50-0x5F,0x60-0x6F */
	uint16_t section_syntax_indicator : 1;
	uint16_t reserved_future_use : 1;
	uint16_t reserved : 2;
	uint16_t section_length : 12;
	uint16_t service_id;
	uint8_t reserved1 : 2;
	uint8_t version_number : 5;
	uint8_t current_next_indicator : 1;
	uint8_t section_number;
	uint8_t last_section_number;
	uint16_t transport_stream_id;
	uint16_t original_network_id;
	uint8_t segment_last_section_number;
	uint8_t last_table_id;
	struct list_head h;
	uint32_t crc32;
} eit_t;

typedef struct
{
	uint8_t table_id; /* 0x70 */
	uint16_t section_syntax_indicator : 1;
	uint16_t reserved_future_use : 1;
	uint16_t reserved : 2;
	uint16_t section_length : 12;
	UTC_time_t utc_time;
} tdt_t;

typedef struct
{
	uint8_t table_id; /* 0x73 */
	uint16_t section_syntax_indicator : 1;
	uint16_t reserved_future_use : 1;
	uint16_t reserved : 2;
	uint16_t section_length : 12;
	UTC_time_t utc_time;
	uint16_t reserved1 : 4;
	uint16_t descriptors_loop_length : 12;
	// struct descriptor * time_offset_descriptor_list;
	struct list_head list;
	uint32_t crc32;
} tot_t;

struct running_status
{
	uint16_t transport_stream_id;
	uint16_t original_network_id;
	uint16_t service_id;
	uint16_t event_id;
	uint8_t reserved_future_use : 5;
	uint8_t running_status : 3;
	struct running_status *next;
};

typedef struct
{
	uint8_t table_id; /* 0x71 */
	uint16_t section_syntax_indicator : 1;
	uint16_t reserved_future_use : 1;
	uint16_t reserved : 2;
	uint16_t section_length : 12;
	struct running_status *status_list;
} rst_t;

typedef struct
{
	uint8_t table_id;
	uint16_t section_syntax_indicator : 1;
	uint16_t reserved_future_use : 1;
	uint16_t reserved : 2;
	uint16_t section_length : 12;
	uint8_t data_byte[0];
} st_t;

typedef struct
{
	pat_t pat;
	cat_t cat;
	int ca_num;
	int pmt_num;
	uint64_t pmt_bitmap[128];
	pmt_t pmt[8192]; /*maybe no necessary */
	uint8_t has_pat : 1;
	uint8_t has_nit : 1;
	uint8_t has_sdt : 1;
	uint8_t has_bat : 1;
	uint8_t has_eit : 1;
	uint8_t has_tdt : 1;
	uint8_t has_tot : 1;
	nit_t nit;
	sdt_t sdt;
	bat_t bat;
	eit_t eit;
	tdt_t tdt;
	tot_t tot;
	stats_t stats;
} mpeg_psi_t;

void init_table_ops(void);

void dump_tables(void);

#ifdef __cplusplus
}
#endif

#endif
