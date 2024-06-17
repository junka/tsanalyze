#ifndef _ISDB_DESCRIPTOR_H_
#define _ISDB_DESCRIPTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

//see ARIB STD - B10 Version 4.6-E2

#define foreach_enum_isdb_descriptor    \
    _(AVC_video, 0x28)  \
    _(AVC_timing_and_HRD, 0x2A) \
    _(hierarchical_transmission, 0xC0) \
    _(digital_copy_control, 0xC1) \
    _(network_identification, 0xC2) \
    _(Partial_Transport_Stream_time, 0xC3)  \
    _(audio_component, 0xC4)    \
    _(hyperlink, 0xC5) \
    _(target_region, 0xC6)  \
    _(data_content, 0xC7)   \
    _(video_decode_control, 0xC8) \
    _(download_content, 0xC9)   \
    _(CA_emm_ts, 0xCA)  \
    _(CA_contract_info, 0xCB)    \
    _(CA_service, 0xCC) \
    _(ts_information, 0xCD) \
    _(extended_broadcaster, 0xCE)   \
    _(logo_transmission, 0xCF)  \
    _(basic_local_event, 0xD0)  \
    _(reference, 0xD1)  \
    _(node_relation, 0xD2)  \
    _(short_node_information, 0xD3) \
    _(STC_reference, 0xD4)  \
    _(series, 0xD5) \
    _(event_group, 0xD6)    \
    _(SI_parameter, 0xD7)   \
    _(broadcaster_name, 0xD8)   \
    _(component_group, 0xD9)    \
    _(SI_prime_TS, 0xDA)    \
    _(board_information, 0xDB)  \
    _(LDT_linkage, 0xDC)    \
    _(connected_transmission, 0xDD) \
    _(content_availability, 0xDE)   \
    _(composite, 0xDF)  \
    _(service_group, 0xE0)  \
    _(carousel_compatible_composite, 0xF7)  \
    _(conditional_playback, 0xF8)   \
    /* _(cable_TS_division_system, 0xF9)  */ \
    _(isdb_terrestrial_delivery_system, 0xFA)   \
    _(partial_reception, 0xFB)  \
    _(emergency_information, 0xFC)  \
    _(data_component, 0xFD) \
    _(system_management, 0xFE)



/*0x28*/
#define foreach_AVC_video_member    \
    __m1(uint8_t, AVC_profile ) \
    __m(uint8_t, set0_constraint_flag, 1)   \
    __m(uint8_t, set1_constraint_flag, 1)   \
    __m(uint8_t, set2_constraint_flag, 1)   \
    __m(uint8_t, AVC_compatible_flag, 5)   \
    __m1(uint8_t, AVC_level)    \
    __m(uint8_t, AVC_still_picture, 1)  \
    __m(uint8_t, AVC_24_hours_video_flag, 1)    \
    __m(uint8_t, reserved, 6)

/*0x2A*/
#define foreach_AVC_timing_and_HRD_member   \
    __m(uint8_t, HRD_management_valid_flag, 1)  \
    __m(uint8_t, reserved, 6)  \
    __m(uint8_t, picture_and_timing_info_present, 1)  \
    __mif(uint8_t, _90kHz_flag, picture_and_timing_info_present, 1) \
    __mif(uint32_t, N, _90kHz_flag, 0)  \
    __mif(uint32_t, K, _90kHz_flag, 0)  \
    __mif(uint32_t, time_calculation_unit, picture_and_timing_info_present, 1)  \
    __m(uint8_t, fixed_flame_late_flag, 1)  \
    __m(uint8_t, time_POC_flag, 1)  \
    __m(uint8_t, video_display_conversion_flag, 1)  \
    __m(uint8_t, reserved1, 5)

/*0xC0*/
#define foreach_hierarchical_transmission_member    \
    __m(uint8_t, reserved, 7)   \
    __m(uint8_t, hierarchical_level, 1) \
    __m(uint16_t, reserved1, 3) \
    __m(uint16_t, reference_PID , 13)

struct component_control {
    uint8_t component_tag;
    uint8_t digital_copy_control_info:2;
    uint8_t maximum_bitrate_flag:1;
    uint8_t reserved : 5;
    uint8_t maximum_bitrate;
};

//TODO
/*0xC1*/
#define foreach_digital_copy_control_member     \
    __m(uint8_t, digital_copy_control_info, 2)  \
    __m(uint8_t, maximum_bitrate_flag, 1)   \
    __m(uint8_t, component_control_flag, 1)   \
    __m(uint8_t, user_defined, 4)   \
    __mif(uint8_t, maximum_bitrate, maximum_bitrate_flag, 1)    \
    __mif(uint8_t, component_control_length, component_control_flag, 1)    \
    __mplast(struct component_control, coponents)


/*0xC2*/
#define foreach_network_identification_member \
    __m1(uint24_t, country_code) \
    __m1(uint16_t, media_type) \
    __m1(uint16_t, network_id) \
    __mplast(uint8_t, private_data)

#define foreach_Partial_Transport_Stream_time_member \
    __mplast(uint8_t, tbd)
/*0xC4*/
#define foreach_audio_component_member  \
    __m(uint8_t, reserved, 4)   \
    __m(uint8_t, stream_content, 4) \
    __m1(uint8_t, component_type)   \
    __m1(uint8_t, component_tag)    \
    __m1(uint8_t, stream_type)  \
    __m1(uint8_t, simulcast_group_tag)  \
    __m(uint32_t, ES_multi_language_flag, 1)    \
    __m(uint32_t, main_component_flag, 1)    \
    __m(uint32_t, quality_indicator, 2)    \
    __m(uint32_t, sampling_rate, 3)    \
    __m(uint32_t, reserved_for_future, 1)    \
    __m(uint32_t, audio_lang, 1)    \
    __mif(uint24_t, audio_lang2, ES_multi_language_flag, 1) \
    __mplast(uint8_t, text_byte)

/*0xC5*/
#define foreach_hyperlink_member    \
    __m1(uint16_t, hyper_linkage_type)  \
    __m1(uint8_t, link_destination_type)    \
    __m1(uint8_t, selector_length)  \
    __mlv(uint8_t, selector_length, selector_byte)  \
    __mplast(uint8_t, private_area)

/*0xC6*/
#define foreach_target_region_member    \
    __m1(uint8_t, region_spec_type) \
    __mplast(uint8_t, target_region_spec)

/*0xC7*/
#define foreach_data_content_member    \
    __m1(uint16_t, data_component_id)   \
    __m1(uint8_t, entry_component)  \
    __m1(uint8_t, selector_length)  \
    __mlv(uint8_t, selector_length, selector_byte)  \
    __m1(uint8_t, number_of_component_ref)  \
    __mlv(uint8_t, number_of_component_ref, component_ref)  \
    __m(uint32_t, lang_code, 24)   \
    __m(uint32_t, text_length, 8)  \
    __mlv(uint8_t, text_length, text_char)

/*0xC8*/
#define foreach_video_decode_control_member \
    __m(uint8_t, still_picture_flag, 1) \
    __m(uint8_t, sequence_end_code_flag, 1) \
    __m(uint8_t, video_encode_format, 4) \
    __m(uint8_t, reserved, 2)


struct sub_descriptor{
    uint8_t sub_descriptor_tag;
    uint8_t sub_descriptor_length;
    uint8_t* sub_descriptor_byte;
}PACK;

struct compatobilitylist_descriptor {
    uint8_t type;
    uint8_t length;
    uint32_t specifierType:8;
    uint32_t specifierData:24;
    uint16_t model;
    uint16_t version;
    uint8_t count;
    struct sub_descriptor * sub;
};

struct compatibility_descriptor {
    uint16_t length;
    uint16_t count;
    struct compatobilitylist_descriptor *sub;
};

static inline int
__parse_compatibility_descriptor(uint8_t *buf, int len, struct compatibility_descriptor *c)
{
    uint8_t *ptr = buf;
    c->length = TS_READ16(ptr);
    ptr += 2;
    c->count = TS_READ16(ptr);
    ptr += 2;
    c->sub = (struct compatobilitylist_descriptor *)calloc(c->count, sizeof(struct compatobilitylist_descriptor));
    for (int i = 0; i < c->count; i++) {
        c->sub[i].type = TS_READ8(ptr);
        ptr += 1;
        c->sub[i].length = TS_READ8(ptr);
        ptr += 1;
        c->sub[i].specifierType = TS_READ32_BITS(ptr, 8, 0);
        c->sub[i].specifierData = TS_READ32_BITS(ptr, 24, 8);
        ptr += 4;
        c->sub[i].model = TS_READ16(ptr);
        ptr += 2;
        c->sub[i].version = TS_READ16(ptr);
        ptr += 2;
        c->sub[i].count = TS_READ8(ptr);
        ptr += 1;
        c->sub[i].sub = (struct sub_descriptor *)calloc(c->sub[i].count, sizeof(struct sub_descriptor));
        for (int j = 0; j < c->sub[i].count; j ++) {
            c->sub[i].sub[j].sub_descriptor_tag = TS_READ8(ptr);
            ptr += 1;
            c->sub[i].sub[j].sub_descriptor_length = TS_READ8(ptr);
            ptr += 1;
            c->sub[i].sub[j].sub_descriptor_byte = (uint8_t *)calloc(1, c->sub[i].sub[j].sub_descriptor_length);
            for (int m = 0; m < c->sub[i].sub[j].sub_descriptor_length; m ++) {
                c->sub[i].sub[j].sub_descriptor_byte[m] = TS_READ8(ptr);
                ptr ++;
            }
        }
    }
    return ptr - buf;
}

static inline void
__dump_compatibility_descriptor(int lv, struct compatibility_descriptor *c)
{
    rout(lv, "length", "%d", c->length);
    rout(lv, "count", "%d", c->count);
    for (int i = 0; i < c->count; i++) {
        rout(lv, "type", "%d", c->sub[i].type);
        rout(lv, "length", "%d", c->sub[i].length);
        rout(lv, "specifierType", "%d", c->sub[i].specifierType);
        rout(lv, "specifierData", "%d", c->sub[i].specifierData);
        rout(lv, "model", "%d", c->sub[i].model);
        rout(lv, "version", "%d", c->sub[i].version);
        rout(lv, "count", "%d", c->sub[i].count);
        for (int j = 0; j < c->sub[i].count; j ++) {
            rout(lv, "sub_descriptor_tag", "%d", c->sub[i].sub[j].sub_descriptor_tag);
            rout(lv, "sub_descriptor_length", "%d", c->sub[i].sub[j].sub_descriptor_length);
            rout(lv, "sub_descriptor_byte", "%s", c->sub[i].sub[j].sub_descriptor_byte);
        }
    }
}

static inline void
__free_compatibility_descriptor(struct compatibility_descriptor *c)
{
    for (int i = 0; i < c->count; i ++) {
        for (int j = 0; j < c->sub[i].count; j ++) {
            free(c->sub[i].sub[j].sub_descriptor_byte);
        }
        free(c->sub[i].sub);
    }
    free(c->sub);
}

struct module_info{
    uint16_t module_id;
    uint32_t module_size;
    uint8_t module_info_length;
    uint8_t *module_info_byte;
};
struct module_info_list {
    uint16_t num_of_modules;
    struct module_info *info;
};

static inline int
__parse_module_info_list(uint8_t *buf, int len, struct module_info_list *m)
{
    uint8_t * ptr = buf;
    m->num_of_modules = TS_READ16(ptr);
    ptr += 2;
    m->info = (struct module_info *)calloc(m->num_of_modules, sizeof(struct module_info));
    for (int i = 0; i < m->num_of_modules; i ++) {
        m->info[i].module_id = TS_READ16(ptr);
        ptr += 2;
        m->info[i].module_size = TS_READ32(ptr);
        ptr += 4;
        m->info[i].module_info_length = TS_READ8(ptr);
        ptr += 1;
        m->info[i].module_info_byte = (uint8_t *)calloc(1, m->info[i].module_info_length);
        for (int j = 0; j < m->info[i].module_info_length; j ++) {
            m->info[i].module_info_byte[j] = TS_READ8(ptr);
            ptr += 1;
        }
    }
    return ptr - buf;
}

static inline void
__dump_module_info_list(int lv, struct module_info_list *m)
{
    rout(lv, "num_of_modules", "%d", m->num_of_modules);
    for (int i = 0; i < m->num_of_modules; i ++) {
        rout(lv, "module_id", "%d", m->info[i].module_id);
        rout(lv, "module_size", "%d", m->info[i].module_size);
        rout(lv, "module_info_length", "%d", m->info[i].module_info_length);
        rout(lv, "module_info_byte", "%s", m->info[i].module_info_byte);
    }
}

static inline void
__free_module_info_list(struct module_info_list *m)
{
    for (int i = 0; i < m->num_of_modules; i ++) {
        free(m->info[i].module_info_byte);
    }
    free(m->info);
}

struct text_info {
    uint32_t ISO_639_language_code:24;
    uint32_t text_length:8;
    uint8_t *text_char;
};

static inline int
__parse_text_info(uint8_t *buf, int len, struct text_info *t)
{
    uint8_t *ptr = buf;
    t->ISO_639_language_code = TS_READ32_BITS(ptr, 24, 0);
    t->text_length = TS_READ32_BITS(ptr, 8, 24);
    ptr += 4;

    t->text_char = (uint8_t *)calloc(1, t->text_length);
    for (int i = 0; i < t->text_length; i++) {
        t->text_char[i] = TS_READ8(ptr);
        ptr += 1;
    }

    return ptr - buf;
}

static inline void
__dump_text_info(int lv, struct text_info *t)
{
	rout(lv, "ISO_639_language_code", "%c%c%c", (t->ISO_639_language_code >> 16)&0xFF ,
		(t->ISO_639_language_code >> 8) & 0xFF, t->ISO_639_language_code& 0xFF);
    rout(lv, "text_length", "%d", t->text_length);
    rout(lv, "text_char", "%s", t->text_char);
}

static inline void
__free_text_info(struct text_info *t)
{
    free(t->text_char);
}

/*0xC9*/
/* ARIB STD-B1, B21  Table 12-2
* https://www.arib.or.jp/english/html/overview/doc/6-STD-B21v5_12-E1.pdf */
#define foreach_download_content_member  \
    __m(uint8_t, reboot, 1) \
    __m(uint8_t, add_on, 1) \
    __m(uint8_t, compatibility_flag, 1) \
    __m(uint8_t, module_info_flag, 1) \
    __m(uint8_t, text_info_flag, 1) \
    __m(uint8_t, reserved1, 3) \
    __m1(uint32_t, component_size) \
    __m1(uint32_t, download_id) \
    __m1(uint32_t, time_out_value_DII) \
    __m(uint32_t, leak_rate, 22) \
    __m(uint32_t, reserved2, 2) \
    __m(uint32_t, component_tag, 8) \
    __mif_custom(struct compatibility_descriptor, compat_desc, compatibility_flag, 1, __parse_compatibility_descriptor, __dump_compatibility_descriptor, __free_compatibility_descriptor) \
    __mif_custom(struct module_info_list, modules, module_info_flag, 1, __parse_module_info_list, __dump_module_info_list, __free_module_info_list) \
    __m1(uint8_t, private_data_length) \
    __mlv(uint8_t, private_data_length, private_data_byte) \
    __mif_custom(struct text_info, text, text_info_flag, 1, __parse_text_info, __dump_text_info, __free_text_info)




/*0xCA*/
#define foreach_CA_emm_ts_member    \
    __m1(uint16_t, CA_system_id)    \
    __m1(uint16_t, transport_stream_id) \
    __m1(uint16_t, original_network_id) \
    __m1(uint8_t, power_supply_period)

/*0xCB*/
#define foreach_CA_contract_info_member \
    __m1(uint16_t, CA_system_id)    \
    __m(uint8_t, CA_unit_id, 4) \
    __m(uint8_t, num_of_component, 4)   \
    __mlv(uint8_t, num_of_component, component_tag) \
    __m1(uint8_t, contract_verification_info_length)    \
    __mlv(uint8_t, contract_verification_info_length, contract_verification_info)   \
    __m1(uint8_t, fee_name_length)  \
    __mlv(uint8_t, fee_name_length, fee_name)


/*0xCC*/
#define foreach_CA_service_member   \
    __m1(uint16_t, CA_system_id)    \
    __m1(uint8_t, ca_broadcaster_group_id)  \
    __m1(uint8_t, message_control)  \
    __mplast(uint16_t, service_id)

struct ts_information{
    uint8_t transmission_type_info;
    uint8_t num;
    uint16_t* service_id;
};

/*0xCD*/
/* Table 6-28 in STD-B10v4_6 */
#define foreach_ts_information_member   \
    __m1(uint8_t, remote_control_key_identification) \
    __m(uint8_t, length_of_ts_name, 6)  \
    __m(uint8_t, transmission_type_count, 2)   \
    __mlv(uint8_t, length_of_ts_name, ts_name_char) \
    __mploop(struct ts_information, info, num)  \
    __mplast(uint8_t, reserved_for_future)  \



struct broadcaster_id {
    uint16_t original_network_id;
    uint8_t broadcaster_id;
} PACK;

/*0xCE*///TODO
#define foreach_extended_broadcaster_member \
    __m(uint8_t, broadcaster_type, 4)   \
    __m(uint8_t, reserved, 4)   \
    __m1(uint16_t, terrestrial_broadcaster_ID)    \
    __m(uint8_t, number_of_affiliation_ID_loop, 4)  \
    __m(uint8_t, number_of_broadcaster_ID, 4) \
    __mlv(uint8_t, number_of_affiliation_ID_loop, affiliation_ID)   \
    __mlv(struct broadcaster_id, number_of_broadcaster_ID, broadcaster_id)  \
    __mplast(uint8_t, parivate_data)

struct logo_1{
    uint16_t reserved:7;
    uint16_t logo_identifier:9;
    uint16_t reserved1:4;
    uint16_t logo_version:12;
    uint16_t download_data_identifier;
} PACK;

struct logo_2{
    uint16_t reserved:7;
    uint16_t logo_identifier:9;
};

/*0xCF*/
#define foreach_logo_transmission_member \
    __m1(uint8_t, logo_transmission_type)   \
    __mif(struct logo_1, logo_iden_1, logo_transmission_type, 1)    \
    __mif(struct logo_2, logo_iden_2, logo_transmission_type, 2)    \
    __mplast(uint8_t, logo_char)


/*0xD0*/
#define foreach_basic_local_event_member    \
    __m(uint8_t, reserved, 4)   \
    __m(uint8_t, segmentation_mode, 4)  \
    __m1(uint8_t, segmentation_info_length) \
    __mlv(uint8_t, segmentation_info_length, segmentation_info)   \
    __mplast(uint8_t, component_tag)

struct reference_info{
    uint16_t reference_node_id;
    uint8_t reference_number;
    uint8_t last_reference_number;
};

/*0xD1*/
#define foreach_reference_member    \
    __m1(uint16_t, information_provider_id) \
    __m1(uint16_t, event_relation_id)   \
    __mplast(struct reference_info, references)

/*0xD2*/
#define foreach_node_relation_member    \
    __m(uint8_t, reference_type, 4) \
    __m(uint8_t, external_reference_flag, 1)    \
    __m(uint8_t, reserved, 3)   \
    __mif(uint16_t, information_provider_id, external_reference_flag, 1)  \
    __mif(uint16_t, event_relation_id, external_reference_flag, 1)    \
    __m1(uint16_t, reference_node_id)   \
    __m1(uint8_t, reference_number)

/*0xD3*/
#define foreach_short_node_information_member   \
    __m(uint32_t, lang_code, 24)   \
    __m(uint32_t, node_name_length, 8) \
    __mlv(uint8_t, node_name_length, node_name_char)    \
    __m1(uint8_t, text_length)  \
    __mlv(uint8_t, text_length, text_char)

/*0xD4*//*TODO*/
#define foreach_STC_reference_member    \
    __m(uint8_t, reserved, 3)   \
    __m(uint8_t, external_event_flag, 1)    \
    __m(uint8_t, STC_reference_mode, 4)    \
    __mif(uint16_t, external_event_id, external_event_flag, 1)  \
    __mif(uint16_t, external_service_id, external_event_flag, 1)  \
    __mif(uint16_t, external_network_id, external_event_flag, 1)  \
    __mplast(uint8_t, references)

/*0xD5*///TODO
#define foreach_series_member   \
    __m1(uint16_t, series_id)   \
    __m(uint8_t, repeat_label, 4)   \
    __m(uint8_t, program_pattern, 3)   \
    __m(uint8_t, expire_date_valid_flag, 1)   \
    __m1(uint16_t, expire_time) \
    __m1(uint8_t, expire_name)  \
    __m1(uint16_t, last_expire_number)  \
    __mplast(uint8_t, series_name_char)

struct event_group{
    uint16_t service_identifier;
    uint16_t event_identifier;
};

struct event_group_data{
    uint16_t original_network_identifier;
    uint16_t transport_stream_identifier;
    uint16_t service_identifier;
    uint16_t event_identifier;
};

/*0xD6*/
#define foreach_event_group_member  \
    __m(uint8_t, group_type, 4) \
    __m(uint8_t, event_count, 4)   \
    __mlv(struct event_group, event_count, event_groups)    \
    __mplast(struct event_group_data, private_data)

struct table_description{
    uint8_t table_id;
    uint8_t table_description_length;
    uint8_t *table_description_byte;
}PACK;

/*0xD7*///TODO
#define foreach_SI_parameter_member    \
    __m1(uint8_t, parameter_version)    \
    __m1(uint16_t, update_time) \
    __mploop(struct table_description, parameter, table_description_length)

/*0xD8*/
#define foreach_broadcaster_name_member \
    __mplast(uint8_t, text_char)

/*0xD9*/
#define foreach_component_group_member  \
    __m(uint8_t, component_group_type, 3) \
    __m(uint8_t, total_bit_rate_flag, 1) \
    __m(uint8_t, num_of_group, 4) \
    


/*0xDA*/
#define foreach_SI_prime_TS_member  \
    __m1(uint8_t, parameter_version) \
    __m1(uint16_t, update_time) \
    __m1(uint16_t, SI_prime_ts_network_id ) \
    __m1(uint16_t, SI_prime_transport_stream_id ) \
    __mploop(struct table_description, parameter, table_description_length)

/*0xDB*/
#define foreach_board_information_member    \
    __m1(uint8_t, title_length)  \
    __mlv(uint8_t, title_length, title) \
    __m1(uint8_t, text_length)  \
    __mlv(uint8_t, text_length, text)

struct LDT_linkage {
    uint16_t description_id;
    uint8_t reserved:4;
    uint8_t description_type :4;
    uint8_t user_defined;
};

/*0xDC*/
#define foreach_LDT_linkage_member  \
    __m1(uint16_t, original_service_id) \
    __m1(uint16_t, transport_stream_id) \
    __m1(uint16_t, original_network_id) \
    __mplast(struct LDT_linkage , linkage)

/*0xDD*/
#define foreach_connected_transmission_member   \
    __m1(uint16_t, connected_transmission_group_id) \
    __m(uint8_t, segment_type, 2)   \
    __m(uint8_t, modulation_type_A, 2)   \
    __m(uint8_t, modulation_type_B, 2)   \
    __m(uint8_t, reserved, 2)   \
    __mplast(uint8_t, additional_connected_transmission_info)

/*0xDE*/
#define foreach_content_availability_member \
    __m(uint8_t, reserved, 1)   \
    __m(uint8_t, copy_restriction_mode, 1)   \
    __m(uint8_t, image_constraint_token, 1)   \
    __m(uint8_t, temporal_accumulation_control_bit, 1)   \
    __m(uint8_t, allowable_time_of_temporal_accumulation, 3)   \
    __m(uint8_t, output_protection_bit , 1)   \
    __mplast(uint8_t, reserved_for_future)

#define foreach_composite_member    \
    __m1(uint8_t, sub_descriptor_tag)   \
    __m1(uint8_t, sub_descriptor_length)    \
    __mlv(uint8_t, sub_descriptor_length, data_byte)

struct service_group_info{
    uint16_t primary_service_identifier;
    uint16_t seconary_service_identifier;
};
/*0xE0*/
#define foreach_service_group_member    \
    __m(uint8_t, service_group_type, 4) \
    __m(uint8_t, undefined, 4)  \
    __mplast(struct service_group_info, service_info)

/*0xF7*/
#define foreach_carousel_compatible_composite_member    \
    __mploop(struct sub_descriptor, subdescriptor, sub_descriptor_length)

/*0xF8*/
#define foreach_conditional_playback_member  \
    __m1(uint16_t, conditional_playback_system_identifier)  \
    __m(uint16_t, reserved, 3)  \
    __m(uint16_t, conditional_playback_PID, 13) \
    __mplast(uint8_t, private_data)

// #define foreach_cable_TS_division_system_member

/*0xFA*/
#define foreach_isdb_terrestrial_delivery_system_member  \
    __m(uint16_t, area_code, 12)    \
    __m(uint16_t, guard_interval, 2)    \
    __m(uint16_t, transmission_mode, 2) \
    __mplast(uint16_t, frequency)

/*0xFB*/
#define foreach_partial_reception_member    \
    __mplast(uint16_t, service_id)    

struct area {
    uint16_t area_code:12;
    uint16_t reserved:4;
};

/*0xFC*/
#define foreach_emergency_information_member    \
    __m1(uint16_t, service_id)  \
    __m(uint8_t, start_end_flag, 1) \
    __m(uint8_t, signal_level, 1)   \
    __m(uint8_t, reserved, 6)   \
    __m1(uint8_t, area_code_length) \
    __mlv(struct area, area_code_length/2, areas)

/*0xFD*/
#define foreach_data_component_member   \
    __m1(uint16_t, data_component_id)    \
    __mplast(uint8_t, additional_identifier_info)

/*0xFE*/
#define foreach_system_management_member    \
    __m1(uint16_t, system_management_id)    \
    __mplast(uint8_t, additional_identifier_info)

#ifdef __cplusplus
}
#endif

#endif /*_ISDB_DESCRIPTOR_H_*/