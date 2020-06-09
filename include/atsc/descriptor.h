#ifndef _ATSC_DESCRIPTOR_H_
#define _ATSC_DESCRIPTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#define foreach_enum_atsc_descriptor                                                                                   \
	_(ATSC_AC3_audio_stream, 0x81)                                                                                     \
	_(ATSC_private_information, 0xAD)                                                                                  \
	_(ATSC_enhanced_signaling, 0xB2)

#define foreach_ATSC_AC3_audio_stream_member                                                                           \
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

#define foreach_ATSC_private_information_member \
    __m1(uint32_t, format_identifier) \
    __mplast(uint8_t, private_data_byte)

#define foreach_ATSC_enhanced_signaling_member                                                                         \
	__m(uint8_t, linkage_preference, 2) \
    __m(uint8_t, tx_method, 2) \
    __m(uint8_t, linked_component_tag, 4)

#ifdef __cplusplus
}
#endif

#endif /* _ATSC_DESCRIPTOR_H_*/