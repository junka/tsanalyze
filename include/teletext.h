#ifndef _TELETEXT_H_
#define _TELETEXT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* see en 300472 */
// data_identifier 0x10 - 0x1F

enum teletext_data_unit_id {
    /* 0x00 to 0x01 reserved for future use */
    TELETEXT_NON_SUBTITLE = 0x02,
    TELETEXT_SUBTITLE = 0x03,
    /* 0x04 to 0x7F reserved for future use */
    /* 0x80 to 0xFE user defined */
    TELETEXT_STUFFING = 0xFF,
};

struct teletext_pes_data {
    uint8_t data_identifier; /* 0x10 to 0x1F */

    int n_units;
    struct data_unit {
        uint8_t data_unit_id; 
        uint8_t data_unit_length; /* should be 0x2C */
        struct data_field {
            uint8_t reserved_future_use:2;
            uint8_t field_parity :1;
            uint8_t line_offset: 5;
            uint8_t framing_code;
            uint16_t magazine_and_packet_address;
            uint8_t data_block[40];
        } data;
    } *units;
};

int parse_teletext(uint16_t pid, uint8_t *pbuf, int len, void *subtitle);

void dump_teletext(struct teletext_pes_data *text);

void free_teletext(struct teletext_pes_data *text);

#ifdef __cplusplus
}
#endif

#endif /*_TELETEXT_H_*/
