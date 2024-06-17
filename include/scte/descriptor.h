#ifndef _SCTE_DESCRIPTOR_H_
#define _SCTE_DESCRIPTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#define foreach_enum_scte_pmt_descriptor \
    _(cue_identifier, 0x8A)


enum cue_stream_type {
    CUE_STREAM_TYPE_SPLICE = 0,
    CUE_STREAM_TYPE_ALL = 1,
    CUE_STREAM_TYPE_SEGMEMTATION = 2,
    CUE_STREAM_TYPE_TIERED_SPLICE = 3,
    CUE_STREAM_TYPE_TIERED_SEGMETATION = 4,
    /* 0x05 - 0x7F reserverd */
    /* 0x80 - 0xFF user defined */
};

#define foreach_cue_identifier_member \
    __m1(uint8_t, cue_stream_type)



/* splice descriptors below are only used within a splice_info_section.
 * They are not to be used within MPEG syntax, such as the PMT, or
 * in the syntax of any other standard. This allows one to draw
 * on the entire range of descriptor tags when defining new descriptors.
 */
#define foreach_enum_scte_descriptor \
	_(avail, 0x00)  \
	_(DTMF, 0x01)   \
	_(segmentation, 0x02)   \
	_(time, 0x03)   \
	_(audio, 0x04)
	/* 0x05 - 0xEF  reserved for SCTE splice */
	/* 0xF0 - 0xFF  reserved fir DVB use, DVB in ETSI TS 103 752-1 */

/* see SCTE 35 table 17 */
struct splice_desciptor {
	uint8_t splice_descriptor_tag;
	uint8_t descriptor_length;
	uint32_t identifier;
	uint8_t private_byte[0];
};

#define foreach_avail_member \
	__m1(uint32_t, provider_avail_id)

#define foreach_DTMF_member \
	__m1(uint8_t, preroll)   \
	__m(uint8_t, dtmf_count, 3) \
	__m(uint8_t, reserved, 5) \
	__mlv(uint8_t, dtmf_count, DTMF_char)

#define foreach_segmentation_member \
	__m1(uint32_t, segmentation_event_id)   \
	__m(uint8_t, segmentation_event_cancel_indicator, 1) \
	__m(uint8_t, reserved0, 7) \
	__mif(uint8_t, program_segmentation_flags, segmentation_event_cancel_indicator, 0) \
	__mplast(uint8_t, program_segmentation)

#define foreach_time_member \
	__m1(uint48_t, TAI_seconds) \
	__m1(uint32_t, TAI_ns) \
	__m1(uint16_t, UTC_offset)

struct audio_component {
    uint8_t component_tag;
    uint32_t ISO_code:24;
    uint32_t Bit_Stream_Mode:3;
    uint32_t Num_Channels:4;
    uint32_t Full_Srvc_Audio:1;
} PACK;

#define foreach_audio_member \
	__m(uint8_t, audio_count, 4)    \
	__m(uint8_t, reserved, 4)    \
	__mlv(struct audio_component, audio_count, components)

#ifdef __cplusplus
}
#endif

#endif /*_SCTE_DESCRIPTOR_H_*/