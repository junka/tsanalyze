#ifndef _DESCRIPTOR_H_
#define _DESCRIPTOR_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "types.h"
#include "list.h"

typedef struct descriptor{
	uint8_t tag;
	uint8_t length;
	struct list_node n;
	uint8_t data[0];
}descriptor_t;

/* ISO/IEC 13818-1 */
#define foreach_enum_descriptor \
	_(video_stream, 0x02)\
	_(audio_stream,	0x03)\
	_(hierarchy,		0x04)\
	_(registration,	0x05)\
	_(data_stream_alignment,	0x06)\
	_(target_background_grid,	0x07)\
	_(video_window,			0x08)\
	_(CA,			0x09)\
	_(ISO_639_language,	0x0A)\
	_(system_clock,		0x0B)\
	_(multiplex_buffer_utilization,	0x0C)\
	_(copyright,			0x0D)\
	_(maximum_bitrate,	0x0E)\
	_(private_data_indicator,	0x0F)\
	_(smoothing_buffer,	0x10)\
	_(STD,				0x11)\
	_(ibp,				0x12)\
	/* 0x12 - 0x1A Defined in ISO/IEC 13818-6*/\
	\
	_(MPEG4_video,		0x1B)\
	_(MPEG4_audio,		0x1C)\
	_(IOD,				0x1D)\
	_(SL,				0x1E)\
	_(FMC,				0x1F)\
	_(external_ES_ID,	0x20)\
	_(muxcode,			0x21)\
	_(FmxBufferSize,	0x22)\
	_(MultiplexBuffer,	0x23)\
	/* 0x24 - 0x3F reserved */\
	/* EN 300 468*/\
	_(network_name,		0x40)\
	_(service_list,		0x41)\
	_(stuffing,			0x42)\
	_(satellite_delivery_system, 0x43)\
	_(cable_delivery_system,	0x44)\
	_(VBI_data,			0x45)\
	_(VBI_teletext,		0x46)\
	_(bouquet_name,		0x47)\
	_(service,			0x48)\
	_(country_availability,	0x49)\
	_(linkage,			0x4A)\
	_(NVOD_reference,	0x4B)\
	_(time_shifted_service,	0x4C)\
	_(short_event,		0x4D)\
	_(extended_event,	0x4E)\
	_(time_shifted_event,	0x4F)\
	_(component,			0x50)\
	_(mosaic,			0x51)\
	_(stream_identifier,	0x52)\
	_(CA_identifier,		0x53)\
	_(content,			0x54)\
	_(parental_rating,	0x55)\
	_(teletext,			0x56)\
	_(telephone,			0x57)\
	_(local_time_offset,	0x58)\
	_(subtitling,		0x59)\
	_(terrestrial_delivery_system,	0x5A)\
	_(multilingual_network_name,	0x5B)\
	_(multilingual_bouquet_name,	0x5C)\
	_(multilingual_service_name,	0x5D)\
	_(multilingual_component,	0x5E)\
	_(private_data_specifier,	0x5F)\
	_(service_move,		0x60)\
	_(short_smoothing_buffer, 0x61)\
	_(frequency_list,	0x62)\
	_(partial_transport_stream, 0x63)\
	_(data_broadcast,	0x64)\
	_(scrambling,		0x65)\
	_(data_broadcast_id,	0x66)\
	_(transport_stream,	0x67)\
	_(DSNG,			0x68)\
	_(PDC,				0x69)\
	_(AC3,				0x6A)\
	_(ancillary_data,	0x6B)\
	_(cell_list,			0x6C)\
	_(cell_frequency_link,	0x6D)\
	_(announcement_support,	0x6E)\
	_(application_signalling,	0x6F)\
	_(adaptation_field_data,	0x70)\
	_(service_identifier,	0x71)\
	_(service_availability,	0x72)\
	_(default_authority,	0x73)\
	_(related_content,	0x74)\
	_(TVA_id,			0x75)\
	_(content_identifier,	0x76)\
	_(time_slice_fec_identifier, 0x77)\
	_(ECM_repetition_rate, 0x78)\
	_(S2_satellite_delivery_system, 0x79)\
	_(enhanced_AC3,		0x7A)\
	_(DTS,				0x7B)\
	_(AAC,				0x7C)\
	_(XAIT_location,	0x7D)\
	_(FTA_content_management,0x7E)\
	_(extension,		0x7F)
	/*0x80 to 0xFE user defined */
	/*0xFF forbidden */

enum descriptor_e{
#define _(a,b)  dr_##a = b,
	foreach_enum_descriptor
#undef _
};

/* see ISO/IEC 13818-1 chapter 2.6*/
typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t multiple_frame_rate_flag:1; /*set to '1' indicates that multiple frame rates may be present*/
			uint8_t frame_rate_code:4;
			uint8_t MPEG_1_only_flag:1; /*set to '1' indicates that the video stream contains only ISO/IEC 11172-2 data*/
			uint8_t constrained_parameter_flag:1;
			uint8_t still_picture_flag:1; /*set to '1' indicates that the video stream contains only still pictures.*/
			/*exist only when MPEG_1_only_flag == 0*/
			uint8_t profile_and_level_indication;
			uint8_t chroma_format:2;
			uint8_t frame_rate_extension_flag:1;
			uint8_t reserved:5;
		};
	};
}video_stream_descriptor_t;

typedef struct {
	EXT_STD_C11
	union {
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			//void *next;
			struct list_node n;
			uint8_t free_format_flag:1;
			uint8_t ID:1;
			uint8_t layer:2;
			uint8_t variable_rate_audio_indicator:1;
			uint8_t reserved:3;
		};
	};
} audio_stream_descriptor_t;

enum hierarchy_type_e{
	spatial_scalability = 1, /*ITU-T Rec. H.262 | ISO/IEC 13818-2 spatial_scalability*/
	SNR_scalability = 2, /*ITU-T Rec. H.262 | ISO/IEC 13818-2 SNR_scalability */
	temporal_scalability = 3,/* ITU-T Rec. H.262 | ISO/IEC 13818-2 temporal_scalability*/
	data_partitioning = 4,/* ITU-T Rec. H.262 | ISO/IEC 13818-2 data_partitioning */
	extension_bitstream = 5, /*ISO/IEC 13818-3 extension_bitstream*/
	private_stream = 6,	 /* ITU-T Rec.H222.0 | ISO/IEC 13818-1 Private Stream private_stream*/
	multiview_profile = 7,/* ITU-T Rec. H.262 | ISO/IEC 13818-2 Multi-view Profile*/
	base_layer = 15,
};

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t reserved:4;
			uint8_t hierarchy_type:4; /*see definition in @hierarchy_type_e*/
			uint8_t reserved1:2;
			uint8_t hierarchy_layer_index:6;
			uint8_t reserved2:2;
			uint8_t hierarchy_embedded_layer_index:6;
			uint8_t reserved3:2;
			uint8_t hierarchy_channel:6;
		};
	};

}hierarchy_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint32_t format_identifier;
			uint8_t* additional_identification_info;
		};
	};
} registration_descriptor_t;

enum video_alignment_type_e{
	slice_or_video_access_unit = 1,
	video_access_unit = 2,
	GOP_or_SEQ = 3,
	SEQ = 4,
	/* 0x05 - 0xFF reserved*/
};

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t alignment_type; /* see definition in @video_alignment_type_e */
		};
	};
} data_stream_alignment_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint32_t horizontal_size:14;
			uint32_t vertical_size:14;
			uint32_t aspect_ratio_information:4;
		};
	};
} target_background_grid_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint32_t horizontal_offset:14;
			uint32_t vertical_offset:14;
			uint32_t window_priority:4;
		};
	};
} video_window_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint16_t CA_system_ID;
			uint16_t reserved:3;
			uint16_t CA_PID:13;
			uint8_t* private_data_byte;
		};
	};
} CA_descriptor_t;

enum audio_type_e{
	undefined = 0x0,
	clean_effects = 0x1,
	hearing_impaired = 0x2,
	visual_impaired_commentary = 0x3,
	/*0x04-0xFF reserved*/
};

struct language_node{
	uint32_t ISO_639_language_code:24;
	uint32_t audio_type:8; /*see definition in @audio_type_e */
	struct list_node n;
	//struct language_info * next;
};

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			//uint32_t lang_num;
			//struct language_info* language_list;
			struct list_head list;
		};
	};
} ISO_639_language_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t external_clock_reference_indicator:1;
			uint8_t reserved:1;
			uint8_t clock_accuracy_integer:6;
			uint8_t clock_accuracy_exponent:3;
			uint8_t reserved1:5;
		};
	};
}system_clock_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint16_t bound_valid_flag:1;
			uint16_t LTW_offset_lower_bound:15;
			uint16_t reserved:1;
			uint16_t LTW_offset_upper_bound:15;
		};
	};
} multiplex_buffer_utilization_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint32_t copyright_identifier;
			uint8_t *additional_copyright_info;
		};
	};
} copyright_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint24_t maximum_bitrate;//22bit
		};
	};
} maximum_bitrate_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint32_t private_data_indicator;
		};
	};
}private_data_indicator_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint24_t sb_leak_rate;
			uint24_t sb_size;
			//uint24_t reserved:2;
			//uint24_t sb_leak_rate:22;
			//uint24_t reserved1:2;
			//uint24_t sb_size:22;
		};
	};
}smoothing_buffer_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t reserved:7;
			uint8_t leak_valid_flag:1;
		};
	};
}STD_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint16_t closed_gop_flag:1;
			uint16_t identical_gop_flag:1;
			uint16_t max_gop_length:14;
		};
	};
}ibp_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t MPEG4_visual_profile_and_level;
		};
	};
}MPEG4_video_descriptor_t;

enum MPEG4_audio_profile_and_level_e{
	main_profile_lv1 = 0x10,
	main_profile_lv2 = 0x11,
	main_profile_lv3 = 0x12,
	main_profile_lv4 = 0x13,
	/*0x14-0x17 reserved */
	
	
};

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t MPEG4_audio_profile_and_level;
		};
	};
} MPEG4_audio_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t Scope_of_IOD_label;
			uint8_t IOD_label;
		};
	};
} IOD_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint16_t ES_ID;
		};
	};
} SL_descriptor_t;

struct FMC_node{
	uint16_t ES_ID;
	uint8_t FlexMuxChannel;
	struct list_node n;
	//struct FMC_info*next;
};

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			struct list_head list;
			//struct FMC_info *FMC_info_list;
		};
	};
}FMC_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint16_t external_ES_ID;
		};
	};
}external_ES_ID_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
		};
	};
}muxcode_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
		};
	};
}FmxBufferSize_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint24_t MB_buffer_size;
			uint24_t TB_leak_rate;/* in units of 400 bits per second the rate at which data is transferred */
		};
	};
}MultiplexBuffer_descriptor_t;

/*see EN_300 468 chapter 6*/

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			char *text_byte;
		};
	};
}network_name_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
		};
	};

}service_list_descriptor_t;

typedef struct{
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			char *stuffing_byte;
		};
	};
}stuffing_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint32_t frequency;
			uint16_t orbital_position;
			uint8_t west_east_flag:1;
			uint8_t polarization:2;
			uint8_t roll_off:2;
			uint8_t modulation_system:1;
			uint8_t modulation_type:2;
			uint32_t symbol_rate:28;
			uint32_t FEC_inner:4;
		};
	};
}satellite_delivery_system_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint32_t frequency;
			uint16_t reserved_future_use:12;
			uint16_t FEC_outer:4;
			uint8_t modulation;
			uint32_t symbol_rate:28;
			uint32_t FEC_inner:4;
		};
	};
} cable_delivery_system_descriptor_t;

struct VBI_data_node {
	uint8_t data_service_id;
	uint8_t data_service_descriptor_length;
	uint8_t *reserved;
	struct list_node n;
};

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			struct list_head list;
			//struct VBI_data_info *list;
		};
	};
}VBI_data_descriptor_t;

struct VBI_teletext_node {
	uint32_t ISO_639_language_code:24;
	uint32_t teletext_type:5;
	uint32_t teletext_magazine_number:3;
	uint8_t teletext_page_number;
	struct list_node n;
};

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			struct list_head list;
			//struct VBI_teletext_info* list;
		};
	};
}VBI_teletext_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			char *sub_table;
		};
	};
}bouquet_name_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t service_type;
			uint8_t service_provider_name_length;
			uint8_t *text_char;
			uint8_t service_name_length;
			uint8_t *service_char;
		};
	};
}service_descriptor_t;


enum country_code_e{
	AFG,
	ALA,
	ALB,
	DZA,
	ASM,
	AND,
	AGO,
	AIA,
	ATA,
	ATG,
	ARG,
	ARM,
	ABW,
	AUS,
	AUT,
	AZE,
	BHS,
	BHR,
	BGD,
	BRB,
	BLR,
};

struct country_code_node{
	uint24_t country_code;
	//struct country_code * next;
	struct list_node n;
};

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t country_availability_flag:1;
			uint8_t reserved_future_use:7;
			struct list_head list;
			//struct country_code* country_list;
		};
	};
}country_availability_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint16_t transport_stream_id;
			uint16_t original_network_id;
			uint16_t service_id;
			uint8_t linkage_type;
			//TODO
			uint8_t *private_data_byte;
		};
	};
}linkage_descriptor_t;

struct NVOD_reference_node{
	uint16_t transport_stream_id;
	uint16_t original_network_id;
	uint16_t service_id;
	struct list_node n;
};

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			struct list_head list;
			//struct NVOD_refer *nvod_list;
		};
	};
}NVOD_reference_descriptor_t;

typedef struct{
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t reference_service_id;
		};
	};
}time_shifted_service_descriptor_t;

typedef struct{
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint32_t ISO_639_language_code:24;
			uint32_t event_name_length:8;
			uint8_t *event_name_char;
			uint8_t text_length;
			uint8_t *text_char;
		};
	};
}short_event_descriptor_t;


struct event_item_node{
	uint8_t item_description_length;
	uint8_t* item_description_char;
	uint8_t item_length;
	uint8_t *item_char;
	struct list_node n;
};


typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t descriptor_number:4;
			uint8_t last_descriptor_number:4;
			uint32_t ISO_639_language_code:24;
			uint32_t length_of_items:8;
			struct list_head list;
			//struct event_item item_list;
			uint8_t text_length;
			uint8_t *text_char;
		};
	};
}extended_event_descriptor_t;

typedef struct{
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
		};
	};
}time_shifted_event_descriptor_t;

typedef struct { 
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t stream_content_ext:4; 
			uint8_t stream_content:4;
			uint8_t component_type;
			uint32_t component_tag:8; 
			uint32_t ISO_639_language_code:24; 
			uint8_t *text_char;
		};
	};
} component_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
		};
	};

}mosaic_descriptor_t;

typedef struct{
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t component_tag;
		};
	};
}stream_identifier_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint16_t *CA_system_id; /*ETSI TS 101 162 [i.1]*/
		};
	};
}CA_identifier_descriptor_t;

struct content_node{
	uint8_t content_nibble_level_1:4;
	uint8_t content_nibble_level_2:4;
	uint8_t byte;
	struct list_node n;
};

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			struct list_head list;
			//struct content_info * content_list;
		};
	};
}content_descriptor_t;


typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint32_t *countrycode_and_rating;
		};
	};
}parental_rating_descriptor_t;

enum teletext_type{
	teletext_reserved = 0x0,
	teletext_initial_page = 0x1,
	teletext_subtitle_page = 0x2,
	additional_information_page = 0x3,
	programme_schedule_page = 0x4,
	hearing_impaired_page = 0x5,
	/*0x06 to 0x1F reserved*/
};

struct teletext_node{
	uint32_t ISO_639_language_code:24;
	uint32_t teletext_type:5;
	uint32_t teletext_magazine_number:3;
	uint8_t teletext_page_number;
	struct list_node n;
};

typedef struct{
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			struct list_head list;
		};
	};
}teletext_descriptor_t;

typedef struct{
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t reserved_future_use:2;
			uint8_t foreign_availability:1;
			uint8_t connection_type:5;
			uint8_t reserved_future_use1:1;
			uint8_t country_prefix_length:2;
			uint8_t international_area_code_length:3;
			uint8_t operator_code_length:2;
			uint8_t reserved_future_use2:1;
			uint8_t national_area_code_length:3;
			uint8_t core_number_length:4;
			uint8_t *country_prefix_char;
			uint8_t *international_area_code_char;
			uint8_t *operator_code_char;
			uint8_t *national_area_code_char;
			uint8_t *core_number_char;
		};
	};
}telephone_descriptor_t;

struct local_time_node{
	uint32_t country_code:24;
	uint32_t country_region_id:6;
	uint32_t reserved:1;
	uint32_t local_time_offset_polarity:1;
	uint16_t local_time_offset;
	uint40_t time_of_change;
	uint16_t next_time_offset;
	struct list_node n;
};

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			struct list_head list;
			//struct local_time_info* time_list;
		};
	};
}local_time_offset_descriptor_t;

struct subtitling_node{
	uint32_t ISO_639_language_code:24;
	uint32_t subtitling_type:8;
	uint16_t composition_page_id;
	uint16_t ancillary_page_id;
	struct list_node n;
};

typedef struct{
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			struct list_head list;
			//struct subtitling_info* subtitle_list;
		};
	};
}subtitling_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint32_t centre_frequency;
			uint8_t bandwidth:3;
			uint8_t priority:1;
			uint8_t time_slicing_indicator:1;
			uint8_t MPE_FEC_indicator:1;
			uint8_t reserved_future_use:2;
			uint8_t constellation:2;
			uint8_t hierarchy_information:3;
			uint8_t code_rate_HP_stream:3;
			uint8_t code_rate_LP_stream:3;
			uint8_t guard_interval:2;
			uint8_t transmission_mode:2;
			uint8_t other_frequency_flag:1;
			uint32_t reserved_future_use1;
		};
	};
}terrestrial_delivery_system_descriptor_t;

struct multilingual_node{
	uint32_t ISO_639_language_code:24;
	uint32_t name_length:8;
	uint8_t *text_char;
	struct list_node n;
};

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			struct list_head list;
			//struct multilingual_info* time_list;
		};
	};

}multilingual_network_name_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			struct list_head list;
			//struct multilingual_info* time_list;
		};
	};
}multilingual_bouquet_name_descriptor_t;

struct multilingual_service_node{
	uint32_t ISO_639_language_code:24;
	uint32_t name_length:8;
	uint8_t *text_char;
	uint8_t service_name_length;
	uint8_t *service_char;
	struct list_node n;
};

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			struct list_head list;
			//struct multilingual_service_node
		};
	};
}multilingual_service_name_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t component_tag;
			struct list_head list;
			//struct multilingual_info* time_list;
		};
	};

}multilingual_component_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint32_t private_data_specifier;
		};
	};
}private_data_specifier_descriptor_t;

typedef struct{
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint16_t new_original_network_id;
			uint16_t new_transport_stream_id;
			uint16_t new_service_id;
		};
	};
}service_move_descriptor_t;

typedef struct{
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t sb_size:2;
			uint8_t sb_leak_rate:6;
			uint8_t *DVB_reserved;
		};
	};
}short_smoothing_buffer_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t reserved_future_use:6;
			uint8_t coding_type:2;
			uint32_t * centre_frequency;
		};
	};
}frequency_list_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint64_t DVB_reserved_future_use:2;
			uint64_t peak_rate:22;
			uint64_t DVB_reserved_future_use1:2;
			uint64_t minimum_overall_smoothing_rate:22;
			uint64_t DVB_reserved_future_use2:2;
			uint64_t maximum_overall_smoothing_buffer:14;
		};
	};
}partial_transport_stream_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint16_t data_broadcast_id;
			uint8_t component_tag;
			uint8_t selector_length;
			uint8_t *selector_byte;
			uint32_t ISO_639_language_code:24;
			uint32_t text_length:8;
			uint8_t *text_char;
		};
	};
}data_broadcast_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t scrambling_mode;
		};
	};
}scrambling_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint16_t data_broadcast_id;
			uint8_t *id_selector_byte;
		};
	};
}data_broadcast_id_descriptor_t;


typedef struct{ 
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t *byte;
		};
	};
}transport_stream_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t * byte;
		};
	};
}DSNG_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint24_t programme_identification_label;//20bit
		};
	};
}PDC_descriptor_t;


typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t component_type_flag:1;
			uint8_t bsid_flag:1;
			uint8_t mainid_flag:1;
			uint8_t asvc_flag:1;
			uint8_t reserved_flags:4;
			uint8_t component_type;
			uint8_t bsid;
			uint8_t mainid;
			uint8_t asvc;
			uint8_t *additional_info_byte;
		};
	};

}AC3_descriptor_t;

/*ancillary_data_descriptor*/
struct ancillary_data_identifier
{
	uint8_t DVD_video_ancillary_data:1;
	uint8_t extended_ancillary_data:1;
	uint8_t announcement_switching_data:1;
	uint8_t DAB_ancillary_data:1;
	uint8_t scale_factor_error_check:1;
	uint8_t MPEG4_ancillary_data:1;
	uint8_t RDS_via_UECP:1;
	uint8_t reserved:1;
};

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			struct ancillary_data_identifier identifier;
		};
	};
}ancillary_data_descriptor_t;


struct subcell_list_node{
	uint64_t cell_id_extension:8;
	uint64_t subcell_latitude:16;
	uint64_t subcell_longitude:16;
	uint64_t subcell_extent_of_latitude:12;
	uint64_t subcell_extent_of_longitude:12;
	struct list_node n;
};

struct cell_list_node{
	uint16_t cell_id;
	uint16_t cell_latitude;
	uint16_t cell_longitude;
	uint32_t cell_extent_of_latitude:12;
	uint32_t cell_extent_of_longitude:12;
	uint32_t subcell_info_loop_length:8;
	//struct subcell_list_info *subcell_list;
	struct list_head list;
	struct list_node n;
};

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			struct list_head list;
			//struct cell_list_info * cell_list;
		};
	};
} cell_list_descriptor_t;

struct subcell_node{
	uint8_t cell_id_extension;
	uint32_t transposer_frequency;
	struct list_node n;
	//struct subcell_info *next;
};

struct cell_frequency_node{
	uint16_t cell_id;
	uint32_t frequency;
	uint8_t subcell_info_loop_length;
	//struct subcell_info *subcell_info_list;
	struct list_head list;
};

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			struct list_head list;
			//struct cell_frequency_info *cell_info_list;
		};
	};
}cell_frequency_link_descriptor_t;


/*Announcement support descriptor*/
struct announcement_support_indicator {
	uint16_t emergency_alarm:1;
	uint16_t road_traffic_flash:1;
	uint16_t public_transport_flash:1;
	uint16_t warning_message:1;
	uint16_t news_flash:1;
	uint16_t weather_flash:1;
	uint16_t event_announcement:1;
	uint16_t personal_call:1;
	uint16_t reserved:8;
};

struct reference{
	uint16_t original_network_id;
	uint16_t transport_stream_id;
	uint16_t service_id;
	uint8_t component_tag;
};

struct announcement_node {
	uint8_t announcement_type:4;
	uint8_t reserved_future_use:1;
	uint8_t reference_type:3;
	struct reference ref;
	struct list_node n;
	//struct announcement_info * next;
};

typedef struct {
	EXT_STD_C11
	union{
	descriptor_t descriptor;
	struct {
		uint8_t descriptor_tag;
		uint8_t descriptor_length;
		struct list_node n;
		struct announcement_support_indicator indicator;
		struct list_head list;
		//struct announcement_info *info;
		};
	};
}announcement_support_descriptor_t;


typedef struct{
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
		};
	};

}application_signalling_descriptor_t;

/*adaptation_field_data_descriptor*/
struct adaptation_field_data_identifier
{
	uint8_t announcement_switching_data:1;
	uint8_t AU_information_data:1;
	uint8_t PVR_assist_information_data:1;
	uint8_t tsap_timeline:1;
	uint8_t reserved:4;
};

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			struct adaptation_field_data_identifier identifier;
		};
	};
}adaptation_field_data_descriptor_t;

typedef struct{
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
		};
	};
}service_identifier_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t availability_flag:1;
			uint8_t reserved:7;
			uint16_t *cell_id;
		};
	};
}service_availability_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
		};
	};

}default_authority_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
		};
	};

} related_content_descriptor_t;

typedef struct{
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
		};
	};

}TVA_id_descriptor_t;


typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
		};
	};

}content_identifier_descriptor_t;

typedef struct{
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
		};
	};

}time_slice_fec_identifier_descriptor_t;

typedef struct{
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
		};
	};

}ECM_repetition_rate_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t scrambling_sequence_selector:1;
			uint8_t multiple_input_stream_flag:1;
			uint8_t backwards_compatibility_indicator:1;
			uint8_t reserved_future_use:5;
			uint32_t reserved:6;
			uint32_t scrambling_sequence_index:18;
			uint32_t input_stream_identifier:8;
		};
	};
}S2_satellite_delivery_system_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t component_type_flag:1;
			uint8_t bsid_flag:1;
			uint8_t mainid_flag:1;
			uint8_t asvc_flag:1;
			uint8_t mixinfoexists:1;
			uint8_t substream1_flag:1;
			uint8_t substream2_flag:1;
			uint8_t substream3_flag:1;
			uint8_t component_type;
			uint8_t bsid;
			uint8_t mainid;
			uint8_t asvc;
			uint8_t substream1;
			uint8_t substream2;
			uint8_t substream3;
			uint8_t *additional_info_byte;
		};
	};
}enhanced_AC3_descriptor_t;


typedef struct{
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			//40bits
			uint64_t sample_rate_code:4;
			uint64_t bit_rate_code:6;
			uint64_t nblks:7;
			uint64_t fsize:14;
			uint64_t surround_mode:6;
			uint64_t lfe_flag:1;
			uint64_t extended_surround_flag:2;
			uint8_t *additional_info_byte;
		};
	};
}DTS_descriptor_t;


typedef struct{
	EXT_STD_C11
		union{
			descriptor_t descriptor;
			struct {
				uint8_t descriptor_tag;
				uint8_t descriptor_length;
				struct list_node n;
				uint8_t profile_and_level;
				//valid when descriptor_length >1
				uint8_t AAC_type_flag:1;
				uint8_t SAOC_DE_flag:1;
				uint8_t reserved_future_use:6;
				uint8_t AAC_type;
				uint8_t *additional_info_byte;
			};
		};
}AAC_descriptor_t;
	
typedef struct{
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
		};
	};
}XAIT_location_descriptor_t;

typedef struct { /*0x7E*/
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t user_defined:1;
			uint8_t reserved_future_use:3;
			uint8_t do_not_scramble:1;
			uint8_t control_remote_access_over_internet:2;
			uint8_t do_not_apply_revocation:1;
		};
	};
}FTA_content_management_descriptor_t;

typedef struct {
	EXT_STD_C11
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			struct list_node n;
			uint8_t descriptor_tag_extension;
			uint8_t *selector_byte;
		};
	};
}extension_descriptor_t;

/*externsion_descriptors */
enum extension_tag{
	image_icon = 0x0,
	cpcm_delivery_signalling= 0x1,
	CP = 0x2,
	CP_identifier = 0x3,
	T2_delivery_system = 0x4,
	SH_delivery_system = 0x5,
	supplementary_audio = 0x6,
	network_change_notify = 0x7,
	message = 0x8,
	target_region = 0x9,
	target_region_name = 0xA,
	service_relocated = 0xB,
	XAIT_PID = 0xC,
	C2_delivery_system = 0xD,
	DTS_HD_audio_stream = 0xE,
	DTS_Neural = 0xF,
	video_depth_range = 0x10,
	T2MI = 0x11,
	/* 0x12 reserved*/
	URI_linkage = 0x13,
	CI_ancillary_data = 0x14,
	AC_4 = 0x15,
	C2_bundle_delivery_system = 0x16,
	/*0x17 - 0x7F  reserved*/
	/*0x80 - 0xFF  user defined */
};


struct descriptor_ops{
	uint8_t tag;
	char tag_name[64];
	int ( *descriptor_parse)(uint8_t *data, uint32_t len,void *ptr);
	void* (*descriptor_alloc)(void);
	void (*descriptor_free)(descriptor_t* ptr);
	void (* descriptor_dump)(descriptor_t* ptr);
};

void init_descriptor_parsers();

void free_descriptors(struct list_head *list);

void dump_descriptors(const char* str, struct list_head* list);

void parse_descriptors(struct list_head *h,uint8_t *buf, int len);


#ifdef __cplusplus
}
#endif

#endif /* _DESCRIPTOR_H_ */
