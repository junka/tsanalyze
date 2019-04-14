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


int parse_pat(uint8_t * pbuf, uint32_t buf_size, pat_t * pPAT)
{
	uint32_t section_len = 0;
	uint8_t *pdata = pbuf;

	if (pbuf == NULL || pPAT == NULL)
	{
		return -1;
	}

	if (*pbuf != PAT_TID)
	{
		return -1;
	}

	section_len = ((pdata[1] << 8) | pdata[2])  & 0x0FFF;
	if ((section_len + 3) != buf_size)
	{
		return -1;
	}
	pPAT->section_length = section_len;

	//Transport Stream ID
	pPAT->transport_stream_id = (pdata[3] << 8) | pdata[4];

	pPAT->version_number = (pdata[5] >> 1) & 0x1F;

	if (!(pdata[5] & 0x01)) //current_next_indicator
	{
		return -1;
	}

	pPAT->section_number = pdata[6];
	pPAT->last_section_number = pdata[7];

	section_len -= 5 + 4; 
	pdata += 8;
	
	//TODO: limit program total length
	pPAT->list = NULL;

	while (section_len > 0)
	{
		struct program_list *pl = malloc(sizeof(struct program_list));
		struct program_list *next = NULL;
		pl->program_number = (pdata[0] << 8) + pdata[1]; 
		pl->program_map_PID = ((pdata[2] << 8) + pdata[3]) & 0x1FFF;
		next = pPAT->list;
		pPAT->list = pl;
		pl->next = next;
		pdata += 4;
		section_len -= 4;
	}

	return 0;
}


int parse_cat(uint8_t * pbuf, uint32_t buf_size, cat_t * pCAT)
{
	uint32_t section_len = 0;
	uint8_t *pdata = pbuf;

	if (pbuf == NULL || pCAT == NULL)
	{
		return -1;
	}

	if (*pbuf != CAT_TID)
	{
		return -1;
	}

	section_len = ((pdata[1] << 8) | pdata[2])  & 0x0FFF;
	if ((section_len + 3) != buf_size)
	{
		return -1;
	}
	pCAT->section_length = section_len;

	//Transport Stream ID
	pCAT->transport_stream_id = (pdata[3] << 8) | pdata[4];

	pCAT->version_number = (pdata[5] >> 1) & 0x1F;

	if (!(pdata[5] & 0x01)) //current_next_indicator
	{
		return -1;
	}

	pCAT->section_number = pdata[6];
	pCAT->last_section_number = pdata[7];

	section_len -= 5 + 4; 
	pdata += 8;
	
	//TODO: limit program total length
	pCAT->list = NULL;

	while (section_len > 0)
	{
		struct program_list *pl = malloc(sizeof(struct program_list));
		struct program_list *next = NULL;
		pl->program_number = (pdata[0] << 8) + pdata[1]; 
		pl->program_map_PID = ((pdata[2] << 8) + pdata[3]) & 0x1FFF;
		next = pCAT->list;
		pCAT->list = pl;
		pl->next = next;
		pdata += 4;
		section_len -= 4;
	}

	return 0;
}


int parse_pmt(uint8_t * pbuf, uint32_t buf_size, pmt_t * pPMT)
{
	uint32_t section_len = 0;
	uint8_t *pdata = pbuf;

	if (pbuf == NULL || pPMT == NULL)
	{
		return -1;
	}

	if (*pbuf != PMT_TID)
	{
		return -1;
	}

	section_len = ((pdata[1] << 8) | pdata[2])  & 0x0FFF;
	if ((section_len + 3) != buf_size)
	{
		return -1;
	}
	pPMT->section_length = section_len;

	//Transport Stream ID
	pPMT->program_number = (pdata[3] << 8) | pdata[4];

	pPMT->version_number = (pdata[5] >> 1) & 0x1F;

	if (!(pdata[5] & 0x01)) //current_next_indicator
	{
		return -1;
	}

	pPMT->section_number = pdata[6];
	pPMT->last_section_number = pdata[7];

	section_len -= 5 + 4; 
	pdata += 8;
	
	pPMT->desriptor_list = NULL;

	while (section_len > 0)
	{
		struct program_list *pl = malloc(sizeof(struct program_list));
		struct program_list *next = NULL;
		pl->program_number = (pdata[0] << 8) + pdata[1]; 
		pl->program_map_PID = ((pdata[2] << 8) + pdata[3]) & 0x1FFF;
		next = pPMT->desriptor_list;
		pPMT->desriptor_list = pl;
		pl->next = next;
		pdata += 4;
		section_len -= 4;
	}

	return 0;
}

static int pat_proc(uint8_t *pkt,uint8_t len)
{
	return 0;
}

static int cat_proc(uint8_t *pkt,uint8_t len)
{
	return 0;
}

static int pmt_proc(uint8_t *pkt,uint8_t len)
{
	return 0;
}

static table_ops pat_ops = {
	.table_id = PAT_TID,
	.table_proc = pat_proc,
};

static table_ops cat_ops = {
	.table_id = CAT_TID,
	.table_proc = cat_proc,
};

static table_ops pmt_ops = {
	.table_id = PMT_TID,
	.table_proc = pmt_proc,
};


