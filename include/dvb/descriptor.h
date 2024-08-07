#ifndef _DVB_DESCRIPTOR_H_
#define _DVB_DESCRIPTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "result.h"
#include "utils.h"
#include "ts.h"

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
#define foreach_network_name_member	\
	__mplast(uint8_t, text_byte)

struct service_info {
	uint16_t service_id;
	uint8_t service_type;
} PACK;

enum service_type_e {
	SERVICE_DIGITAL_TELEVISION = 0x1,
	SERVICE_H264_AVC = 0x2,
	SERVICE_H264_AVC_FRAME_COMPATIBLE_STEREOSCOPIC_HD = 0x9,
	SERVICE_ADVANCE_CODEC_DIGITAL_RADIO_SOUND = 0xA,
	SERVICE_HEVC_DIGITAL_TELEVISION = 0x1F,
	SERVICE_HEVC_UHD_DIGITAL_TELEVISION = 0x20,
};

static inline const char * server_type_name(int service_type)
{
	if (SERVICE_H264_AVC <= service_type &&
		service_type <= SERVICE_H264_AVC_FRAME_COMPATIBLE_STEREOSCOPIC_HD) {
		return "H.264/AVC";
		// return "H.264/AVC frame compatible stereoscopic HD";
	}
	switch(service_type) {
	case SERVICE_DIGITAL_TELEVISION:
		return "digital television service";
	case SERVICE_ADVANCE_CODEC_DIGITAL_RADIO_SOUND:
		return "advanced codec digital radio sound service";
	case SERVICE_HEVC_DIGITAL_TELEVISION:
		return "HEVC digital television service";
	case SERVICE_HEVC_UHD_DIGITAL_TELEVISION:
		return "HEVC UHD digital television service";
	default:
		return "unknown type";
	}
}

static inline
void dump_service_list__(int lv, struct service_info *info)
{
	rout(lv, "service_id", "0x%x", info->service_id);
	rout(lv, "service_type", "%s", server_type_name(info->service_type));
}

#define foreach_service_list_member	\
	__mplast_custom(struct service_info, services, dump_service_list__)

#define foreach_stuffing_member	\
	__mplast(uint8_t, stuffing_byte)

#define foreach_satellite_delivery_system_member	\
	__m1(uint32_t, frequency)	\
	__m1(uint16_t, orbital_position)	\
	__m(uint8_t, west_east_flag, 1)	\
	__m(uint8_t, polarization, 2)	\
	__m(uint8_t, roll_off, 2)	\
	__m(uint8_t, modulation_system, 1)	\
	__m(uint8_t, modulation_type, 2)	\
	__m(uint32_t, symbol_rate, 28)	\
	__m(uint32_t, FEC_inner, 4)	

#define foreach_cable_delivery_system_member	\
	__m1(uint32_t, frequency)	\
	__m(uint16_t, reserved_future_use, 12)	\
	__m(uint16_t, FEC_outer, 4)	\
	__m1(uint8_t, modulation)	\
	__m(uint32_t, symbol_rate, 28)	\
	__m(uint32_t, FEC_inner, 4)

struct VBI_data_node {
	uint8_t data_service_id;
	uint8_t data_service_descriptor_length;
	uint8_t *reserved;
} PACK;

#define foreach_VBI_data_member	\
	__mploop(struct VBI_data_node, vbi_data, data_service_descriptor_length)

struct teletext_node {
	uint32_t ISO_639_language_code : 24;
	uint32_t teletext_type : 5;
	uint32_t teletext_magazine_number : 3;
	uint8_t teletext_page_number;
} PACK;

static inline
void dump_teletext_infos__(int lv, struct teletext_node *n)
{
	rout(lv, "ISO_639_language_code", "%c%c%c", (n->ISO_639_language_code >> 16)&0xFF ,
		(n->ISO_639_language_code >> 8) & 0xFF, n->ISO_639_language_code& 0xFF);
	rout(lv, "teletext_type", "%d", n->teletext_type);
	rout(lv, "teletext_magazine_number", "%d", n->teletext_type);
	rout(lv, "teletext_page_number", "%d", n->teletext_page_number);
}

#define foreach_VBI_teletext_member	\
	__mplast_custom(struct teletext_node, vbi_teletext, dump_teletext_infos__)

#define foreach_bouquet_name_member	\
	__mplast(uint8_t, sub_table)

#define foreach_service_member	\
	__m1(uint8_t, service_type)	\
	__m1(uint8_t, service_provider_name_length)	\
	__mlv(uint8_t, service_provider_name_length, provider_name)	\
	__m1(uint8_t, service_name_length)	\
	__mlv(uint8_t, service_name_length, service_name)

struct country_code_node {
	uint24_t country_code;
};

#define  foreach_country_availability_member	\
	__m(uint8_t, country_availability_flag, 1)	\
	__m(uint8_t, reserved_future_use, 7)	\
	__mplast(struct country_code_node, cuontries)

struct mobile_handover_info{
	uint8_t handover_type : 4;
	uint8_t reserved_for_furture : 3;
	uint8_t origin_type : 1;
	union{
		uint16_t network_id;
		uint16_t initial_service_id;
	};
}PACK;

struct event_linkage_info{
	uint16_t target_event_id;
	uint8_t target_listed : 1;
	uint8_t event_simulcast : 1;
	uint8_t reserved : 6;
}PACK;

struct extend_event_linkage_subinfo{
	uint16_t target_event_id;
	uint8_t target_listed : 1;
	uint8_t event_simulcast : 1;
	uint8_t link_type : 2;
	uint8_t target_id_type : 2;
	uint8_t original_network_id_flag : 1;
	uint8_t service_id_flag : 1;
	
	uint16_t user_defined_id;
	uint16_t target_transport_stream_id;
	uint16_t target_original_network_id;
	uint16_t target_service_id;
	
}PACK;

struct extend_event_linkage_info{
	uint8_t loop_length;
	int n_subinfo;
	struct extend_event_linkage_subinfo * subinfo;
};

static inline int
__parse_extend_event_linkage_info(uint8_t *buf, int len, struct extend_event_linkage_info *e)
{
	uint8_t *ptr = buf;
	e->loop_length = TS_READ8(ptr);
	ptr += 1;
	int l = 0, i = 0;
	while (l < e->loop_length) {
		struct extend_event_linkage_subinfo *einfo = (struct extend_event_linkage_subinfo *)realloc(e->subinfo, (i + 1) * sizeof(struct extend_event_linkage_subinfo));
		if (!einfo) {return ENOMEM; }
		e->subinfo = einfo;
		e->subinfo[i].target_event_id = TS_READ16(ptr);
		ptr += 2;
		l += 2;

		e->subinfo[i].target_listed = TS_READ8_BITS(buf, 1, 0);
		e->subinfo[i].event_simulcast = TS_READ8_BITS(buf, 1, 1);
		e->subinfo[i].link_type = TS_READ8_BITS(buf, 2, 2);
		e->subinfo[i].target_id_type = TS_READ8_BITS(buf, 2, 4);
		e->subinfo[i].original_network_id_flag = TS_READ8_BITS(buf, 1, 6);
		e->subinfo[i].service_id_flag = TS_READ8_BITS(buf, 1, 7);
		ptr += 1;
		l += 1;
		if (e->subinfo[i].target_id_type == 3) {
			e->subinfo[i].user_defined_id = TS_READ16(ptr);
			ptr += 2;
			l += 2;
		} else {
			if (e->subinfo[i].target_id_type == 1) {
				e->subinfo[i].target_transport_stream_id = TS_READ16(ptr);
				ptr += 2;
				l += 2;
			}
			if (e->subinfo[i].original_network_id_flag) {
				e->subinfo[i].target_original_network_id = TS_READ16(ptr);
				ptr += 2;
				l += 2;
			}
			if (e->subinfo[i].service_id_flag) {
				e->subinfo[i].target_service_id = TS_READ16(ptr);
				ptr += 2;
				l += 2;
			}
		}
		i ++;
	}
	e->n_subinfo = i;
	return e->loop_length + 1;
}

static inline void
__dump_extend_event_linkage_info(int lv, struct extend_event_linkage_info *e)
{
	rout(lv, "loop_length", "%d", e->loop_length);
	for (int i = 0; i < e->n_subinfo; i ++) {
		rout(lv, "target_event_id", "%d", e->subinfo[i].target_event_id);
		rout(lv, "target_listed", "%d", e->subinfo[i].target_listed);
		rout(lv, "event_simulcast", "%d", e->subinfo[i].event_simulcast);
		rout(lv, "link_type", "%d", e->subinfo[i].link_type);
		rout(lv, "target_id_type", "%d", e->subinfo[i].target_id_type);
		rout(lv, "original_network_id_flag", "%d", e->subinfo[i].original_network_id_flag);
		rout(lv, "service_id_flag", "%d", e->subinfo[i].service_id_flag);
		if (e->subinfo[i].target_id_type == 3) {
			rout(lv, "user_defined_id", "%d", e->subinfo[i].user_defined_id);
		} else {
			if (e->subinfo[i].target_id_type == 1) {
				rout(lv, "target_transport_stream_id", "%d", e->subinfo[i].target_transport_stream_id);
			}
			if (e->subinfo[i].original_network_id_flag) {
				rout(lv, "target_original_network_id", "%d", e->subinfo[i].target_original_network_id);
			}
			if (e->subinfo[i].service_id_flag) {
				rout(lv, "target_service_id", "%d", e->subinfo[i].target_service_id);
			}
		}
	}
}
static inline void
__free_extend_event_linkage_info(struct extend_event_linkage_info *e)
{
	free(e->subinfo);
}

/* 0x4A */
#define foreach_linkage_member	\
	__m1(uint16_t, transport_stream_id)	\
	__m1(uint16_t, original_network_id)	\
	__m1(uint16_t, service_id)	\
	__m1(uint8_t, linkage_type)	\
	__mif(struct mobile_handover_info, mobile_handover, linkage_type, 0x08)	\
	__mif(struct event_linkage_info, event_linkage, linkage_type, 0x0D)	\
	__mrangelv_custom(struct extend_event_linkage_info, extend_event_linkage, linkage_type, 0x0E, 0x1F, __parse_extend_event_linkage_info, __dump_extend_event_linkage_info, __free_extend_event_linkage_info)	\
	__mplast(uint8_t, private_data_byte)


struct NVOD_reference_node {
	uint16_t transport_stream_id;
	uint16_t original_network_id;
	uint16_t service_id;
};

#define foreach_NVOD_reference_member	\
	__mplast(struct NVOD_reference_node, references)

#define foreach_time_shifted_service_member	\
	__m1(uint8_t, reference_service_id)

#define foreach_short_event_member	\
	__m(uint32_t, ISO_639_language_code, 24)	\
	__m1(uint8_t, event_name_length)	\
	__mlv(uint8_t, event_name_length, event_name_char)	\
	__m1(uint8_t, text_length)	\
	__mlv(uint8_t, text_length, text_char)

struct event_item_node {
	uint8_t item_description_length;
	uint8_t *item_description_char;
	uint8_t item_length;
	uint8_t *item_char;
};

#define foreach_extended_event_member	\
	__m(uint32_t, descriptor_number, 4)	\
	__m(uint32_t, last_descriptor_number, 4)	\
	__m(uint32_t, ISO_639_language_code, 24)	\
	__m1(uint8_t, length_of_items)	\
	__mlv(struct event_item_node, length_of_items, items)	\
	__m1(uint8_t, text_length)	\
	__mlv(uint8_t, text_length, text_char)

#define foreach_time_shifted_event_member	\
	__m1(uint16_t, reference_service_id)	\
	__m1(uint16_t, reference_event_id)

#define foreach_component_member	\
	__m(uint8_t, stream_content_ext, 4)	\
	__m(uint8_t, stream_content, 4)	\
	__m1(uint8_t, component_type)	\
	__m(uint32_t, component_tag, 8)	\
	__m(uint32_t, ISO_639_language_code, 24)	\
	__mplast(uint8_t, text_char)

//TODO
#define foreach_mosaic_member	\
	__m(uint8_t, mosaic_entry_point, 1)	\
	__m(uint8_t, number_of_horizontal_elementary_cells, 3)	\
	__m(uint8_t, reserved_futrue_use, 1)	\
	__m(uint8_t, number_of_vertical_elementary_cells, 3)

#define foreach_stream_identifier_member	\
	__m1(uint8_t, component_tag)

/*ETSI TS 101 162 [i.1]*/
#define foreach_CA_identifier_member	\
	__mplast(uint16_t, CA_system_id)

struct content_node {
	uint8_t content_nibble_level_1 : 4;
	uint8_t content_nibble_level_2 : 4;
	uint8_t byte;
};

#define foreach_content_member	\
	__mplast(struct content_node, contents)

#define foreach_parental_rating_member	\
	__mplast(uint32_t, countrycode_and_rating)

enum teletext_type {
	teletext_reserved = 0x0,
	teletext_initial_page = 0x1,
	teletext_subtitle_page = 0x2,
	additional_information_page = 0x3,
	programme_schedule_page = 0x4,
	hearing_impaired_page = 0x5,
	/*0x06 to 0x1F reserved*/
};

// struct teletext_node {
// 	uint32_t ISO_639_language_code : 24;
// 	uint32_t teletext_type : 5;
// 	uint32_t teletext_magazine_number : 3;
// 	uint8_t teletext_page_number;
// }PACK;

#define  foreach_teletext_member	\
	__mplast_custom(struct teletext_node, teletext_infos, dump_teletext_infos__)

#define foreach_telephone_member	\
	__m(uint8_t, reserved_future_use, 2)	\
	__m(uint8_t, foreign_availability, 1)	\
	__m(uint8_t, connection_type, 5)	\
	__m(uint8_t, reserved_future_use1, 1)	\
	__m(uint8_t, country_prefix_length, 2)	\
	__m(uint8_t, international_area_code_length, 3)	\
	__m(uint8_t, operator_code_length, 2)	\
	__m(uint8_t, reserved_future_use2, 1)	\
	__m(uint8_t, national_area_code_length, 3)	\
	__m(uint8_t, core_number_length, 4)	\
	__mlv(uint8_t, country_prefix_length, country_prefix_char)	\
	__mlv(uint8_t, international_area_code_length, international_area_code_char)	\
	__mlv(uint8_t, operator_code_length, operator_code_char)	\
	__mlv(uint8_t, national_area_code_length, national_area_code_char)	\
	__mlv(uint8_t, core_number_length, core_number_char)


struct local_time_node {
	uint32_t country_code : 24;
	uint32_t country_region_id : 6;
	uint32_t reserved : 1;
	uint32_t local_time_offset_polarity : 1;
	uint16_t local_time_offset;
	UTC_time_t time_of_change;
	uint16_t next_time_offset;
} PACK;

static inline
void dump_local_time_offset__(int lv, struct local_time_node *n)
{
	rout(lv, "country", "%c%c%c", (n->country_code>>16)& 0xFF, (n->country_code>>8)& 0xFF, n->country_code & 0xFF);
	rout(lv, "country_region_id", "%d", n->country_region_id);
	rout(lv, "local_time_offset_polarity", "%d", (n->local_time_offset_polarity));
	rout(lv, "local_time_offset", "%d", (n->local_time_offset));
	rout(lv, "time_of_change", "%s", convert_UTC(&n->time_of_change));
	rout(lv, "next_time_offset", "%d", (n->next_time_offset));
}

#define foreach_local_time_offset_member	\
	__mplast_custom(struct local_time_node, time_list, dump_local_time_offset__)


struct subtitling_node {
	uint32_t ISO_639_language_code : 24;
	uint32_t subtitling_type : 8;
	uint16_t composition_page_id;
	uint16_t ancillary_page_id;
} PACK;

static inline
void dump_subtitling_descriptor__(int lv, struct subtitling_node *n)
{
	rout(lv, "ISO_639_language_code", "%c%c%c", (n->ISO_639_language_code >> 16)&0xFF ,
		(n->ISO_639_language_code >> 8) & 0xFF, n->ISO_639_language_code& 0xFF);
	
	//see en_300743
	rout(lv, "subtitling_type", "0x%x", n->subtitling_type);
	rout(lv, "composotion_page_id", "%d", n->composition_page_id);
	rout(lv, "ancillary_page_id", "%d", n->ancillary_page_id);
}

#define foreach_subtitling_member	\
	__mplast_custom(struct subtitling_node, subtitle_list, dump_subtitling_descriptor__)

#define foreach_terrestrial_delivery_system_member	\
	__m1(uint32_t, centre_frequency)	\
	__m(uint8_t, bandwidth, 3)	\
	__m(uint8_t, priority, 1)	\
	__m(uint8_t, time_slicing_indicator, 1)	\
	__m(uint8_t, MPE_FEC_indicator, 1)	\
	__m(uint8_t, reserved_future_use, 2)	\
	__m(uint8_t, constellation, 2)	\
	__m(uint8_t, hierarchy_information, 3)	\
	__m(uint8_t, code_rate_HP_stream, 3)	\
	__m(uint8_t, code_rate_LP_stream, 3)	\
	__m(uint8_t, guard_interval, 2)	\
	__m(uint8_t, transmission_mode, 2)	\
	__m(uint8_t, other_frequency_flag, 1)	\
	__m1(uint32_t, reserved_future_use1)


struct multilingual_node {
	uint32_t ISO_639_language_code:24;
	uint32_t name_length:8;
	uint8_t* text_char;
}PACK;

static inline int
__parse_multilingual_node(uint8_t *buf, int len, struct multilingual_node *node)
{
	node->ISO_639_language_code = TS_READ32_BITS(buf, 24, 0);
	node->name_length = TS_READ32_BITS(buf, 8, 24);
	node->text_char = (uint8_t *)calloc(1, node->name_length + 1);
	if (!node->text_char) {
		return ENOMEM;
	}
	memcpy(node->text_char, buf + 4, node->name_length);
	return node->name_length + 4;
}

static inline void
__dump_multilingual_node(int lv, struct multilingual_node *node)
{
	rout(lv, "ISO_639_language_code", "%c%c%c", (node->ISO_639_language_code>>16) &0xFF, 
		 (node->ISO_639_language_code>>8) &0xFF,  (node->ISO_639_language_code) &0xFF);
	rout(lv, "text_char", "%s", node->text_char);
}

static inline void
__free_multilingual_node(struct multilingual_node *node)
{
	free(node->text_char);
}

#define foreach_multilingual_network_name_member	\
	__mploop_custom(struct multilingual_node, network_name, name_length, __parse_multilingual_node, __dump_multilingual_node, __free_multilingual_node)

#define foreach_multilingual_bouquet_name_member	\
	__mploop_custom(struct multilingual_node, bouquet_name, name_length, __parse_multilingual_node, __dump_multilingual_node, __free_multilingual_node)

struct multilingual_service_node {
	uint32_t ISO_639_language_code:24;
	uint32_t name_length:8;
	uint8_t *text_char;
	uint8_t service_name_length;
	uint8_t *service_char;
};


static inline int
__parse_multilingual_service_node(uint8_t *buf, int len, struct multilingual_service_node *node)
{
	node->ISO_639_language_code = TS_READ32_BITS(buf, 24, 0);
	node->name_length = TS_READ32_BITS(buf, 8, 24);
	node->text_char = (uint8_t *)calloc(1, node->name_length + 1);
	if (!node->text_char) {
		return ENOMEM;
	}
	memcpy(node->text_char, buf + 4, node->name_length);
	node->service_name_length = TS_READ8(buf + 4 + node->name_length);
	node->service_char = (uint8_t *)calloc(1, node->service_name_length + 1);
	if (!node->service_char) {
		return ENOMEM;
	}
	memcpy(node->service_char, buf + 5 + node->name_length, node->service_name_length);
	return node->name_length + 5 + node->service_name_length;
}

static inline void
__dump_multilingual_service_node(int lv, struct multilingual_service_node *node)
{
	rout(lv, "ISO_639_language_code", "%c%c%c", (node->ISO_639_language_code>>16) &0xFF, 
		 (node->ISO_639_language_code>>8) &0xFF,  (node->ISO_639_language_code) &0xFF);
	rout(lv, "name_length", "%d", node->name_length);
	rout(lv, "text_char", "%s", node->text_char);
	rout(lv, "service_name_length", "%d", node->service_name_length);
	rout(lv, "service_char", "%s", node->service_char);
}

static inline void
__free_multilingual_service_node(struct multilingual_service_node *node)
{
	free(node->text_char);
	free(node->service_char);
}

#define foreach_multilingual_service_name_member	\
	__mploop_custom(struct multilingual_service_node, service_name, name_length, __parse_multilingual_service_node, __dump_multilingual_service_node, __free_multilingual_service_node)


#define foreach_multilingual_component_member	\
	__m1(uint8_t, component_tag)	\
	__mploop_custom(struct multilingual_node, component, name_length, __parse_multilingual_node, __dump_multilingual_node, __free_multilingual_node)


#define foreach_private_data_specifier_member	\
	__m1(uint32_t, private_data_specifier)

#define foreach_service_move_member	\
	__m1(uint16_t, new_original_network_id)	\
	__m1(uint16_t, new_transport_stream_id)	\
	__m1(uint16_t, new_service_id)

#define foreach_short_smoothing_buffer_member	\
	__m(uint8_t, sb_size, 2)	\
	__m(uint8_t, sb_leak_rate, 6)	\
	__mplast(uint8_t, DVB_reserved)

#define foreach_frequency_list_member	\
	__m(uint8_t, reserved_future_use, 6)	\
	__m(uint8_t, coding_type, 2)	\
	__mplast(uint32_t, centre_frequency)

#define foreach_partial_transport_stream_member	\
	__m(uint64_t, DVB_reserved_future_use, 2)	\
	__m(uint64_t, peak_rate, 22)	\
	__m(uint64_t, DVB_reserved_future_use1, 2)	\
	__m(uint64_t, minimum_overall_smoothing_rate, 22)	\
	__m(uint64_t, DVB_reserved_future_use2, 2)	\
	__m(uint64_t, maximum_overall_smoothing_buffer, 14)


#define foreach_data_broadcast_member	\
	__m1(uint16_t, data_broadcast_id)	\
	__m1(uint8_t, component_tag)	\
	__m1(uint8_t, selector_length)	\
	__mlv(uint8_t, selector_length, selector_byte)	\
	__m(uint32_t, ISO_639_language_code, 24)	\
	__m(uint32_t, text_length, 8)	\
	__mlv(uint8_t, text_length, text_char)


#define foreach_scrambling_member	\
	__m1(uint8_t, scrambling_mode)

#define foreach_data_broadcast_id_member	\
	__m1(uint16_t, data_broadcast_id)	\
	__mplast(uint8_t, id_selector_byte)


#define foreach_transport_stream_member	\
	__mplast(uint8_t, byte)

#define foreach_DSNG_member	\
	__mplast(uint8_t, byte)

/* see definition in EN 300 468, modified to algin*/
#define foreach_PDC_member	\
	__m(uint8_t, reserved_for_futrue, 4)	\
	__m(uint8_t, programme_identification_label_h, 4)	\
	__m1(uint16_t, programme_identification_label) // 20bit

#define foreach_AC3_member	\
	__m(uint8_t, component_type_flag, 1)	\
	__m(uint8_t, bsid_flag, 1)	\
	__m(uint8_t, mainid_flag, 1)	\
	__m(uint8_t, asvc_flag, 1)	\
	__m(uint8_t, reserved_flags, 4)	\
	__mif(uint8_t, component_type, component_type_flag, 1)	\
	__mif(uint8_t, bsid, bsid_flag, 1)	\
	__mif(uint8_t, mainid, mainid_flag, 1)	\
	__mif(uint8_t, asvc, asvc_flag, 1)	\
	__mplast(uint8_t, additional_info_byte)

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

#define foreach_ancillary_data_member	\
	__m1(uint8_t, ancillary_data_identifier)

struct subcell_list_info {
	uint64_t cell_id_extension : 8;
	uint64_t subcell_latitude : 16;
	uint64_t subcell_longitude : 16;
	uint64_t subcell_extent_of_latitude : 12;
	uint64_t subcell_extent_of_longitude : 12;
};

struct cell_list_node {
	uint16_t cell_id;
	uint16_t cell_latitude;
	uint16_t cell_longitude;
	uint32_t cell_extent_of_latitude : 12;
	uint32_t cell_extent_of_longitude : 12;
	uint32_t subcell_info_loop_length : 8;
	struct subcell_list_info *subcell_list;
};

static inline
void dump_cell_list_descriptor__(int lv, struct cell_list_node *info)
{
	struct subcell_list_info *sub;
	rout(lv, "cell_id", "%d", info->cell_id);
	rout(lv, "cell_latitude", "0x%x", info->cell_latitude);
	rout(lv, "cell_longitude", "0x%x", info->cell_longitude);
	rout(lv, "cell_extent_of_latitude", "0x%x", info->cell_extent_of_latitude);
	rout(lv, "cell_extent_of_longitude", "0x%x", info->cell_extent_of_longitude);
	rout(lv, "subcell_info_loop_length", "%d", info->subcell_info_loop_length);
	int n = info->subcell_info_loop_length / sizeof(struct subcell_list_info);
	for (int i = 0; i < n; i ++) {
		sub  = info->subcell_list + i;
		rout(lv, "cell_id_extension", "%d", sub->cell_id_extension);
		rout(lv, "subcell_latitude", "%d", sub->subcell_latitude);
		rout(lv, "subcell_longitude", "%d", sub->subcell_longitude);
		rout(lv, "subcell_extent_of_latitude", "%d", sub->subcell_extent_of_latitude);
		rout(lv, "subcell_extent_of_longitude", "%d", sub->subcell_extent_of_longitude);
	}
}

static inline int
parse_cell_list_descriptor__(uint8_t *buf, int len, struct cell_list_node *n)
{
	uint8_t *ptr = buf;
	n->cell_id = TS_READ16(ptr);
	ptr += 2;
	n->cell_latitude = TS_READ16(ptr);
	ptr += 2;
	n->cell_longitude = TS_READ16(ptr);
	ptr += 2;
	n->cell_extent_of_latitude = TS_READ32_BITS(ptr, 12, 0);
	n->cell_extent_of_longitude = TS_READ32_BITS(ptr, 12, 12);
	n->subcell_info_loop_length = TS_READ32_BITS(ptr, 8, 24);
	ptr += 4;
	int num = n->subcell_info_loop_length / sizeof(struct subcell_list_info);
	n->subcell_list = (struct subcell_list_info *)calloc(num, sizeof(struct subcell_list_info));
	if (!n->subcell_list) {
		return ENOMEM;
	}
	for (int i = 0; i < num; i ++) {
		n->subcell_list[i].cell_id_extension = TS_READ64_BITS(ptr, 8, 0);
		n->subcell_list[i].subcell_latitude = TS_READ64_BITS(ptr, 16, 8);
		n->subcell_list[i].subcell_longitude = TS_READ64_BITS(ptr, 16, 24);
		n->subcell_list[i].subcell_extent_of_latitude = TS_READ64_BITS(ptr, 12, 40);
		n->subcell_list[i].subcell_extent_of_longitude = TS_READ64_BITS(ptr, 12, 52);
		ptr += 8;
	}
	return (int) (ptr - buf);
}


static inline void
free_cell_list_descriptor__(struct cell_list_node *info)
{
	free(info->subcell_list);
}

//TODO
#define foreach_cell_list_member	\
	__mplast_custom_sub(struct cell_list_node, cell_list, parse_cell_list_descriptor__, dump_cell_list_descriptor__, free_cell_list_descriptor__)


struct subcell_info {
	uint8_t cell_id_extension;
	uint32_t transposer_frequency;
} PACK;

struct cell_frequency_node {
	uint16_t cell_id;
	uint32_t frequency;
	uint8_t subcell_info_loop_length;
	struct subcell_info *subcell_info_list;
};


static inline
void dump_cell_frequency_link_descriptor__(int lv, struct cell_frequency_node *n)
{
	rout(lv, "cell_id", "%d", n->cell_id);
	rout(lv, "frequency", "%d", n->frequency);
	rout(lv, "subcell_info_loop_length", "%d", n->subcell_info_loop_length);
	int num = n->subcell_info_loop_length / sizeof(struct subcell_info);
	for (int i = 0; i < num; i ++) {
		rout(lv, "cell_id_extension", "%d", n->subcell_info_list[i].cell_id_extension);
		rout(lv, "transposer_frequency", "%u", n->subcell_info_list[i].transposer_frequency);
	}
}

static inline int
parse_cell_frequency_link_descriptor__(uint8_t *buf, int len, struct cell_frequency_node *n)
{
	uint8_t *ptr = buf;
	n->cell_id = TS_READ16(ptr);
	ptr += 2;
	n->frequency = TS_READ32(ptr);
	ptr += 4;
	n->subcell_info_loop_length = TS_READ8(ptr);
	ptr += 1;
	int num = n->subcell_info_loop_length / sizeof(struct subcell_info);
	n->subcell_info_list = (struct subcell_info *)calloc(num, sizeof(struct subcell_info));
	if (!n->subcell_info_list) {
		return ENOMEM;
	}
	for (int i = 0; i < num; i ++) {
		n->subcell_info_list[i].cell_id_extension = TS_READ8(ptr);
		ptr += 1;
		n->subcell_info_list[i].transposer_frequency = TS_READ32(ptr);
		ptr += 4;
	}
	return (int)(ptr - buf);
}
static inline void
free_cell_frequency_link_descriptor__(struct cell_frequency_node *info)
{
	free(info->subcell_info_list);
}

#define foreach_cell_frequency_link_member \
	__mplast_custom_sub(struct cell_frequency_node, cell_frequency_list, parse_cell_frequency_link_descriptor__, dump_cell_frequency_link_descriptor__, free_cell_frequency_link_descriptor__)

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
}PACK;

struct announcement_node {
	uint8_t announcement_type : 4;
	uint8_t reserved_future_use : 1;
	uint8_t reference_type : 3;
	struct reference ref;
	// struct list_node n;
	// struct announcement_info * next;
}PACK;

#define foreach_announcement_support_member	\
	__m1(uint16_t, announcement_support_indicator)	\
	__mplast(struct announcement_node, announcement_support)

struct application_signalling {
	uint16_t reserved:1;
	uint16_t application_type:15;
	uint8_t reserved1:3;
	uint8_t AIT_version_number:5;
}PACK;

/*see definition in ETSI TS 102 809*/
#define foreach_application_signalling_member	\
	__mplast(struct application_signalling, signalling)

/*adaptation_field_data_descriptor*/
struct adaptation_field_data_identifier {
	uint8_t announcement_switching_data : 1;
	uint8_t AU_information_data : 1;
	uint8_t PVR_assist_information_data : 1;
	uint8_t tsap_timeline : 1;
	uint8_t reserved : 4;
};

#define foreach_adaptation_field_data_member	\
	__m1(uint8_t, adaptation_field_data_identifier)

/*see definition in ETSI TS 102 812*/
#define foreach_service_identifier_member \
	__mplast(uint8_t, textual_service_identifier_bytes)

#define foreach_service_availability_member	\
	__m(uint8_t, availability_flag, 1)	\
	__m(uint8_t, reserved, 7)	\
	__mplast(uint16_t, cell_id)


/* see ETSI TS 102 323 for TV-Anytime information below */
#define foreach_default_authority_member	\
	__mplast(uint8_t, default_authority_byte)

//TODO
/*see ETSI TS 102 323*/
#define foreach_related_content_member	\
	__mplast(uint8_t, related_content_bytes)


struct TVA_id {
	uint16_t TVA_id;
	uint8_t reserved : 5;
	uint8_t running_status : 3;
}PACK;

/*see ETSI TS 102 323*/
#define foreach_TVA_id_member	\
	__mplast(struct TVA_id, TVA_ids)

struct crid {
	uint8_t type : 6;
	uint8_t location : 2;
	union {
		struct {
			uint8_t length;
#ifdef _MSC_VER
			uint8_t byte[1]; 
#else
			uint8_t byte[0]; 
#endif
		};
		uint16_t ref;
	};
};

//TODO
/*see ETSI TS 102 323*/
#define foreach_content_identifier_member	\
	__mplast(uint8_t, content_identifier_byte)

/*see ETSI EN 301 192 */
#define foreach_time_slice_fec_identifier_member	\
	__m(uint8_t, time_slicing, 1)	\
	__m(uint8_t, mpe_fec, 2)	\
	__m(uint8_t, reserved, 2)	\
	__m(uint8_t, frame_size, 3)	\
	__m1(uint8_t, max_burst_duration) \
	__m(uint8_t, max_average_rate, 4)	\
	__m(uint8_t, time_slice_fec_id, 4)	\
	__mplast(uint8_t, id_selector_byte)

/*see ETSI EN 301 192 */
#define foreach_ECM_repetition_rate_member	\
	__m1(uint16_t, CA_system_ID)	\
	__m1(uint16_t, ECM_repetition_rate)	\
	__mplast(uint8_t, private_data_byte)

#define foreach_S2_satellite_delivery_system_member	\
	__m(uint8_t, scrambling_sequence_selector, 1)	\
	__m(uint8_t, multiple_input_stream_flag, 1)	\
	__m(uint8_t, backwards_compatibility_indicator, 1)	\
	__m(uint8_t, reserved_future_use, 5)	\
	__mif(uint24_t, scrambling_sequence_index, scrambling_sequence_selector, 1)	\
	__mif(uint8_t, input_stream_identifier, multiple_input_stream_flag, 1)	

#define foreach_enhanced_AC3_member	\
	__m(uint8_t, component_type_flag, 1)	\
	__m(uint8_t, bsid_flag, 1)	\
	__m(uint8_t, mainid_flag, 1)	\
	__m(uint8_t, asvc_flag, 1)	\
	__m(uint8_t, mixinfoexists, 1)	\
	__m(uint8_t, substream1_flag, 1)	\
	__m(uint8_t, substream2_flag, 1)	\
	__m(uint8_t, substream3_flag, 1)	\
	__mif(uint8_t, component_type, component_type_flag, 1)	\
	__mif(uint8_t, bsid, bsid_flag, 1)	\
	__mif(uint8_t, mainid, mainid_flag, 1)	\
	__mif(uint8_t, asvc, asvc_flag, 1)	\
	__mif(uint8_t, substream1, substream1_flag, 1)	\
	__mif(uint8_t, substream2, substream2_flag, 1)	\
	__mif(uint8_t, substream3, substream3_flag, 1)	\
	__mplast(uint8_t, additional_info_byte)


#define foreach_DTS_member	\
	__m(uint32_t, sample_rate_code, 4)	\
	__m(uint32_t, bit_rate_code, 6)	\
	__m(uint32_t, nblks, 7)		\
	__m(uint32_t, fsize, 14)	\
	__m(uint32_t, surround_mode_h, 1)	\
	__m(uint8_t, surround_mode, 5)	\
	__m(uint8_t, lfe_flag, 1)	\
	__m(uint8_t, extended_surround_flag, 2)	\
	__mplast(uint8_t, additional_info_byte)


#define foreach_AAC_member	\
	__m1(uint8_t, profile_and_level)	\
	__m(uint8_t, AAC_type_flag, 1)	\
	__m(uint8_t, SAOC_DE_flag, 1)	\
	__m(uint8_t, reserved_future_use, 6)	\
	__m1(uint8_t, AAC_type)	\
	__mplast(uint8_t, additional_info_byte)

/*see definition in ETSI TS 102 727 */
#define foreach_XAIT_location_member \
	__m1(uint16_t, xait_original_network_id)	\
	__m1(uint16_t, xait_service_id)	\
	__m(uint8_t, xait_version_number, 5)	\
	__m(uint8_t, xait_update_policy, 3)

#define foreach_FTA_content_management_member	\
	__m(uint8_t, user_defined, 1)	\
	__m(uint8_t, reserved_future_use, 3)	\
	__m(uint8_t, do_not_scramble, 1)	\
	__m(uint8_t, control_remote_access_over_internet, 2)	\
	__m(uint8_t, do_not_apply_revocation, 1)

#define foreach_extension_member	\
	__m1(uint8_t, descriptor_tag_extension)	\
	__mplast(uint8_t, selector_byte)

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