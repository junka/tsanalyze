#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "ts.h"
#include "table.h"
#include "pes.h"
#include "teletext.h"
#include "utils.h"




int parse_teletext(uint16_t pid, uint8_t *pbuf, int len, void *teletext)
{
    uint8_t *pdata = pbuf;
    int t_len = 0;
    struct teletext_pes_data *text = (struct teletext_pes_data *)teletext;
    if (text->units) {
        free(text->units);
    }
    text->data_identifier = TS_READ8(pdata);
    pdata += 1;
    t_len += 1;
    text->n_units = (len - 1) / (2 + sizeof(struct data_field));
    text->units = calloc(text->n_units, sizeof(struct data_unit));
    int i = 0, j = 0;
    while (t_len < len) {
        text->units[i].data_unit_id = TS_READ8(pdata);
        assert(text->units[i].data_unit_id == 0x2 || text->units[i].data_unit_id == 0x3 ||text->units[i].data_unit_id == 0xff);
        pdata += 1;
        text->units[i].data_unit_length = TS_READ8(pdata);
        assert(text->units[i].data_unit_length == 0x2c);
        pdata += 1;
        text->units[i].data.field_parity = TS_READ8_BITS(pdata, 1, 2);
        text->units[i].data.line_offset = TS_READ8_BITS(pdata, 5, 3);
        pdata += 1;
        text->units[i].data.framing_code = TS_READ8(pdata);
        pdata += 1;
        text->units[i].data.magazine_and_packet_address = TS_READ16(pdata);
        pdata += 2;
        for (j = 0; j < 40; j ++) {
            text->units[i].data.data_block[j] = TS_READ8(pdata);
            pdata += 1;
        }
        t_len += 46;
        i ++;
    }
    // dump_teletext(text);
    return 0;
}

void dump_teletext(struct teletext_pes_data *text)
{
    rout(2, "data_identifier", "0x%x", text->data_identifier);
    for (int i = 0; i < text->n_units; i ++) {
        rout(3, "data_unit_id", "0x%x", text->units[i].data_unit_id);
        rout(3, "data_unit_length", "%d", text->units[i].data_unit_length);
        rout(3, "field_parity", "%d", text->units[i].data.field_parity);
        rout(3, "line_offset", "%d->%d", text->units[i].data.line_offset, (text->units[i].data.line_offset > 0x6 && text->units[i].data.line_offset < 0x17)?
                ( text->units[i].data.field_parity ? text->units[i].data.line_offset : text->units[i].data.line_offset + 313) : -1);
        rout(3, "field_parity", "%d", text->units[i].data.field_parity);
        rout(3, "framing_code", "%d", text->units[i].data.framing_code);
        rout(3, "magazine_and_packet_address", "0x%x", text->units[i].data.magazine_and_packet_address);
        res_hexdump(3, "data_block", text->units[i].data.data_block, 40);
    }
}

void free_teletext(struct teletext_pes_data *text)
{
    free(text->units);
}