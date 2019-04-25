#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ts.h"
#include "descriptor.h"


int parse_video_stream_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	if(buf[0]!=dr_video_stream)
		return -1;
	video_stream_descriptor_t * vs = (video_stream_descriptor_t*)ptr;
	vs->descriptor_tag = dr_video_stream;
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

int parse_audio_stream_descriptor(uint8_t *buf, uint32_t len,void *ptr )
{
	if(buf[0]!= dr_audio_stream)
		return -1;
	audio_stream_descriptor_t *as = (audio_stream_descriptor_t *)ptr;
	as->descriptor_tag = dr_audio_stream;
	as->descriptor_length = buf[1];
	as->free_format_flag = buf[2]>>7;
	as->ID = (buf[2]>>6)&0x01;
	as->layer = (buf[2]>>4)&0x03;
	as->variable_rate_audio_indicator = (buf[2]>>3)&0x01;
	return 0;
}

int parse_hierarchy_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	if(buf[0]!=dr_hierarchy)
		return -1;
	hierarchy_descriptor_t *hi = (hierarchy_descriptor_t *)ptr;
	hi->descriptor_tag = dr_hierarchy;
	hi->descriptor_length = buf[1];
	hi->hierarchy_type = buf[2] &0x0F;
	hi->hierarchy_layer_index = (buf[3])&0x03F;
	hi->hierarchy_embedded_layer_index = (buf[4])&0x3F;
	hi->hierarchy_channel = (buf[5])&0x3F;
	return 0;
}

int parse_registration_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	uint8_t *p = buf;

	if(buf[0]!=dr_registration)
		return -1;
	registration_descriptor_t *re = (registration_descriptor_t *)ptr;
	re->descriptor_tag = dr_registration;
	re->descriptor_length = buf[1];
	p += 2;
	re->format_identifier = TS_READ32(p);
	re->additional_identification_info = malloc(buf[1]-4);
	memcpy(re->additional_identification_info,buf+6, buf[1]-4);
	return 0;
}

int parse_data_stream_alignment_descriptor(uint8_t *buf, uint32_t len,  void* ptr)
{
	if(buf[0]!=dr_data_stream_alignment)
		return -1;
	data_stream_alignment_descriptor_t *dsa = (data_stream_alignment_descriptor_t*)ptr;
	dsa->descriptor_tag = dr_data_stream_alignment;
	dsa->descriptor_length = buf[1];
	dsa->alignment_type = buf[2];
	return 0;
}

int parse_target_background_grid_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	uint8_t *p = buf;
	if(buf[0]!=dr_target_background_grid)
		return -1;
	target_background_grid_descriptor_t * tbg = (target_background_grid_descriptor_t*)ptr;
	tbg->descriptor_tag = dr_target_background_grid;
	tbg->descriptor_length = buf[1];
	tbg->horizontal_size = (buf[2]<<6|buf[3]>>2);//TS_READ32(buf+2) >>18;
	p += 2;
	tbg->vertical_size = (TS_READ32(p)>>4) &0x3FFF;
	tbg->aspect_ratio_information = buf[5]&0x0F;
	return 0;
}

int parse_video_window_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	uint8_t *p = buf;
	if(buf[0]!=dr_video_window)
		return -1;
	video_window_descriptor_t *vw = (video_window_descriptor_t *)ptr;
	vw->descriptor_tag = dr_video_window;
	vw->descriptor_length = buf[1];
	vw->next = NULL;
	vw->horizontal_offset = (buf[2]<<6|buf[3]>>2);
	p += 2;
	vw->vertical_offset = (TS_READ32(p)>>4) &0x3FFF;
	vw->window_priority = buf[5]&0x0F;
	return 0;
}

int parse_CA_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	if(buf[0]!=dr_CA)
		return -1;
	CA_descriptor_t *ca = (CA_descriptor_t *)ptr;
	ca->descriptor_tag = dr_CA;
	ca->descriptor_length = buf[1];
	ca->next = NULL;
	ca->CA_system_ID = buf[2]<<8 |buf[3];
	ca->CA_PID = (buf[5]<<8 |buf[6]) & 0x1FFF ;
	return 0;
}
int parse_ISO_639_language_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}

static int parse_system_clock_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	if(buf[0]!=dr_system_clock)
		return -1;
	system_clock_descriptor_t *sc = (system_clock_descriptor_t *)ptr;
	sc->descriptor_tag = dr_maximum_bitrate;
	sc->descriptor_length = buf[1];
	sc->next = NULL;
	sc->external_clock_reference_indicator = buf[2]>>7;
	sc->clock_accuracy_integer = buf[2]&0x3F;
	sc->clock_accuracy_exponent = buf[3]>>5;
	return 0;
}

int parse_multiplex_buffer_utilization_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}

int parse_copyright_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}

int parse_maximum_bitrate_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	if(buf[0]!=dr_maximum_bitrate)
		return -1;
	maximum_bitrate_descriptor_t *mb = (maximum_bitrate_descriptor_t *)ptr;
	mb->descriptor_tag = dr_maximum_bitrate;
	mb->descriptor_length = buf[1];
	mb->next = NULL;
	mb->maximum_bitrate.bits = ((buf[2]<<16 |buf[3]<<8 |buf[4])&0x3FFFFF);
	return 0;
}

int parse_private_data_indicator_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}

int parse_smoothing_buffer_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_STD_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_ibp_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_MPEG4_video_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_MPEG4_audio_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_IOD_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}

int parse_SL_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_FMC_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_external_ES_ID_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_muxcode_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_FmxBufferSize_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_MultiplexBuffer_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_network_name_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}int parse_service_list_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_stuffing_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_satellite_delivery_system_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_cable_delivery_system_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_VBI_data_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_VBI_teletext_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_bouquet_name_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_service_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_country_availability_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_linkage_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_NVOD_reference_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}int parse_time_shifted_service_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}int parse_short_event_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}int parse_extended_event_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}int parse_time_shifted_event_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}int parse_component_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}int parse_mosaic_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}int parse_stream_identifier_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}int parse_CA_identifier_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}int parse_content_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_parental_rating_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_teletext_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_telephone_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_local_time_offset_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_subtitling_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_terrestrial_delivery_system_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_multilingual_network_name_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_multilingual_bouquet_name_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_multilingual_service_name_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_multilingual_component_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_private_data_specifier_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_service_move_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_short_smoothing_buffer_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_frequency_list_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_partial_transport_stream_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_data_broadcast_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_scrambling_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_data_broadcast_id_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_transport_stream_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_DSNG_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_PDC_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_AC3_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_ancillary_data_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_cell_list_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_cell_frequency_link_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_announcement_support_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_application_signalling_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_adaptation_field_data_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_service_identifier_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_service_availability_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_default_authority_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_related_content_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}

int parse_TVA_id_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}

int parse_content_identifier_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}

int parse_time_slice_fec_identifier_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_ECM_repetition_rate_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_S2_satellite_delivery_system_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_enhanced_AC3_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_DTS_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_AAC_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_XAIT_location_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}
int parse_FTA_content_management_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}

int parse_extension_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	return 0;
}

int parse_reserved_descriptor(uint8_t *buf,uint32_t len,void *ptr)
{
	return 0;
}
void *alloc_reserved()
{
	return malloc(sizeof(descriptor_t));
}


#define FUNC(descriptor) parse_##descriptor##_descriptor

#define ALLOC(descriptor) void *alloc_##descriptor(){ return malloc(sizeof(descriptor##_descriptor_t));}
#define _(a,b) ALLOC(a)
	foreach_enum_descriptor
#undef _

static struct descriptor_ops des_ops[255];

void init_descriptor_parsers()
{
	uint8_t i=0;
	for(i=0;i<0xFF;i++)
	{
		des_ops[i].tag = i;
		des_ops[i].descriptor_parse = parse_reserved_descriptor;
		des_ops[i].descriptor_alloc = alloc_reserved;
	}

#define _(a,b) des_ops[b].tag = b;des_ops[b].descriptor_parse = FUNC(a); des_ops[b].descriptor_alloc = alloc_##a ;
	foreach_enum_descriptor
#undef _

}


descriptor_t* parse_descriptors(uint8_t *buf, uint32_t len)
{
	uint32_t l = len;
	uint8_t* ptr = buf;
	descriptor_t* h = NULL, *more;
	while( l )
	{
		void* des = des_ops[ptr[0]].descriptor_alloc();
		descriptor_t *more = (descriptor_t*) des;
		des_ops[ptr[0]].descriptor_parse(buf,ptr[1]+2,des);
		more->next = h;
		more->tag = ptr[0];
		more->length = ptr[1];
		l -= more->length+ 2;
		ptr += more->length+ 2;
		h = more;
	}
	return h;
}

void free_descriptors(descriptor_t *des)
{
	descriptor_t* t = des,*n;
	while(t)
	{
		n = t->next;
		free(t);
		t = n;
	}
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
            case dr_system_clock:
                dump_system_clock_descriptor(p_descriptor);
                break;
            case dr_maximum_bitrate:
                dump_maxbitrate_descriptor(p_descriptor);
                break;
            case dr_stream_identifier:
                //DumpStreamIdentifierDescriptor(dvbpsi_DecodeStreamIdentifierDr(p_descriptor));     
                break;
            case dr_subtitling:
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
