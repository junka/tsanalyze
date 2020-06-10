#ifndef _ATSC_DESCRIPTOR_H_
#define _ATSC_DESCRIPTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#define foreach_enum_atsc_descriptor                                                                                   \
	_(atsc_stuffing, 0x80)                                                                                             \
    _(AC3_audio_stream, 0x81)                                                                                     \
	_(caption_service, 0x86)                                                                                      \
	_(content_advisory, 0x87)                                                                                     \
	_(extended_channel_name, 0xA0)                                                                                \
    _(service_location, 0xA1)                                                                                     \
    _(atsc_time_shifted_service, 0xA2)                                                                                 \
    _(component_name, 0xA3)                                                                                       \
    _(dcc_departing_request, 0xA8)                                                                                \
    _(dcc_arriving_request, 0xA9)                                                                                 \
    _(rc, 0xAA)                                                                                                   \
    _(genre, 0xAB)                                                                                                \
    _(private_information, 0xAD)                                                                                  \
	_(enhanced_signaling, 0xB2)                                                                                   \
    _(enhanced_AC3_audio_stream, 0xCC)

#define foreach_atsc_stuffing_member \
    __mplast(uint8_t, ignore)

#define foreach_AC3_audio_stream_member                                                                           \
	__m(uint8_t, sample_rate_code, 3) \
    __m(uint8_t, bsid, 5) \
    __m(uint8_t, bit_rate_code, 6)                             \
	__m(uint8_t, surround_mode, 2) \
    __m(uint8_t, bsmod, 3) \
    __m(uint8_t, num_channels, 4) \
    __m(uint8_t, full_svc, 1)  \
	__m1(uint8_t, langcod) \
    __mif(uint8_t, langcod2, num_channels, 0) \
    __m(uint8_t, mainid, 3)                   \
	__m(uint8_t, priority, 2) \
    __m(uint8_t, reserved, 3) \
    __m(uint8_t, textlen, 7)                           \
	__m(uint8_t, text_code, 1) \
    __mlv(uint8_t, textlen, text) \
    __m(uint8_t, language_flag, 1)            \
	__m(uint8_t, language_flag_2, 1) \
    __m(uint8_t, reserved1, 6)                                    \
	__mif(uint24_t, language, language_flag, 1)                                                \
	__mif(uint24_t, language_2, language_flag_2, 1) \
    __mplast(uint8_t, additional_info)

struct caption_service_info {
    uint24_t language;
    uint8_t digital_cc : 1;
    uint8_t reserved : 1;
    uint8_t caption_service_number : 6;/*if digital_cc == 0, only last bit valid as line21_field */
    uint16_t easy_reader : 1;
    uint16_t wide_aspect_ratio : 1;
    uint16_t reserved1 : 14;
};

#define foreach_caption_service_member \
    __m(uint8_t, reserved, 3)   \
    __m(uint8_t, number_of_services, 5) \
    __mplast(struct caption_service_info, captions)

struct rated_dimension {
    uint8_t rating_dimension_j;
    uint8_t reserved : 4;
    uint8_t rating_value : 4;
};

struct content_advisory_info {
    uint8_t rating_region;
    uint8_t rated_dimensions;
    struct rated_dimension * dimensions;
    uint8_t rating_description_length;
    uint8_t * rating_description_text;
};

#define foreach_content_advisory_member    \
    __m(uint8_t, reserved, 2)   \
    __m(uint8_t, rating_region_count, 6)    \
    __mplast(struct content_advisory_info, rating_region)

#define foreach_extended_channel_name_member   \
    __mplast(uint8_t, long_channel_name_text)

struct service_location_info{
    uint8_t stream_type;
    uint16_t reserved:3;
    uint16_t elementary_PID:13;
    uint24_t ISO_639_language_code;
}__attribute__((packed));

#define foreach_service_location_member \
    __m(uint16_t, reserved, 3)      \
    __m(uint16_t, PCR_PID, 13)  \
    __m1(uint8_t, number_elements)  \
    __mplast(struct service_location_info, service_locations)

struct time_shifted_service{
    uint16_t reserved : 6;
    uint16_t time_shift : 10;
    uint32_t reserved1 : 4;
    uint32_t major_channel_number : 10;
    uint32_t minor_channel_number : 10;
};

#define foreach_atsc_time_shifted_service_member \
    __m(uint8_t, reserved, 3)   \
    __m(uint8_t, number_of_services, 5) \
    __mplast(struct time_shifted_service, time_shift_services)

#define foreach_component_name_member   \
    __mplast(uint8_t, component_name_string)

#define foreach_dcc_departing_request_member    \
    __m1(uint8_t, dcc_departing_request_type)   \
    __m1(uint8_t, dcc_departing_request_text_length)    \
    __mlv(uint8_t, dcc_departing_request_text_length, dcc_departing_request_text)

#define foreach_dcc_arriving_request_member    \
    __m1(uint8_t, dcc_arriving_request_type)    \
    __m1(uint8_t, dcc_arriving_request_text_length) \
    __mlv(uint8_t, dcc_arriving_request_text_length, dcc_arriving_request_text)

#define foreach_rc_member   \
    __mplast(uint8_t, rc_information)

#define foreach_genre_member    \
    __m(uint8_t, reserved, 3)   \
    __m(uint8_t, attribute_count, 5)    \
    __mlv(uint8_t, attribute_count, attribute)

#define foreach_private_information_member \
    __m1(uint32_t, format_identifier) \
    __mplast(uint8_t, private_data_byte)

#define foreach_enhanced_signaling_member                                                                         \
	__m(uint8_t, linkage_preference, 2) \
    __m(uint8_t, tx_method, 2) \
    __m(uint8_t, linked_component_tag, 4)

#define foreach_enhanced_AC3_audio_stream_member    \
    __m(uint8_t, reserved, 1)   \
	__m(uint8_t, bsid_flag, 1)	\
	__m(uint8_t, mainid_flag, 1)	\
	__m(uint8_t, asvc_flag, 1)	\
	__m(uint8_t, mixinfoexists, 1)	\
	__m(uint8_t, substream1_flag, 1)	\
	__m(uint8_t, substream2_flag, 1)	\
	__m(uint8_t, substream3_flag, 1)	\
    __m(uint8_t, reserved1, 1)  \
    __m(uint8_t, full_service_flag, 1)  \
    __m(uint8_t, audio_service_type, 3) \
    __m(uint8_t, number_of_channels, 3) \
    __m(uint8_t, language_flag, 1)  \
    __m(uint8_t, language_flag_2, 1)  \
    __m(uint8_t, bsid, 5)   \
    __mif(uint8_t, priority_mainid, mainid_flag, 1)	\
	__mif(uint8_t, asvc, asvc_flag, 1)	\
	__mif(uint8_t, substream1, substream1_flag, 1)	\
	__mif(uint8_t, substream2, substream2_flag, 1)	\
	__mif(uint8_t, substream3, substream3_flag, 1)	\
	__mif(uint24_t, language, language_flag, 1)	\
	__mif(uint24_t, language_2, language_flag_2, 1)	\
	__mif(uint24_t, substream1_lang, substream1_flag, 1)	\
	__mif(uint24_t, substream2_lang, substream2_flag, 1)	\
	__mif(uint24_t, substream3_lang, substream3_flag, 1)	\
	__mplast(uint8_t, additional_info_byte)

#ifdef __cplusplus
}
#endif

#endif /* _ATSC_DESCRIPTOR_H_*/