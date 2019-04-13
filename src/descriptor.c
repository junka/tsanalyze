#include <stdio.h>
#include <stdint.h>
#include "descriptor.h"

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





static void DumpMaxBitrateDescriptor(dvbpsi_max_bitrate_dr_t* bitrate_descriptor)
{  
    printf( "Bitrate: %d\n", bitrate_descriptor->i_max_bitrate);
}


static void DumpSystemClockDescriptor(dvbpsi_system_clock_dr_t* p_clock_descriptor)
{  
    printf( "External clock: %s, Accuracy: %E\n", p_clock_descriptor->b_external_clock_ref ? "Yes" : "No",     p_clock_descriptor->i_clock_accuracy_integer *     pow(10.0, -(double)p_clock_descriptor->i_clock_accuracy_exponent));
}


static void DumpStreamIdentifierDescriptor(dvbpsi_stream_identifier_dr_t* p_si_descriptor)
{  
    printf( "Component tag: %d\n", p_si_descriptor->i_component_tag);
}

static void DumpSubtitleDescriptor(dvbpsi_subtitling_dr_t* p_subtitle_descriptor)
{  
    int a; 
    printf("%d subtitles,\n", p_subtitle_descriptor->i_subtitles_number);  
    for (a = 0; a < p_subtitle_descriptor->i_subtitles_number; ++a)    
    {
        printf( "       | %d - lang: %c%c%c, type: %d, cpid: %d, apid: %d\n", a,         p_subtitle_descriptor->p_subtitle[a].i_iso6392_language_code[0],         p_subtitle_descriptor->p_subtitle[a].i_iso6392_language_code[1],         p_subtitle_descriptor->p_subtitle[a].i_iso6392_language_code[2],         p_subtitle_descriptor->p_subtitle[a].i_subtitling_type,         p_subtitle_descriptor->p_subtitle[a].i_composition_page_id,         p_subtitle_descriptor->p_subtitle[a].i_ancillary_page_id);    
    }
}


static void DumpDescriptors(const char* str, descriptor_t* p_descriptor)
{
    int i;
    while(p_descriptor)
    {
        printf( "%s 0x%02x : ", str, p_descriptor->tag);
        switch (p_descriptor->tag)
        {
            case SYSTEM_CLOCK_DR:
                DumpSystemClockDescriptor(dvbpsi_DecodeSystemClockDr(p_descriptor));
                break;
            case MAX_BITRATE_DR:
                DumpMaxBitrateDescriptor(dvbpsi_DecodeMaxBitrateDr(p_descriptor));         
                break;
            case STREAM_IDENTIFIER_DR:
                DumpStreamIdentifierDescriptor(dvbpsi_DecodeStreamIdentifierDr(p_descriptor));     
                break;
            case SUBTITLING_DR:
                DumpSubtitleDescriptor(dvbpsi_DecodeSubtitlingDr(p_descriptor));      
                break;
            default:
                printf(  "\"");
                for(i = 0; i < p_descriptor->i_length; i++)
                    printf(  "%c", p_descriptor->p_data[i]);
                printf( "\"\n");
        }
        p_descriptor = p_descriptor->p_next;
    }
}

