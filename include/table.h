#ifndef _TABLE_H_
#define _TABLE_H_

/* define ts structure ,see ISO/IEC13818-1 */

#ifdef __cplusplus
extern "C" {
#endif

#include "descriptor.h"
#include "list.h"
#include "statistics.h"
#include "scte/scte.h"
#include "filter.h"

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

	/* see ATSC A/65 */
	/* PSIP tables */
	MGT_TID = 0xC7,
	TVCT_TID = 0xC8,
	CVCT_TID = 0xC9,
	RRT_TID = 0xCA,
	EIT_TID = 0xCB,
	ETT_TID = 0xCC,
	STT_TID = 0xCD,

	/* 0xCE - 0xD2 atsc coordinated values */
	DCCT_TID = 0xD3,
	DCCSCT_TID = 0xD4,

	/* see SCTE 35 2022 Digital Program Insertion Cueing Message */
	SCTE_SPLICE_TID = 0xFC,

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

struct section_node
{
	uint16_t len;
	uint8_t *ptr;
};

#define MAX_SECTION_NUM 256

struct table_header
{
	uint8_t table_id;
	uint16_t section_syntax_indicator : 1;
	uint16_t private_bit : 1;
	uint16_t reserved : 2;
	uint16_t section_length : 12;
	uint16_t table_id_ext;
	uint8_t reserved1 : 2;
	uint8_t version_number : 5;
	uint8_t current_next_indicator : 1;
	uint8_t section_number;
	uint8_t last_section_number;
	uint64_t section_bitmap[4];
	struct section_node sections[MAX_SECTION_NUM]; /* section list */
	uint8_t *private_data_byte;
	uint32_t crc32;
};

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
	struct table_header pat_header;
	uint64_t program_bitmap[1024];
	struct list_head h; /* pmt list */
	uint32_t crc32;
} pat_t;

/* INFO int CAT */
typedef struct
{
	struct table_header cat_header;
	struct list_head list; /* ca_descriptor list */
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
	struct list_head list;
	struct list_node n;
};

typedef struct
{
	/* 0x02 */
	struct table_header pmt_header;
	uint16_t program_number;

	uint16_t reserved2 : 3;
	uint16_t PCR_PID : 13;
	uint16_t reserved3 : 4;
	uint16_t program_info_length : 12;
	struct list_head list; /*program info list*/
	struct list_head h;
	uint32_t crc32;
} pmt_t;


typedef struct
{
	/* 0x03 */
	struct table_header tsdt_header;
	struct list_head list; /* tsdt_descriptor list */
} tsdt_t;

/*infos int nit and bat*/
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
	/* 0x40,0x41 */
	uint16_t network_id;
	struct table_header nit_header;

	uint16_t reserved2 : 4;
	uint16_t network_descriptors_length : 12;
	struct list_head list; /*list of network descriptor*/
	uint16_t reserved3 : 4;
	uint16_t transport_stream_loop_length : 12;
	struct list_head h;
	uint32_t crc32;
} nit_t;

/*infos in bat*/
typedef struct
{
	/* 0x4A */

	struct table_header bat_header;
	uint16_t bouquet_id;

	uint16_t reserved2 : 4;
	uint16_t bouquet_descriptors_length : 12;
	struct list_head list; /*bouquet desriptor list */
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
	/* 0x42,0x46 */
	struct table_header sdt_header;
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
	struct list_head list;
	struct list_node n;
};

typedef struct
{
	/* 0x4E,0x4F,0x50-0x5F,0x60-0x6F */
	struct table_header eit_header;
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
	tsdt_t tsdt;
	int ca_num;
	int pmt_num;
	uint64_t pmt_bitmap[128];
	pmt_t pmt[8192]; /*maybe no necessary */

	uint64_t section_bitmap[128];

	nit_t nit_actual;
	nit_t nit_other;
	sdt_t sdt_actual;
	sdt_t sdt_other;
	bat_t bat;
	eit_t eit_actual;
	eit_t eit_other;
	tdt_t tdt;
	tot_t tot;
	stats_t stats;
} mpeg_psi_t;


typedef struct {
	/* 0xCD */
	struct table_header stt_header;
	uint8_t protocol_version;
	uint32_t system_time;
	uint8_t GPS_UTC_offset;
	uint16_t daylight_saving;
	struct list_head list;
} atsc_stt_t;

struct define_table {
	uint16_t table_type;
	uint16_t reserved:3;
	uint16_t table_type_PID:13;
	uint8_t reserved1:3;
	uint8_t table_type_version_number:5;
	uint32_t number_bytes;
	uint16_t reserved2:4;
	uint16_t table_type_descriptors_length:12;
	struct list_head list;
};

typedef struct {
	/* 0xC7 */
	struct table_header mgt_header;
	uint8_t protocol_version;
	uint16_t tables_defined;
	struct define_table *tables;
	uint16_t reserved:4;
	uint16_t descriptors_length:12;
	struct list_head list;
	uint32_t crc;
} atsc_mgt_t;

struct define_channel {
	uint16_t short_name[7];
	uint32_t reserved:4;
	uint32_t major_channel_number:10;
	uint32_t minor_channel_number:10;
	uint32_t modulation_mode:8;
	uint32_t carrier_frequency;
	uint16_t channel_TSID;
	uint16_t program_number;
	uint16_t ETM_location:2;
	uint16_t access_controlled:1;
	uint16_t hidden:1;
	uint16_t path_select:1;
	uint16_t out_of_band:1;
	uint16_t hide_guide:1;
	uint16_t reserved2:3;
	uint16_t service_type:6;
	uint16_t source_id;
	uint16_t reserved3:6;
	uint16_t descriptors_length:10;
	struct list_head list;
};

typedef struct {
	/* 0xC8, 0xC9 */
	struct table_header vct_header;
	uint8_t protocol_version;
	uint8_t num_channels_in_section;
	struct define_channel *channels;
	uint16_t reserved:4;
	uint16_t additional_descriptors_length:12;
	struct list_head list;
} atsc_vct_t;


struct string_segment {
	uint8_t compression_type;
	uint8_t mode;
	uint8_t number_bytes;
	uint8_t *compressed_string_byte;
};

struct lang_string {
	uint32_t ISO_639_language_code:24;
	uint32_t number_segments:8;
	struct string_segment *segments;
};


struct multiple_string {
	uint8_t number_strings;
	struct lang_string *strings;
};

struct define_rating {
	uint8_t abbrev_rating_value_length;
	struct multiple_string abbrev_rating_value_text;
	uint8_t rating_value_length;
	struct multiple_string rating_value_text;
};

struct define_dimension {
	uint8_t dimension_name_length;
	struct multiple_string dimension_name_text;
	uint8_t reserved:3;
	uint8_t graduated_scale:1;
	uint8_t values_defined:4;
	struct define_rating *rating;
};

typedef struct {
	/* 0xCA */
	struct table_header rrt_header;
	uint8_t protocol_version;
	uint8_t rating_region_name_length;
	struct multiple_string rating_region_name_text;
	uint8_t dimensions_defined;
	struct define_dimension *dimensions;
	uint16_t reserved3:6;
	uint16_t descriptors_length:10;
	struct list_head list;
} atsc_rrt_t;

struct define_event {
	uint16_t reserved:2;
	uint16_t event_id:14;
	uint32_t start_time;
	uint32_t reserved1:2;
	uint32_t ETM_location:2;
	uint32_t length_in_seconds:20;
	uint32_t title_length:8;
	struct multiple_string title_text;
	uint16_t reserved2:4;
	uint16_t descriptors_length:12;
	struct list_head list;
};

typedef struct {
	/* 0xCB */
	struct table_header eit_header;
	uint8_t protocol_version;
	uint8_t num_events_in_section;
	struct define_event *events;
} atsc_eit_t;

typedef struct {
	/* 0xCC */
	struct table_header ett_header;
	uint8_t protocol_version;
	uint32_t ETM_id;
	struct multiple_string extended_text_message;
} atsc_ett_t;

struct define_dcc_term {
	uint8_t dcc_selection_type;
	uint64_t dcc_selection_id;
	uint16_t reserved:6;
	uint16_t dcc_term_descriptors_length:10;
	struct list_head list;
};

struct define_dcc_test {
	uint64_t dcc_context:1;
	uint64_t reserved:4;
	uint64_t dcc_from_major_channel_number:10;
	uint64_t dcc_from_minor_channel_number:10;
	uint64_t reserved1:4;
	uint64_t dcc_to_major_channel_number:10;
	uint64_t dcc_to_minor_channel_number:10;
	uint64_t dcc_start_time:16;
	uint16_t dcc_start_time1;
	uint32_t dcc_end_time;
	uint8_t dcc_term_count;
	struct define_dcc_term *dcc_terms;
	uint16_t reserved2:6;
	uint16_t dcc_test_descriptors_length:10;
	struct list_head list;
};

typedef struct {
	/* 0xD3 */
	struct table_header dcct_header;
	uint8_t protocol_version;
	uint8_t dcc_test_count;
	struct define_dcc_test *dcc_tests;
	uint16_t reserved:6;
	uint16_t dcc_additional_descriptors_length:10;
	struct list_head list;
} atsc_dcct_t;

struct define_update {
	uint8_t update_type;
	uint8_t update_data_length;
	union {
		struct {
			uint8_t genre_category_code;
			struct multiple_string genre_category_name_text;
		};
		struct {
			uint8_t dcc_state_location_code;
			struct multiple_string dcc_state_location_code_text;
		};
		struct {
			uint8_t state_code;
			uint16_t reserved:6;
			uint16_t dcc_county_location_code:10;
			struct multiple_string dcc_county_location_code_text;
		};
	};
	uint16_t reserved1:6;
	uint16_t dccsct_descriptors_length:10;
	struct list_head list;
};

typedef struct {
	/* 0xD4 */
	struct table_header dccsct_header;
	uint8_t protocol_version;
	uint8_t updates_defined;
	struct define_update *updates;
	uint16_t reserved:6;
	uint16_t dccsct_additional_descriptors_length:10;
	struct list_head list;
} atsc_dccsct_t;

typedef struct {
	atsc_stt_t stt;
	atsc_mgt_t mgt;
	atsc_vct_t tvct;
	atsc_vct_t cvct;
	atsc_rrt_t rrt;
	atsc_eit_t eit;
	atsc_ett_t ett;
	atsc_dcct_t dcct;
	atsc_dccsct_t dccsct;
} atsc_psip_t;


static inline char const *get_stream_type(uint8_t type)
{
	const char *stream_type[] = {
		"Reserved",
		"ISO/IEC 11172 Video",
		"ISO/IEC 13818-2 Video",
		"ISO/IEC 11172 Audio",
		"ISO/IEC 13818-3 Audio",
		"ISO/IEC 13818-1 Private Section",
		"ISO/IEC 13818-1 Private PES data packets",
		"ISO/IEC 13522 MHEG",
		"ISO/IEC 13818-1 Annex A DSM CC",
		"ITU-T Rec. H.222.1",
		"ISO/IEC 13818-6 type A",
		"ISO/IEC 13818-6 type B",
		"ISO/IEC 13818-6 type C",
		"ISO/IEC 13818-6 type D",
		"ISO/IEC 13818-1 auxillary",
		"ISO/IEC 13818-7 Audio with ADTS transport syntax",
		"ISO/IEC 14496-2 Visual",
		"ISO/IEC 14496-3 Audio with the LATM transport syntax",
		"ISO/IEC 14496-1 SL-packetized stream or FlexMux stream carried in PES packets",
		"ISO/IEC 14496-1 SL-packetized stream or FlexMux stream carried in ISO/IEC14496_sections",
		"ISO/IEC 13818-6 Synchronized Download Protocol",
	};
	if (type < 0x15) {
		return stream_type[type];
	} else if (type < 0x80)
		return "ISO/IEC 13818-1 reserved";
	else if (type == 0x86)
		return "SCTE SPLICE INFO";
	else
		return "User Private";
}

void unregister_pmt_ops(uint16_t pid);

void register_pmt_ops(uint16_t pid);

void register_section_ops(uint16_t pid, uint8_t tableid, filter_cb callback);

void unregister_section_ops(uint16_t pid);

void init_table_ops(void);

void uninit_table_ops(void);

bool check_pmt_pid(uint16_t pid);

bool check_section_pid(uint16_t pid);

bool check_es_pid(uint16_t pid);

void dump_tables(void);

void free_tables(void);

#ifdef __cplusplus
}
#endif

#endif /* _TABLE_H_ */
