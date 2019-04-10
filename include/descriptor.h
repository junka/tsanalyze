#ifndef _DESCRIPTOR_H_
#define _DESCRIPTOR_H_

#ifdef __cplusplus
extern "C"{
#endif

typedef struct descriptor{
	uint8_t tag;
	uint8_t length;
	struct descriptor * next;
	uint8_t data[0];
}descriptor_t;

enum descriptor_e{
	/* ISO/IEC 13818-1 */
	video_stream_descriptor		= 0x02,
	audio_stream_descriptor		= 0x03,
	hierarchy_descriptor		= 0x04,
	registration_descriptor		= 0x05,
	data_stream_alignment_descriptor = 0x06,
	target_background_grid_descriptor = 0x07,
	video_window_descriptor		= 0x08,
	CA_descriptor				= 0x09,
	ISO_639_language_descriptor	= 0x0A,
	system_clock_descriptor		= 0x0B,
	multiplex_buffer_utilization_descriptor = 0x0C,
	copyright_descriptor		= 0x0D,
	maximum_bitrate_descriptor	= 0x0E,
	private_data_indicator_descriptor = 0x0F,
	smoothing_buffer_descriptor	= 0x10,
	STD_descriptor				= 0x11,
	ibp_descriptor				= 0x12,
	/* 0x12 - 0x1A Defined in ISO/IEC 13818-6*/
	
	MPEG4_video_descriptor		= 0x1B,
	MPEG4_audio_descriptor		= 0x1C,
	IOD_descriptor				= 0x1D,
	SL_descriptor				= 0x1E,
	FMC_descriptor				= 0x1F,
	external_ES_ID_descriptor	= 0x20,
	muxcode_descriptor			= 0x21,
	FmxBufferSize_descriptor	= 0x22,
	MultiplexBuffer_descriptor	= 0x23,
	/* 0x24 - 0x3F reserved */

	/* EN 300 468*/
	network_name_descriptor		= 0x40,
	service_list_descriptor 	= 0x41,
	stuffing_descriptor			= 0x42,
	satellite_delivery_system_descriptor = 0x43,
	cable_delivery_system_descriptor = 0x44,
	VBI_data_descriptor			= 0x45,
	VBI_teletext_descripto		= 0x46,
	bouquet_name_descriptor		= 0x47,
	service_descriptor			= 0x48,
	country_availability_descriptor = 0x49,
	linkage_descriptor			= 0x4A,
	NVOD_reference_descriptor	= 0x4B,
	time_shifted_service_descriptor = 0x4C,
	short_event_descriptor		= 0x4D,
	extended_event_descriptor	= 0x4E,
	time_shifted_event_descriptor = 0x4F,
	component_descriptor		= 0x50,
	mosaic_descriptor			= 0x51,
	stream_identifier_descriptor	= 0x52,
	CA_identifier_descriptor	= 0x53,
	content_descriptor			= 0x54,
	parental_rating_descriptor	= 0x55,
	teletext_descriptor			= 0x56,
	telephone_descriptor		= 0x57,
	local_time_offset_descriptor	= 0x58,
	subtitling_descriptor		= 0x59,
	terrestrial_delivery_system_descriptor = 0x5A,
	multilingual_network_name_descriptor =0x5B,
	multilingual_bouquet_name_descriptor = 0x5C,
	multilingual_service_name_descriptor = 0x5D,
	multilingual_component_descriptor = 0x5E,
	private_data_specifier_descriptor = 0x5F,
	service_move_descriptor		= 0x60,
	short_smoothing_buffer_descriptor = 0x61,
	frequency_list_descriptor	= 0x62,
	partial_transport_stream_descriptor = 0x63,
	data_broadcast_descriptor	= 0x64,
	scrambling_descriptor		= 0x65,
	data_broadcast_id_descriptor	= 0x66,
	transport_stream_descriptor	= 0x67,
	DSNG_descriptor 			= 0x68,
	PDC_descriptor				= 0x69,
	AC3_descriptor				= 0x6A,
	ancillary_data_descriptor	= 0x6B,
	cell_list_descriptor		= 0x6C,
	cell_frequency_link_descriptor	= 0x6D,
	announcement_support_descriptor	= 0x6E,
	application_signalling_descriptor = 0x6F,
	adaptation_field_data_descriptor = 0x70,
	service_identifier_descriptor	= 0x71,
	service_availability_descriptor = 0x72,
	default_authority_descriptor 	= 0x73,
	related_content_descriptor	= 0x74,
	TVA_id_descriptor			= 0x75,
	content_identifier_descriptor	= 0x76,
	time_slice_fec_identifier_descriptor = 0x77,
	ECM_repetition_rate_descriptor = 0x78,
	S2_satellite_delivery_system_descriptor = 0x79,
	enhanced_AC3_descriptor		= 0x7A,
	DTS_descriptor				= 0x7B,
	AAC_descriptor				= 0x7C,
	XAIT_location_descriptor	= 0x7D,
	FTA_content_management_descriptor = 0x7E,
	extension_descriptor		= 0x7F,

	/*0x80 to 0xFE user defined */
	/*0xFF forbidden */
	
};


/* see ISO/IEC 13818-1 chapter 2.6*/
struct video_stream_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
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
};

struct audio_stream_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
			uint8_t free_format_flag:1;
			uint8_t ID:1;
			uint8_t layer:2;
			uint8_t variable_rate_audio_indicator:1;
			uint8_t reserved:3;
		};
	};
};

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

struct hierarchy_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
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

};

struct registration_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
			uint32_t format_identifier;
			uint8_t* additional_identification_info;
		};
	};
};

enum video_alignment_type_e{
	slice_or_video_access_unit = 1,
	video_access_unit = 2,
	GOP_or_SEQ = 3,
	SEQ = 4,
	/* 0x05 - 0xFF reserved*/
};

struct data_stream_alignment_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
			uint8_t alignment_type; /* see definition in @video_alignment_type_e */
		};
	};
};

struct target_background_grid_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void * next;
			uint32_t horizontal_size:14;
			uint32_t vertical_size:14;
			uint32_t aspect_ratio_information:4;
		};
	};
};

struct video_window_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
			uint32_t horizontal_offset:14;
			uint32_t vertical_offset:14;
			uint32_t window_priority:4;
		};
	};
};

struct CA_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
			uint16_t CA_system_ID;
			uint16_t reserved:3;
			uint16_t CA_PID:13;
			uint8_t* private_data_byte;
		};
	};
};

enum audio_type_e{
	clean_effects = 0x1,
	hearing_impaired = 0x2,
	visual_impaired_commentary = 0x3,
	/*0x04-0xFF reserved*/
};

struct language_info{
	uint32_t ISO_639_language_code:24;
	uint32_t audio_type:8; /*see definition in @audio_type_e */
	struct language_info * next;
};

struct ISO_639_language_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
			struct language_info* language_list;
		};
	};
};

struct system_clock_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
			uint8_t external_clock_reference_indicator:1;
			uint8_t reserved:1;
			uint8_t clock_accuracy_integer:6;
			uint8_t clock_accuracy_exponent:3;
			uint8_t reserved:5;
		};
	};
};

struct multiplex_buffer_utilization_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
			uint16_t bound_valid_flag:1;
			uint16_t LTW_offset_lower_bound:15;
			uint16_t reserved:1;
			uint16_t LTW_offset_lower_bound:15;
		};
	};
};

struct copyright_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
			uint32_t copyright_identifier;
			uint8_t *additional_copyright_info;
		};
	};
};

struct maximum_bitrate_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
			uint24_t reserved:2;
			uint24_t maximum_bitrate:22;
		};
	};
};

struct private_data_indicator_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
			uint32_t private_data_indicator;
		};
	};
};

struct smoothing_buffer_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
			uint24_t reserved:2;
			uint24_t sb_leak_rate:22;
			uint24_t reserved1:2;
			uint23_t sb_size:22;
		};
	};
};

struct STD_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
			uint8_t reserved:7;
			uint8_t leak_valid_flag:1;
		};
	};
};

struct ibp_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
			uint16_t closed_gop_flag:1;
			uint16_t identical_gop_flag:1;
			uint14_t max_gop_length:14;
		};
	};
};

struct MPEG4_video_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
			uint8_t MPEG4_visual_profile_and_level;
		};
	};
};

enum MPEG4_audio_profile_and_level_e{
	main_profile_lv1 = 0x10,
	main_profile_lv2 = 0x11,
	main_profile_lv3 = 0x12,
	main_profile_lv4 = 0x13,
	/*0x14-0x17 reserved */
	
	
};

struct MPEG4_audio_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
			uint8_t MPEG4_audio_profile_and_level;
		};
	};
};

struct IOD_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
			uint8_t Scope_of_IOD_label;
			uint8_t IOD_label;
		};
	};
};

struct SL_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
			uint16_t ES_ID;
		};
	};
};

struct FMC_descriptor_t{
};

struct external_ES_ID_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void *next;
			uint16_t external_ES_ID;
		};
	};
};

struct muxcode_descriptor_t{
};

struct FmxBufferSize_descriptor_t{
	
};

struct MultiplexBuffer_descriptor_t{
	uint64_t descriptor_tag:8;
	uint64_t descriptor_length:8;
	uint64_t MB_buffer_size:24;
	uint64_t TB_leak_rate:24;
};

/*see EN_300 468 chapter 6*/

/*adaptation_field_data_descriptor*/
struct adaptation_field_data_identifier
{
	uint8_t announcement_switching_data:1;
	uint8_t AU_information_data:1;
	uint8_t PVR_assist_information_data:1;
	uint8_t tsap_timeline:1;
	uint8_t reserved:4;
};
struct adaptation_field_data_descriptor_t {
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void * next;
			struct adaptation_field_data_identifier identifier;
		};
};

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

struct ancillary_data_descriptor_t {
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void * next;
			struct ancillary_data_identifier identifier;
		};
	};
};


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

struct announcement_info {
	uint8_t announcement_type:4;
	uint8_t reserved_future_use:1;
	uint8_t reference_type:3;
	struct reference ref;
	struct announcement_info * next;
};

struct announcement_support_descriptor_t{
	union{
	descriptor_t descriptor;
	struct {
		uint8_t descriptor_tag;
		uint8_t descriptor_length;
		void *next;
		struct announcement_support_indicator indicator;
		struct announcement_info *info;
		};
	};
};

struct bouquet_name_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void * next;
			char *sub_table;
		};
	};
};

struct CA_identifier_descriptor_t{
	union{
		descriptor_t descriptor;
		struct {
			uint8_t descriptor_tag;
			uint8_t descriptor_length;
			void * next;
			uint16_t *CA_system_id; /*ETSI TS 101 162 [i.1]*/
		};
	};
};

struct cell_frequency_link_descriptor_t{

};

struct cell_list_descriptor_t{

};

struct component_descriptor{ 
	uint8_t descriptor_tag;
	uint8_t descriptor_length;
	uint8_t stream_content_ext:4; 
	uint8_t stream_content:4;
	uint8_t component_type;
	uint32_t component_tag:8; 
	uint32_t ISO_639_language_code:24; 
	uint8_t text_char[0];
};

struct content_descriptor_t{

};

struct country_availability_descriptor_t{

};

struct data_broadcast_descriptor_t{

};

struct data_broadcast_id_descriptor_t{

};

struct cable_delivery_system_descriptor_t{

};

struct satellite_delivery_system_descriptor_t{

};

struct S2_satellite_delivery_system_descriptor_t{
};

struct terrestrial_delivery_system_descriptor_t{
};

struct DSNG_descriptor_t{
};

struct extended_event_descriptor_t{
};

struct extension_descriptor_t{
};

struct frequency_list_descriptor_t{
};

struct FTA_content_management_descriptor_t{ /*0x7E*/
};

struct linkage_descriptor_t{
};

struct local_time_offset_descriptor_t{
};

struct mosaic_descriptor_t{
};

struct multilingual_bouquet_name_descriptor_t{
};

struct multilingual_component_descriptor_t{
};

struct multilingual_network_name_descriptor_t{
};

struct multilingual_service_name_descriptor_t{
};

struct NVOD_reference_descriptor_t{
};

struct network_name_descriptor_t{
};

struct parental_rating_descriptor_t{
};

struct PDC_descriptor_t{
};

struct private_data_specifier_descriptor_t{
};

struct scrambling_descriptor_t{
};

struct service_descriptor_t{
	
};

struct service_availability_descriptor_t{
};

struct service_list_descriptor_t{
	
};


#ifdef __cplusplus
}
#endif

#endif
