#include "table.h"

static char const* get_stream_type(uint8_t type)
{
    switch (type)
    {
        case 0x00:
            return "Reserved";
        case 0x01:
            return "ISO/IEC 11172 Video";
        case 0x02:
            return "ISO/IEC 13818-2 Video";
        case 0x03:
            return "ISO/IEC 11172 Audio";
        case 0x04:
            return "ISO/IEC 13818-3 Audio";
        case 0x05:
            return "ISO/IEC 13818-1 Private Section";
        case 0x06:
            return "ISO/IEC 13818-1 Private PES data packets";    
        case 0x07:
            return "ISO/IEC 13522 MHEG";
        case 0x08:
            return "ISO/IEC 13818-1 Annex A DSM CC";
        case 0x09:
            return "H222.1";
        case 0x0A:
            return "ISO/IEC 13818-6 type A";
        case 0x0B:
            return "ISO/IEC 13818-6 type B";
        case 0x0C:
            return "ISO/IEC 13818-6 type C";
        case 0x0D:
            return "ISO/IEC 13818-6 type D";
        case 0x0E:
            return "ISO/IEC 13818-1 auxillary";
        default:
            if (type < 0x80)
                return "ISO/IEC 13818-1 reserved";
            else
                return "User Private";
        }
}

static int pat_proc(uint8_t *pkt,uint8_t len)
{
	pmt t;
	t.table_id = pkt[0];
	
}

static int pmt_proc(uint8_t *pkt,uint8_t len)
{
	
}

static table_ops pat_ops = {
	.table_id = PAT_TID,
	.table_proc = pat_proc,
};

static table_ops pmt_ops = {
	.table_id = PMT_TID,
	.table_proc = pmt_proc,
};


