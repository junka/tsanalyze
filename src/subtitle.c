#include <stdint.h>
#include <stdio.h>

#include "types.h"
#include "ts.h"
#include "table.h"
#include "pes.h"
#include "subtitle.h"


int parse_subtitle_segment_header(uint8_t *pbuf, int len, struct segment_header *h)
{
    uint8_t *pdata = pbuf;
    int hlen = 0;
    h->sync_byte = TS_READ8(pdata);
    if (h->sync_byte != 0x0F) {
        printf("Must be sync error, drop parsing\n");
        return -1;
    }
    hlen += 1;
    h->segment_type = TS_READ8(pdata);
    hlen += 1;
    h->page_id = TS_READ16(pdata);
    hlen += 2;
    h->segment_length = TS_READ16(pdata);
    hlen += 2;

    return hlen;
}

int parse_subtitle_segment(uint16_t pid, uint8_t *pbuf, int len)
{
    struct segment_header seg;
    int hen = parse_subtitle_segment_header(pbuf, len, &seg);
    printf("segment type %d\n", seg.segment_type);
    switch (seg.segment_type) {
        case display_definition_segment:
            break;
        case page_composition_segment:
            break;
        case region_composition_segment:
            break;
        case CLUT_composition_segment:
            break;
        case object_data_segment:
            break;
        case disparity_signalling_segment:
            break;
        case alternative_CLUT_segment:
            break;
        case end_of_display_set_segment:
            /* only header here */
            break;
        default:
            break;
    }

    return 0;
}

void init_subtitle_parser()
{
    register_pes_data_callback(STEAM_TYPE_MPEG2_PES, parse_subtitle_segment);
}