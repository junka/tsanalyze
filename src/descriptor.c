#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "ts.h"
#include "descriptor.h"

#define INVALID_DR_RETURN(a,buf) \
	do { if(buf[0]!=dr_##a){ return -1; } }while(0)

#define DR_TAG(a,buf,ptr) a##_descriptor_t *dr = (a##_descriptor_t*)ptr; \
	dr->descriptor_tag = dr_##a;\
	dr->descriptor_length = buf[1]; \
	dr->next = NULL;

#define DR_MEMBER(dr,a,member)  dr ->member

int parse_video_stream_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(video_stream,buf);
	DR_TAG(video_stream,buf,ptr)
	DR_MEMBER(dr,video_stream,multiple_frame_rate_flag) = (buf[2]>>7)&0x01;
	DR_MEMBER(dr,video_stream,frame_rate_code) = (buf[2] >>3)&0x0F;
	DR_MEMBER(dr,video_stream,MPEG_1_only_flag) = (buf[2]>>2)&0x01;
	DR_MEMBER(dr,video_stream,constrained_parameter_flag) = (buf[2]>>1)&0x01;
	DR_MEMBER(dr,video_stream,still_picture_flag) = buf[2]&0x01;
	DR_MEMBER(dr,video_stream,profile_and_level_indication) = buf[3] ;
	DR_MEMBER(dr,video_stream,chroma_format) = buf[4]>>6;
	DR_MEMBER(dr,video_stream,frame_rate_extension_flag) = (buf[4]>>5)&0x01;
	return 0;
}

int parse_audio_stream_descriptor(uint8_t *buf, uint32_t len,void *ptr )
{
	INVALID_DR_RETURN(audio_stream,buf);
	DR_TAG(audio_stream,buf,ptr)
	DR_MEMBER(dr,audio_stream,free_format_flag) = buf[2]>>7;
	DR_MEMBER(dr,audio_stream,ID)= (buf[2]>>6)&0x01;
	DR_MEMBER(dr,audio_stream,layer) = (buf[2]>>4)&0x03;
	DR_MEMBER(dr,audio_stream,variable_rate_audio_indicator) = (buf[2]>>3)&0x01;
	return 0;
}

int parse_hierarchy_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(hierarchy,buf);
	DR_TAG(hierarchy,buf,ptr)
	DR_MEMBER(dr,hierarchy,hierarchy_type) = buf[2] &0x0F;
	DR_MEMBER(dr,hierarchy,hierarchy_layer_index) = (buf[3])&0x03F;
	DR_MEMBER(dr,hierarchy,hierarchy_embedded_layer_index) = (buf[4])&0x3F;
	DR_MEMBER(dr,hierarchy,hierarchy_channel) = (buf[5])&0x3F;
	return 0;
}

int parse_registration_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(registration,buf);
	uint8_t *p = buf;
	DR_TAG(registration,buf,ptr)
	p += 2;
	dr->format_identifier = TS_READ32(p);
	dr->additional_identification_info = malloc(buf[1]-4);
	memcpy(dr->additional_identification_info,buf+6, buf[1]-4);
	return 0;
}

int parse_data_stream_alignment_descriptor(uint8_t *buf, uint32_t len,  void* ptr)
{
	INVALID_DR_RETURN(data_stream_alignment,buf);
	DR_TAG(data_stream_alignment,buf,ptr)
	dr->alignment_type = buf[2];
	return 0;
}

int parse_target_background_grid_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(target_background_grid,buf);
	DR_TAG(target_background_grid,buf,ptr)
	uint8_t *p = buf;
	dr->horizontal_size = (buf[2]<<6|buf[3]>>2);//TS_READ32(buf+2) >>18;
	p += 2;
	dr->vertical_size = (TS_READ32(p)>>4) &0x3FFF;
	dr->aspect_ratio_information = buf[5]&0x0F;
	return 0;
}

int parse_video_window_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(video_window,buf);
	uint8_t *p = buf;
	DR_TAG(video_window,buf,ptr)
	dr->horizontal_offset = (buf[2]<<6|buf[3]>>2);
	p += 2;
	dr->vertical_offset = (TS_READ32(p)>>4) &0x3FFF;
	dr->window_priority = buf[5]&0x0F;
	return 0;
}

int parse_CA_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(CA,buf);
	DR_TAG(CA,buf,ptr)
	dr->CA_system_ID = buf[2]<<8 |buf[3];
	dr->CA_PID = (buf[5]<<8 |buf[6]) & 0x1FFF ;
	return 0;
}
int parse_ISO_639_language_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(ISO_639_language,buf);
	DR_TAG(ISO_639_language,buf,ptr)
	dr->language_list = NULL;
	//lang->lang_num = 0;
	return 0;
}

static int parse_system_clock_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(system_clock,buf);
	DR_TAG(system_clock,buf,ptr)
	dr->external_clock_reference_indicator = buf[2]>>7;
	dr->clock_accuracy_integer = buf[2]&0x3F;
	dr->clock_accuracy_exponent = buf[3]>>5;
	return 0;
}

int parse_multiplex_buffer_utilization_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(multiplex_buffer_utilization,buf);
	DR_TAG(multiplex_buffer_utilization,buf,ptr)
	return 0;
}

int parse_copyright_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(copyright,buf);
	DR_TAG(copyright,buf,ptr)
	return 0;
}

int parse_maximum_bitrate_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(maximum_bitrate,buf);
	DR_TAG(maximum_bitrate,buf,ptr)
	dr->maximum_bitrate.bits = ((buf[2]<<16 |buf[3]<<8 |buf[4])&0x3FFFFF);
	return 0;
}

int parse_private_data_indicator_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(private_data_indicator,buf);
	DR_TAG(private_data_indicator,buf,ptr)
	return 0;
}

int parse_smoothing_buffer_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(smoothing_buffer,buf);
	DR_TAG(smoothing_buffer,buf,ptr)
	return 0;
}
int parse_STD_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(STD,buf);
	DR_TAG(STD,buf,ptr)
	return 0;
}
int parse_ibp_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(ibp,buf);
	DR_TAG(ibp,buf,ptr)
	return 0;
}
int parse_MPEG4_video_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(MPEG4_video,buf);
	DR_TAG(MPEG4_video,buf,ptr)
	return 0;
}
int parse_MPEG4_audio_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(MPEG4_audio,buf);
	DR_TAG(MPEG4_audio,buf,ptr)
	return 0;
}
int parse_IOD_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(IOD,buf);
	DR_TAG(IOD,buf,ptr)
	return 0;
}

int parse_SL_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(SL,buf);
	DR_TAG(SL,buf,ptr)
	return 0;
}
int parse_FMC_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(FMC,buf);
	DR_TAG(FMC,buf,ptr)
	return 0;
}
int parse_external_ES_ID_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(external_ES_ID,buf);
	DR_TAG(external_ES_ID,buf,ptr)

	return 0;
}
int parse_muxcode_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(muxcode,buf);
	DR_TAG(muxcode,buf,ptr)

	return 0;
}
int parse_FmxBufferSize_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(FmxBufferSize,buf);
	DR_TAG(FmxBufferSize,buf,ptr)

	return 0;
}
int parse_MultiplexBuffer_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(MultiplexBuffer,buf);
	DR_TAG(MultiplexBuffer,buf,ptr)

	return 0;
}
int parse_network_name_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(network_name,buf);
	DR_TAG(network_name,buf,ptr)

	return 0;
}int parse_service_list_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(service_list,buf);
	DR_TAG(service_list,buf,ptr)

	return 0;
}
int parse_stuffing_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(stuffing,buf);
	DR_TAG(stuffing,buf,ptr)

	return 0;
}
int parse_satellite_delivery_system_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(satellite_delivery_system,buf);
	DR_TAG(satellite_delivery_system,buf,ptr)
	return 0;
}
int parse_cable_delivery_system_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(cable_delivery_system,buf);
	DR_TAG(cable_delivery_system,buf,ptr)
	return 0;
}
int parse_VBI_data_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(VBI_data,buf);
	DR_TAG(VBI_data,buf,ptr)
	return 0;
}
int parse_VBI_teletext_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(VBI_teletext,buf);
	DR_TAG(VBI_teletext,buf,ptr)

	return 0;
}
int parse_bouquet_name_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(bouquet_name,buf);
	DR_TAG(bouquet_name,buf,ptr)
	return 0;
}
int parse_service_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(service,buf);
	DR_TAG(service,buf,ptr)
	return 0;
}
int parse_country_availability_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(country_availability,buf);
	DR_TAG(country_availability,buf,ptr)
	return 0;
}
int parse_linkage_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(linkage,buf);
	DR_TAG(linkage,buf,ptr)
	return 0;
}
int parse_NVOD_reference_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(NVOD_reference,buf);
	DR_TAG(NVOD_reference,buf,ptr)
	return 0;
}
int parse_time_shifted_service_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(time_shifted_service,buf);
	DR_TAG(time_shifted_service,buf,ptr)
	return 0;
}
int parse_short_event_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(short_event,buf);
	DR_TAG(short_event,buf,ptr)
	return 0;
}

int parse_extended_event_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(extended_event,buf);
	DR_TAG(extended_event,buf,ptr)
	return 0;
}

int parse_time_shifted_event_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(time_shifted_event,buf);
	DR_TAG(time_shifted_event,buf,ptr)
	return 0;
}

int parse_component_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(component,buf);
	DR_TAG(component,buf,ptr)
	return 0;
}

int parse_mosaic_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(mosaic,buf);
	DR_TAG(mosaic,buf,ptr)
	return 0;
}

int parse_stream_identifier_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(stream_identifier,buf);
	DR_TAG(stream_identifier,buf,ptr)
	return 0;
}

int parse_CA_identifier_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(CA_identifier,buf);
	DR_TAG(CA_identifier,buf,ptr)
	dr->CA_system_id = malloc(dr->descriptor_length);
	memcpy(dr->CA_system_id,buf+2,dr->descriptor_length);
	return 0;
}

int parse_content_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(content,buf);
	DR_TAG(content,buf,ptr)
	return 0;
}

int parse_parental_rating_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(parental_rating,buf);
	DR_TAG(parental_rating,buf,ptr)
	return 0;
}

int parse_teletext_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(teletext,buf);
	DR_TAG(teletext,buf,ptr)
	return 0;
}

int parse_telephone_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(telephone,buf);
	DR_TAG(telephone,buf,ptr)
	return 0;
}

int parse_local_time_offset_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(local_time_offset,buf);
	DR_TAG(local_time_offset,buf,ptr)
	return 0;
}

int parse_subtitling_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(subtitling,buf);
	DR_TAG(subtitling,buf,ptr)
	return 0;
}

int parse_terrestrial_delivery_system_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(terrestrial_delivery_system,buf);
	DR_TAG(terrestrial_delivery_system,buf,ptr)
	return 0;
}

int parse_multilingual_network_name_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(multilingual_network_name,buf);
	DR_TAG(multilingual_network_name,buf,ptr)
	return 0;
}

int parse_multilingual_bouquet_name_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(multilingual_bouquet_name,buf);
	DR_TAG(multilingual_bouquet_name,buf,ptr)
	return 0;
}

int parse_multilingual_service_name_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(multilingual_service_name,buf);
	DR_TAG(multilingual_service_name,buf,ptr)
	return 0;
}

int parse_multilingual_component_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(multilingual_component,buf);
	DR_TAG(multilingual_component,buf,ptr)
	return 0;
}

int parse_private_data_specifier_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(private_data_specifier,buf);
	DR_TAG(private_data_specifier,buf,ptr)
	return 0;
}

int parse_service_move_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(service_move,buf);
	DR_TAG(service_move,buf,ptr)
	return 0;
}

int parse_short_smoothing_buffer_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(short_smoothing_buffer,buf);
	DR_TAG(short_smoothing_buffer,buf,ptr)
	return 0;
}

int parse_frequency_list_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(frequency_list,buf);
	DR_TAG(frequency_list,buf,ptr)
	return 0;
}

int parse_partial_transport_stream_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(partial_transport_stream,buf);
	DR_TAG(partial_transport_stream,buf,ptr)
	return 0;
}

int parse_data_broadcast_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(data_broadcast,buf);
	DR_TAG(data_broadcast,buf,ptr)
	return 0;
}

int parse_scrambling_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(scrambling,buf);
	DR_TAG(scrambling,buf,ptr)
	return 0;
}

int parse_data_broadcast_id_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(data_broadcast_id,buf);
	DR_TAG(data_broadcast_id,buf,ptr)
	return 0;
}

int parse_transport_stream_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(transport_stream,buf);
	DR_TAG(transport_stream,buf,ptr)
	return 0;
}

int parse_DSNG_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(DSNG,buf);
	DR_TAG(DSNG,buf,ptr)
	return 0;
}

int parse_PDC_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(PDC,buf);
	DR_TAG(PDC,buf,ptr)
	return 0;
}

int parse_AC3_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(AC3,buf);
	DR_TAG(AC3,buf,ptr)
	return 0;
}

int parse_ancillary_data_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(ancillary_data,buf);
	DR_TAG(ancillary_data,buf,ptr)
	return 0;
}

int parse_cell_list_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(cell_list,buf);
	DR_TAG(cell_list,buf,ptr)
	return 0;
}

int parse_cell_frequency_link_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(cell_frequency_link,buf);
	DR_TAG(cell_frequency_link,buf,ptr)
	return 0;
}

int parse_announcement_support_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(announcement_support,buf);
	DR_TAG(announcement_support,buf,ptr)
	return 0;
}

int parse_application_signalling_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(application_signalling,buf);
	DR_TAG(application_signalling,buf,ptr)
	return 0;
}

int parse_adaptation_field_data_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(adaptation_field_data,buf);
	DR_TAG(adaptation_field_data,buf,ptr)
	return 0;
}

int parse_service_identifier_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(service_identifier,buf);
	DR_TAG(service_identifier,buf,ptr)
	return 0;
}

int parse_service_availability_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(service_availability,buf);
	DR_TAG(service_availability,buf,ptr)
	return 0;
}

int parse_default_authority_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(default_authority,buf);
	DR_TAG(default_authority,buf,ptr)
	return 0;
}

int parse_related_content_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(related_content,buf);
	DR_TAG(related_content,buf,ptr)
	return 0;
}

int parse_TVA_id_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(TVA_id,buf);
	DR_TAG(TVA_id,buf,ptr)
	return 0;
}

int parse_content_identifier_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(content_identifier,buf);
	DR_TAG(content_identifier,buf,ptr)
	return 0;
}

int parse_time_slice_fec_identifier_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(time_slice_fec_identifier,buf);
	DR_TAG(time_slice_fec_identifier,buf,ptr)
	return 0;
}
int parse_ECM_repetition_rate_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(ECM_repetition_rate,buf);
	DR_TAG(ECM_repetition_rate,buf,ptr)
	return 0;
}
int parse_S2_satellite_delivery_system_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(S2_satellite_delivery_system,buf);
	DR_TAG(S2_satellite_delivery_system,buf,ptr)
	return 0;
}
int parse_enhanced_AC3_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(enhanced_AC3,buf);
	DR_TAG(enhanced_AC3,buf,ptr)
	return 0;
}
int parse_DTS_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(DTS,buf);
	DR_TAG(DTS,buf,ptr)
	return 0;
}
int parse_AAC_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(AAC,buf);
	DR_TAG(AAC,buf,ptr)
	return 0;
}
int parse_XAIT_location_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(XAIT_location,buf);
	DR_TAG(XAIT_location,buf,ptr)
	return 0;
}
int parse_FTA_content_management_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(FTA_content_management,buf);
	DR_TAG(FTA_content_management,buf,ptr)
	return 0;
}

int parse_extension_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	INVALID_DR_RETURN(extension,buf);
	DR_TAG(extension,buf,ptr)
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

void dump_reserved(descriptor_t * p_descriptor)
{
	int i =0 ;
	printf( "\"");
	for(i = 0; i < p_descriptor->length; i++)
		printf(  "%c", p_descriptor->data[i]);
	printf( "\"\n");

}

#define FUNC(descriptor) parse_##descriptor##_descriptor

#define ALLOC(descriptor) void *alloc_##descriptor(){ return malloc(sizeof(descriptor##_descriptor_t));}

#define FREE(descriptor) void free_##descriptor(descriptor##_descriptor_t * ptr) \
	{ free(ptr); }

#define _(a,b) ALLOC(a)
	foreach_enum_descriptor
#undef _

#define _(a,b) void dump_##a##_descriptor(descriptor_t* p_descriptor) {\
	a##_descriptor_t *dr = container_of(p_descriptor, a##_descriptor_t,descriptor); \
	printf("\n"); \
	}
	foreach_enum_descriptor 
#undef _

static struct descriptor_ops des_ops[256];

void init_descriptor_parsers()
{
	uint16_t i=0;
	for(i=0;i<=0xFF;i++)
	{
		des_ops[i].tag = i;
		strncpy(des_ops[i].tag_name,"reserved",sizeof("reserved"));
		des_ops[i].descriptor_parse = parse_reserved_descriptor;
		des_ops[i].descriptor_alloc = alloc_reserved;
		des_ops[i].descriptor_dump = dump_reserved;
	}

#define _(a,b) des_ops[b].tag = b;\
	des_ops[b].descriptor_parse = parse_##a##_descriptor; \
	des_ops[b].descriptor_alloc = alloc_##a ;\
	des_ops[b].descriptor_dump = dump_##a##_descriptor;\
	snprintf(des_ops[b].tag_name,64, # a"_descriptor");
		foreach_enum_descriptor
#undef _

}

#define ALLOC_DES(tag)  des_ops[tag].descriptor_alloc()
#define PARSE_DES(ptr,des)  do{ des_ops[ptr[0]].descriptor_parse(ptr,ptr[1]+2,des);}while(0)

descriptor_t* parse_descriptors(uint8_t *buf, int len)
{
	//hexdump(buf,len);
	int l = len;
	uint8_t* ptr = buf;
	descriptor_t* h = NULL, *more;
	while(l>0)
	{
		//hexdump( ptr, l);
		//printf("%s : %d, %d\n",des_ops[ptr[0]].tag_name,l,ptr[1]);
		void* des = ALLOC_DES(ptr[0]);
		descriptor_t *more = (descriptor_t*) des;
		PARSE_DES(ptr,des);
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



#if 0
void dump_system_clock_descriptor(system_clock_descriptor_t* p_descriptor)
{
	fprintf(stdout,"external_clock_reference_indicator: %d",
		p_descriptor->external_clock_reference_indicator);
	fprintf(stdout,"clock_accuracy_integer: %d",
		p_descriptor->clock_accuracy_exponent);
	fprintf(stdout,"clock_accuracy_exponent: %d\n",
		p_descriptor->clock_accuracy_exponent);
}

void dump_maxbitrate_descriptor(maximum_bitrate_descriptor_t *p_descriptor)
{
	fprintf(stdout,"maximum_bitrate: %d\n", (int)(p_descriptor->maximum_bitrate.bits));
}

void dump_stream_identifier_descriptor(stream_identifier_descriptor_t *p_descriptor)
{
	fprintf(stdout,"component_tag: %d\n",p_descriptor->component_tag);
}
void dump_subtitling_descriptor(subtitling_descriptor_t *p_descriptor)
{
	if(p_descriptor->subtitle_list){
		fprintf(stdout,"subtitle_list:\n");
		fprintf(stdout,"	ISO_639_language_code %d\n",p_descriptor->subtitle_list->ISO_639_language_code);
	}
}
#endif

void dump_descriptors(const char* str, descriptor_t* p_descriptor)
{
	int i;
	while(p_descriptor)
	{
		printf( "%s 0x%02x (%s) : ", str, p_descriptor->tag,des_ops[p_descriptor->tag].tag_name);
		des_ops[p_descriptor->tag].descriptor_dump(p_descriptor);
		p_descriptor = p_descriptor->next;
	}
}

