#include <stdio.h>
#include <stdint.h>

#include "ts.h"
#include "descriptor.h"

int parse_video_stream_descriptor(uint8_t *buf, uint32_t len, video_stream_descriptor_t *vs)
{
	if(buf[0]!=video_stream_descriptor)
		return -1;
	vs->descriptor_tag = video_stream_descriptor;
	vs->descriptor_length = buf[1];
	vs->next = NULL;
	vs->multiple_frame_rate_flag = (buf[2]>>7)&0x01;
	vs->frame_rate_code = (buf[2] >>3)&0x0F;
	vs->MPEG_1_only_flag = (buf[2]>>2)&0x01;
	vs->constrained_parameter_flag = (buf[2]>>1)&0x01;
	vs->still_picture_flag = buf[2]&0x01;
	vs->profile_and_level_indication = buf[3] ;
	vs->chroma_format = buf[4]>>6;
	vs->frame_rate_extension_flag = (buf[4]>>5)&0x01;
	return 0;
}

int parse_audio_stream_descriptor(uint8_t *buf, uint32_t len, audio_stream_descriptor_t *as)
{
	if(buf[0]!=audio_stream_descriptor)
		return -1;
	as->descriptor_tag = audio_stream_descriptor;
	as->descriptor_length = buf[1];
	as->free_format_flag = buf[2]>>7;
	as->ID = (buf[2]>>6)&0x01;
	as->layer = (buf[2]>>4)&0x03;
	as->variable_rate_audio_indicator = (buf[2]>>3)&0x01;
	return 0;
}

int parse_hierarchy_descriptor(uint8_t *buf, uint32_t len, hierarchy_descriptor_t *hi)
{
	if(buf[0]!=hierarchy_descriptor)
		return -1;
	hi->descriptor_tag = hierarchy_descriptor;
	hi->descriptor_length = buf[1];
	hi->hierarchy_type = buf[2] &0x0F;
	hi->hierarchy_layer_index = (buf[3])&0x03F;
	hi->hierarchy_embedded_layer_index = (buf[4])&0x3F;
	hi->hierarchy_channel = (buf[5])&0x3F;
	return 0;
}

int parse_registration_descriptor(uint8_t *buf, uint32_t len, registration_descriptor_t *re)
{
	uint8_t *p = buf;

	if(buf[0]!=registration_descriptor)
		return -1;
	re->descriptor_tag = registration_descriptor;
	re->descriptor_length = buf[1];
	p += 2;
	re->format_identifier = TS_READ32(p);
	re->additional_identification_info = malloc(buf[1]-4);
	memcpy(re->additional_identification_info,buf+6, buf[1]-4);
	return 0;
}

int parse_data_stream_alignment_descriptor(uint8_t *buf, uint32_t len, data_stream_alignment_descriptor_t *dsa)
{
	if(buf[0]!=data_stream_alignment_descriptor)
		return -1;
	dsa->descriptor_tag = data_stream_alignment_descriptor;
	dsa->descriptor_length = buf[1];
	dsa->alignment_type = buf[2];
	return 0;
}

int parse_target_background_grid_descriptor(uint8_t *buf, uint32_t len, target_background_grid_descriptor_t *tbg)
{
	uint8_t *p = buf;
	if(buf[0]!=target_background_grid_descriptor)
		return -1;
	tbg->descriptor_tag = target_background_grid_descriptor;
	tbg->descriptor_length = buf[1];
	tbg->horizontal_size = (buf[2]<<6|buf[3]>>2);//TS_READ32(buf+2) >>18;
	p += 2;
	tbg->vertical_size = (TS_READ32(p)>>4) &0x3FFF;
	tbg->aspect_ratio_information = buf[5]&0x0F;
	return 0;
}

int parse_video_window_descriptor(uint8_t *buf, uint32_t len, video_window_descriptor_t *vw)
{
	uint8_t *p = buf;
	if(buf[0]!=video_window_descriptor)
		return -1;
	vw->descriptor_tag = video_window_descriptor;
	vw->descriptor_length = buf[1];
	vw->next = NULL;
	vw->horizontal_offset = (buf[2]<<6|buf[3]>>2);
	p += 2;
	vw->vertical_offset = (TS_READ32(p)>>4) &0x3FFF;
	vw->window_priority = buf[5]&0x0F;
	return 0;
}

int parse_ca_descriptor(uint8_t *buf, uint32_t len, CA_descriptor_t *ca)
{
	if(buf[0]!=CA_descriptor)
		return -1;
	ca->descriptor_tag = CA_descriptor;
	ca->descriptor_length = buf[1];
	ca->next = NULL;
	ca->CA_system_ID = buf[2]<<8 |buf[3];
	ca->CA_PID = (buf[5]<<8 |buf[6]) & 0x1FFF ;
	return 0;
}

int parse_maximum_bitrate_descriptor(uint8_t *buf, uint32_t len, maximum_bitrate_descriptor_t *mb)
{
	if(buf[0]!=maximum_bitrate_descriptor)
		return -1;
	mb->descriptor_tag = maximum_bitrate_descriptor;
	mb->descriptor_length = buf[1];
	mb->next = NULL;
	mb->maximum_bitrate.bits = ((buf[2]<<16 |buf[3]<<8 |buf[4])&0x3FFFFF);
	return 0;
}


static int parse_system_clock_descriptor(uint8_t *buf, uint32_t len, system_clock_descriptor_t *sc)
{
	if(buf[0]!=system_clock_descriptor)
		return -1;
	sc->descriptor_tag = maximum_bitrate_descriptor;
	sc->descriptor_length = buf[1];
	sc->next = NULL;
	sc->external_clock_reference_indicator = buf[2]>>7;
	sc->clock_accuracy_integer = buf[2]&0x3F;
	sc->clock_accuracy_exponent = buf[3]>>5;
	return 0;
}

void dump_system_clock_descriptor(system_clock_descriptor_t* p_descriptor)
{
	fprintf(stdout,"external_clock_reference_indicator: %d",
		p_descriptor->external_clock_reference_indicator);
	fprintf(stdout,"clock_accuracy_integer: %d",
		p_descriptor->clock_accuracy_exponent);
	fprintf(stdout,"clock_accuracy_exponent: %d",
		p_descriptor->clock_accuracy_exponent);
}

void dump_maxbitrate_descriptor(maximum_bitrate_descriptor_t *p_descriptor)
{
	fprintf(stdout,"maximum_bitrate: %d",
		p_descriptor->maximum_bitrate);

}

void dump_descriptors(const char* str, descriptor_t* p_descriptor)
{
    int i;
    while(p_descriptor)
    {
        printf( "%s 0x%02x : ", str, p_descriptor->tag);
        switch (p_descriptor->tag)
        {
            case system_clock_descriptor:
                dump_system_clock_descriptor(p_descriptor);
                break;
            case maximum_bitrate_descriptor:
                dump_maxbitrate_descriptor(p_descriptor);
                break;
            case stream_identifier_descriptor:
                //DumpStreamIdentifierDescriptor(dvbpsi_DecodeStreamIdentifierDr(p_descriptor));     
                break;
            case subtitling_descriptor:
                //DumpSubtitleDescriptor(dvbpsi_DecodeSubtitlingDr(p_descriptor));      
                break;
            default:
                printf(  "\"");
                for(i = 0; i < p_descriptor->length; i++)
                    printf(  "%c", p_descriptor->data[i]);
                printf( "\"\n");
        }
        p_descriptor = p_descriptor->next;
    }
}

