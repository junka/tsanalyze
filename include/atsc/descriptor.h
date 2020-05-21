#ifndef _ATSC_DESCRIPTOR_H_
#define _ATSC_DESCRIPTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"
#include "types.h"

#define foreach_enum_atsc_descriptor    \
        _(ATSC_AC3_audio_stream, 0x81)	\
        _(ATSC_private_information, 0xAD)	\
        _(ATSC_enhanced_signaling, 0xB2)


typedef struct {
    descriptor_t descriptor;		
    uint8_t sample_rate_code : 3;
    uint8_t bsid : 5;
    uint8_t bit_rate_code : 6;
    uint8_t surround_mode : 2;
    uint8_t bsmod : 3;
    uint8_t num_channels : 4;
    uint8_t full_svc : 1;
    uint8_t langcod;
    uint8_t langcod2;
    union{
        uint8_t mainid : 3;
        uint8_t priority : 2;
        uint8_t reserved : 3;
        uint8_t asvcflags;
    };
    uint8_t textlen : 7;
    uint8_t text_code : 1;
    uint8_t *text;
    uint8_t language_flag : 1;
    uint8_t language_flag_2 : 1;
    uint8_t reserved1 : 6;
    uint24_t language;
    uint24_t language_2;
    uint8_t *additional_info;		
} ATSC_AC3_audio_stream_descriptor_t;

typedef struct {
    descriptor_t descriptor;
    uint32_t format_identifier;
    uint8_t *private_data_byte;		
} ATSC_private_information_descriptor_t;

typedef struct {
    descriptor_t descriptor;
    uint8_t linkage_preference : 2;
    uint8_t tx_method : 2;
    uint8_t linked_component_tag : 4;
} ATSC_enhanced_signaling_descriptor_t;


#ifdef __cplusplus
}
#endif

#endif /* _ATSC_DESCRIPTOR_H_*/