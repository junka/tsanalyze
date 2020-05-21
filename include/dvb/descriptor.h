#ifndef _DVB_DESCRIPTOR_H_
#define _DVB_DESCRIPTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"
#include "types.h"

#define foreach_enum_dvb_descriptor                                                                                    \
	_(network_name, 0x40)                                                                                              \
	_(service_list, 0x41)                                                                                              \
	_(stuffing, 0x42)                                                                                                  \
	_(satellite_delivery_system, 0x43)                                                                                 \
	_(cable_delivery_system, 0x44)                                                                                     \
	_(VBI_data, 0x45)                                                                                                  \
	_(VBI_teletext, 0x46)                                                                                              \
	_(bouquet_name, 0x47)                                                                                              \
	_(service, 0x48)                                                                                                   \
	_(country_availability, 0x49)                                                                                      \
	_(linkage, 0x4A)                                                                                                   \
	_(NVOD_reference, 0x4B)                                                                                            \
	_(time_shifted_service, 0x4C)                                                                                      \
	_(short_event, 0x4D)                                                                                               \
	_(extended_event, 0x4E)                                                                                            \
	_(time_shifted_event, 0x4F)                                                                                        \
	_(component, 0x50)                                                                                                 \
	_(mosaic, 0x51)                                                                                                    \
	_(stream_identifier, 0x52)                                                                                         \
	_(CA_identifier, 0x53)                                                                                             \
	_(content, 0x54)                                                                                                   \
	_(parental_rating, 0x55)                                                                                           \
	_(teletext, 0x56)                                                                                                  \
	_(telephone, 0x57)                                                                                                 \
	_(local_time_offset, 0x58)                                                                                         \
	_(subtitling, 0x59)                                                                                                \
	_(terrestrial_delivery_system, 0x5A)                                                                               \
	_(multilingual_network_name, 0x5B)                                                                                 \
	_(multilingual_bouquet_name, 0x5C)                                                                                 \
	_(multilingual_service_name, 0x5D)                                                                                 \
	_(multilingual_component, 0x5E)                                                                                    \
	_(private_data_specifier, 0x5F)                                                                                    \
	_(service_move, 0x60)                                                                                              \
	_(short_smoothing_buffer, 0x61)                                                                                    \
	_(frequency_list, 0x62)                                                                                            \
	_(partial_transport_stream, 0x63)                                                                                  \
	_(data_broadcast, 0x64)                                                                                            \
	_(scrambling, 0x65)                                                                                                \
	_(data_broadcast_id, 0x66)                                                                                         \
	_(transport_stream, 0x67)                                                                                          \
	_(DSNG, 0x68)                                                                                                      \
	_(PDC, 0x69)                                                                                                       \
	_(AC3, 0x6A)                                                                                                       \
	_(ancillary_data, 0x6B)                                                                                            \
	_(cell_list, 0x6C)                                                                                                 \
	_(cell_frequency_link, 0x6D)                                                                                       \
	_(announcement_support, 0x6E)                                                                                      \
	_(application_signalling, 0x6F)                                                                                    \
	_(adaptation_field_data, 0x70)                                                                                     \
	_(service_identifier, 0x71)                                                                                        \
	_(service_availability, 0x72)                                                                                      \
	_(default_authority, 0x73)                                                                                         \
	_(related_content, 0x74)                                                                                           \
	_(TVA_id, 0x75)                                                                                                    \
	_(content_identifier, 0x76)                                                                                        \
	_(time_slice_fec_identifier, 0x77)                                                                                 \
	_(ECM_repetition_rate, 0x78)                                                                                       \
	_(S2_satellite_delivery_system, 0x79)                                                                              \
	_(enhanced_AC3, 0x7A)                                                                                              \
	_(DTS, 0x7B)                                                                                                       \
	_(AAC, 0x7C)                                                                                                       \
	_(XAIT_location, 0x7D)                                                                                             \
	_(FTA_content_management, 0x7E)                                                                                    \
	_(extension, 0x7F)

/*see EN_300 468 chapter 6*/
typedef struct {
	descriptor_t descriptor;
	uint8_t *text_byte;

} network_name_descriptor_t;

typedef struct {
	descriptor_t descriptor;

} service_list_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint8_t *stuffing_byte;

} stuffing_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint32_t frequency;
	uint16_t orbital_position;
	uint8_t west_east_flag : 1;
	uint8_t polarization : 2;
	uint8_t roll_off : 2;
	uint8_t modulation_system : 1;
	uint8_t modulation_type : 2;
	uint32_t symbol_rate : 28;
	uint32_t FEC_inner : 4;

} satellite_delivery_system_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint32_t frequency;
	uint16_t reserved_future_use : 12;
	uint16_t FEC_outer : 4;
	uint8_t modulation;
	uint32_t symbol_rate : 28;
	uint32_t FEC_inner : 4;

} cable_delivery_system_descriptor_t;

struct VBI_data_node {
	uint8_t data_service_id;
	uint8_t data_service_descriptor_length;
	uint8_t *reserved;
	struct list_node n;
};

typedef struct {
	descriptor_t descriptor;
	struct list_head list;
	// struct VBI_data_info *list;

} VBI_data_descriptor_t;

struct VBI_teletext_node {
	uint32_t ISO_639_language_code : 24;
	uint32_t teletext_type : 5;
	uint32_t teletext_magazine_number : 3;
	uint8_t teletext_page_number;
	struct list_node n;
};

typedef struct {
	descriptor_t descriptor;
	struct list_head list;
	// struct VBI_teletext_info* list;

} VBI_teletext_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint8_t *sub_table;

} bouquet_name_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint8_t service_type;
	uint8_t service_provider_name_length;
	uint8_t *provider_name;
	uint8_t service_name_length;
	uint8_t *service_name;

} service_descriptor_t;

enum country_code_e {
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

struct country_code_node {
	uint24_t country_code;
	// struct country_code * next;
	struct list_node n;
};

typedef struct {
	descriptor_t descriptor;
	uint8_t country_availability_flag : 1;
	uint8_t reserved_future_use : 7;
	struct list_head list;
	// struct country_code* country_list;
} country_availability_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint16_t transport_stream_id;
	uint16_t original_network_id;
	uint16_t service_id;
	uint8_t linkage_type;
	// TODO
	uint8_t *private_data_byte;

} linkage_descriptor_t;

struct NVOD_reference_node {
	uint16_t transport_stream_id;
	uint16_t original_network_id;
	uint16_t service_id;
	struct list_node n;
};

typedef struct {
	descriptor_t descriptor;
	struct list_head list;
	// struct NVOD_refer *nvod_list;

} NVOD_reference_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint8_t reference_service_id;

} time_shifted_service_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint32_t ISO_639_language_code : 24;
	uint32_t event_name_length : 8;
	uint8_t *event_name_char;
	uint8_t text_length;
	uint8_t *text_char;

} short_event_descriptor_t;

struct event_item_node {
	uint8_t item_description_length;
	uint8_t *item_description_char;
	uint8_t item_length;
	uint8_t *item_char;
	struct list_node n;
};

typedef struct {
	descriptor_t descriptor;
	uint8_t descriptor_number : 4;
	uint8_t last_descriptor_number : 4;
	uint32_t ISO_639_language_code : 24;
	uint32_t length_of_items : 8;
	struct list_head list;
	// struct event_item item_list;
	uint8_t text_length;
	uint8_t *text_char;

} extended_event_descriptor_t;

typedef struct {
	descriptor_t descriptor;

} time_shifted_event_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint8_t stream_content_ext : 4;
	uint8_t stream_content : 4;
	uint8_t component_type;
	uint32_t component_tag : 8;
	uint32_t ISO_639_language_code : 24;
	uint8_t *text_char;

} component_descriptor_t;

typedef struct {
	descriptor_t descriptor;

} mosaic_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint8_t component_tag;

} stream_identifier_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint16_t *CA_system_id; /*ETSI TS 101 162 [i.1]*/

} CA_identifier_descriptor_t;

struct content_node {
	uint8_t content_nibble_level_1 : 4;
	uint8_t content_nibble_level_2 : 4;
	uint8_t byte;
	struct list_node n;
};

typedef struct {
	descriptor_t descriptor;
	struct list_head list;
	// struct content_info * content_list;

} content_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint32_t *countrycode_and_rating;

} parental_rating_descriptor_t;

enum teletext_type {
	teletext_reserved = 0x0,
	teletext_initial_page = 0x1,
	teletext_subtitle_page = 0x2,
	additional_information_page = 0x3,
	programme_schedule_page = 0x4,
	hearing_impaired_page = 0x5,
	/*0x06 to 0x1F reserved*/
};

struct teletext_node {
	uint32_t ISO_639_language_code : 24;
	uint32_t teletext_type : 5;
	uint32_t teletext_magazine_number : 3;
	uint8_t teletext_page_number;
	struct list_node n;
};

typedef struct {
	descriptor_t descriptor;
	struct list_head list;

} teletext_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint8_t reserved_future_use : 2;
	uint8_t foreign_availability : 1;
	uint8_t connection_type : 5;
	uint8_t reserved_future_use1 : 1;
	uint8_t country_prefix_length : 2;
	uint8_t international_area_code_length : 3;
	uint8_t operator_code_length : 2;
	uint8_t reserved_future_use2 : 1;
	uint8_t national_area_code_length : 3;
	uint8_t core_number_length : 4;
	uint8_t *country_prefix_char;
	uint8_t *international_area_code_char;
	uint8_t *operator_code_char;
	uint8_t *national_area_code_char;
	uint8_t *core_number_char;

} telephone_descriptor_t;

struct local_time_node {
	uint32_t country_code : 24;
	uint32_t country_region_id : 6;
	uint32_t reserved : 1;
	uint32_t local_time_offset_polarity : 1;
	uint16_t local_time_offset;
	uint40_t time_of_change;
	uint16_t next_time_offset;
	struct list_node n;
};

typedef struct {
	descriptor_t descriptor;
	struct list_head list;
	// struct local_time_info* time_list;

} local_time_offset_descriptor_t;

struct subtitling_node {
	uint32_t ISO_639_language_code : 24;
	uint32_t subtitling_type : 8;
	uint16_t composition_page_id;
	uint16_t ancillary_page_id;
	struct list_node n;
};

typedef struct {
	descriptor_t descriptor;
	struct list_head list;
	// struct subtitling_info* subtitle_list;

} subtitling_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint32_t centre_frequency;
	uint8_t bandwidth : 3;
	uint8_t priority : 1;
	uint8_t time_slicing_indicator : 1;
	uint8_t MPE_FEC_indicator : 1;
	uint8_t reserved_future_use : 2;
	uint8_t constellation : 2;
	uint8_t hierarchy_information : 3;
	uint8_t code_rate_HP_stream : 3;
	uint8_t code_rate_LP_stream : 3;
	uint8_t guard_interval : 2;
	uint8_t transmission_mode : 2;
	uint8_t other_frequency_flag : 1;
	uint32_t reserved_future_use1;

} terrestrial_delivery_system_descriptor_t;

struct multilingual_node {
	uint32_t ISO_639_language_code : 24;
	uint32_t name_length : 8;
	uint8_t *text_char;
	struct list_node n;
};

typedef struct {
	descriptor_t descriptor;
	struct list_head list;
	// struct multilingual_info* time_list;

} multilingual_network_name_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	struct list_head list;
	// struct multilingual_info* time_list;

} multilingual_bouquet_name_descriptor_t;

struct multilingual_service_node {
	uint32_t ISO_639_language_code : 24;
	uint32_t name_length : 8;
	uint8_t *text_char;
	uint8_t service_name_length;
	uint8_t *service_char;
	struct list_node n;
};

typedef struct {
	descriptor_t descriptor;
	struct list_head list;
	// struct multilingual_service_node

} multilingual_service_name_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint8_t component_tag;
	struct list_head list;
	// struct multilingual_info* time_list;

} multilingual_component_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint32_t private_data_specifier;

} private_data_specifier_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint16_t new_original_network_id;
	uint16_t new_transport_stream_id;
	uint16_t new_service_id;

} service_move_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint8_t sb_size : 2;
	uint8_t sb_leak_rate : 6;
	uint8_t *DVB_reserved;

} short_smoothing_buffer_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint8_t reserved_future_use : 6;
	uint8_t coding_type : 2;
	uint32_t *centre_frequency;

} frequency_list_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint64_t DVB_reserved_future_use : 2;
	uint64_t peak_rate : 22;
	uint64_t DVB_reserved_future_use1 : 2;
	uint64_t minimum_overall_smoothing_rate : 22;
	uint64_t DVB_reserved_future_use2 : 2;
	uint64_t maximum_overall_smoothing_buffer : 14;

} partial_transport_stream_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint16_t data_broadcast_id;
	uint8_t component_tag;
	uint8_t selector_length;
	uint8_t *selector_byte;
	uint32_t ISO_639_language_code : 24;
	uint32_t text_length : 8;
	uint8_t *text_char;

} data_broadcast_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint8_t scrambling_mode;

} scrambling_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint16_t data_broadcast_id;
	uint8_t *id_selector_byte;

} data_broadcast_id_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint8_t *byte;

} transport_stream_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint8_t *byte;

} DSNG_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint24_t programme_identification_label; // 20bit

} PDC_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint8_t component_type_flag : 1;
	uint8_t bsid_flag : 1;
	uint8_t mainid_flag : 1;
	uint8_t asvc_flag : 1;
	uint8_t reserved_flags : 4;
	uint8_t component_type;
	uint8_t bsid;
	uint8_t mainid;
	uint8_t asvc;
	uint8_t *additional_info_byte;
} AC3_descriptor_t;

/*ancillary_data_descriptor*/
struct ancillary_data_identifier {
	uint8_t DVD_video_ancillary_data : 1;
	uint8_t extended_ancillary_data : 1;
	uint8_t announcement_switching_data : 1;
	uint8_t DAB_ancillary_data : 1;
	uint8_t scale_factor_error_check : 1;
	uint8_t MPEG4_ancillary_data : 1;
	uint8_t RDS_via_UECP : 1;
	uint8_t reserved : 1;
};

typedef struct {
	descriptor_t descriptor;
	struct ancillary_data_identifier identifier;
} ancillary_data_descriptor_t;

struct subcell_list_node {
	uint64_t cell_id_extension : 8;
	uint64_t subcell_latitude : 16;
	uint64_t subcell_longitude : 16;
	uint64_t subcell_extent_of_latitude : 12;
	uint64_t subcell_extent_of_longitude : 12;
	struct list_node n;
};

struct cell_list_node {
	uint16_t cell_id;
	uint16_t cell_latitude;
	uint16_t cell_longitude;
	uint32_t cell_extent_of_latitude : 12;
	uint32_t cell_extent_of_longitude : 12;
	uint32_t subcell_info_loop_length : 8;
	// struct subcell_list_info *subcell_list;
	struct list_head list;
	struct list_node n;
};

typedef struct {
	descriptor_t descriptor;
	struct list_head list;
	// struct cell_list_info * cell_list;

} cell_list_descriptor_t;

struct subcell_node {
	uint8_t cell_id_extension;
	uint32_t transposer_frequency;
	struct list_node n;
	// struct subcell_info *next;
};

struct cell_frequency_node {
	uint16_t cell_id;
	uint32_t frequency;
	uint8_t subcell_info_loop_length;
	// struct subcell_info *subcell_info_list;
	struct list_head list;
};

typedef struct {
	descriptor_t descriptor;
	struct list_head list;
	// struct cell_frequency_info *cell_info_list;

} cell_frequency_link_descriptor_t;

/*Announcement support descriptor*/
struct announcement_support_indicator {
	uint16_t emergency_alarm : 1;
	uint16_t road_traffic_flash : 1;
	uint16_t public_transport_flash : 1;
	uint16_t warning_message : 1;
	uint16_t news_flash : 1;
	uint16_t weather_flash : 1;
	uint16_t event_announcement : 1;
	uint16_t personal_call : 1;
	uint16_t reserved : 8;
};

struct reference {
	uint16_t original_network_id;
	uint16_t transport_stream_id;
	uint16_t service_id;
	uint8_t component_tag;
};

struct announcement_node {
	uint8_t announcement_type : 4;
	uint8_t reserved_future_use : 1;
	uint8_t reference_type : 3;
	struct reference ref;
	struct list_node n;
	// struct announcement_info * next;
};

typedef struct {
	descriptor_t descriptor;
	struct announcement_support_indicator indicator;
	struct list_head list;
	// struct announcement_info *info;

} announcement_support_descriptor_t;

typedef struct {
	descriptor_t descriptor;

} application_signalling_descriptor_t;

/*adaptation_field_data_descriptor*/
struct adaptation_field_data_identifier {
	uint8_t announcement_switching_data : 1;
	uint8_t AU_information_data : 1;
	uint8_t PVR_assist_information_data : 1;
	uint8_t tsap_timeline : 1;
	uint8_t reserved : 4;
};

typedef struct {
	descriptor_t descriptor;
	struct adaptation_field_data_identifier identifier;

} adaptation_field_data_descriptor_t;

typedef struct {
	descriptor_t descriptor;

} service_identifier_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint8_t availability_flag : 1;
	uint8_t reserved : 7;
	uint16_t *cell_id;

} service_availability_descriptor_t;

typedef struct {
	descriptor_t descriptor;

} default_authority_descriptor_t;

typedef struct {
	descriptor_t descriptor;

} related_content_descriptor_t;

typedef struct {
	descriptor_t descriptor;

} TVA_id_descriptor_t;

typedef struct {
	descriptor_t descriptor;

} content_identifier_descriptor_t;

typedef struct {
	descriptor_t descriptor;

} time_slice_fec_identifier_descriptor_t;

typedef struct {
	descriptor_t descriptor;

} ECM_repetition_rate_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint8_t scrambling_sequence_selector : 1;
	uint8_t multiple_input_stream_flag : 1;
	uint8_t backwards_compatibility_indicator : 1;
	uint8_t reserved_future_use : 5;
	uint32_t reserved : 6;
	uint32_t scrambling_sequence_index : 18;
	uint32_t input_stream_identifier : 8;
} S2_satellite_delivery_system_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint8_t component_type_flag : 1;
	uint8_t bsid_flag : 1;
	uint8_t mainid_flag : 1;
	uint8_t asvc_flag : 1;
	uint8_t mixinfoexists : 1;
	uint8_t substream1_flag : 1;
	uint8_t substream2_flag : 1;
	uint8_t substream3_flag : 1;
	uint8_t component_type;
	uint8_t bsid;
	uint8_t mainid;
	uint8_t asvc;
	uint8_t substream1;
	uint8_t substream2;
	uint8_t substream3;
	uint8_t *additional_info_byte;

} enhanced_AC3_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	// 40bits
	uint64_t sample_rate_code : 4;
	uint64_t bit_rate_code : 6;
	uint64_t nblks : 7;
	uint64_t fsize : 14;
	uint64_t surround_mode : 6;
	uint64_t lfe_flag : 1;
	uint64_t extended_surround_flag : 2;
	uint8_t *additional_info_byte;
} DTS_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint8_t profile_and_level;
	// valid when descriptor_length >1
	uint8_t AAC_type_flag : 1;
	uint8_t SAOC_DE_flag : 1;
	uint8_t reserved_future_use : 6;
	uint8_t AAC_type;
	uint8_t *additional_info_byte;

} AAC_descriptor_t;

typedef struct {
	descriptor_t descriptor;

} XAIT_location_descriptor_t;

typedef struct { /*0x7E*/
	descriptor_t descriptor;
	uint8_t user_defined : 1;
	uint8_t reserved_future_use : 3;
	uint8_t do_not_scramble : 1;
	uint8_t control_remote_access_over_internet : 2;
	uint8_t do_not_apply_revocation : 1;

} FTA_content_management_descriptor_t;

typedef struct {
	descriptor_t descriptor;
	uint8_t descriptor_tag_extension;
	uint8_t *selector_byte;

} extension_descriptor_t;

/*externsion_descriptors */
enum extension_tag {
	image_icon = 0x0,
	cpcm_delivery_signalling = 0x1,
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

#ifdef __cplusplus
}
#endif

#endif /* _DVB_DESCRIPTOR_H_*/