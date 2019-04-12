#include <stdio.h>
#include <stdint.h>
#include "descriptor.h"

int ParsePAT(uint8_t * pBuffer, uint32_t uiBufSize, pat_t * pPAT)
{
	int iSectionLength = 0;
	uint8_t *pData = pBuffer;

	if (pBuffer == NULL || pPAT == NULL)
	{
		return -1;
	}

	if (*pBuffer != PAT_TID)
	{
		return -1;
	}

	iSectionLength = ((pData[1] << 8) | pData[2])  & 0x0FFF;
	if ((uint32_t)(iSectionLength + 3) != uiBufSize)
	{
		return -1;
	}

	//Transport Stream ID
	pPAT->transport_stream_id = (pData[3] << 8) | pData[4];


	pPAT->version_number = (pData[5] >> 1) & 0x1F;

	if (!(pData[5] & 0x01)) //current_next_indicator
	{
			return -1;
	}

	pPAT->section_number = pData[6];
	pPAT->last_section_number = pData[7];

	iSectionLength -= 5 + 4; 
	pData += 8;
	
	//TODO: limit program total length
	pPAT->list = NULL;

	while (iSectionLength > 0)
	{
		struct program_list *pl = malloc(sizeof(struct program_list));
		struct program_list *next = NULL;
		pl->program_number = (pData[0] << 8) + pData[1]; 
		pl->program_map_PID = ((pData[2] << 8) + pData[3]) & 0x1FFF;
		next = pPAT->list;
		pPAT->list = pl;
		pl->next = next;
		pData += 4;
		iSectionLength -= 4;
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

