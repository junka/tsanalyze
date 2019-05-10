#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ts.h"
#include "table.h"
#include "utils.h"

mpeg_psi_t psi = {0};

static char const* get_stream_type(uint8_t type)
{
	const char *stream_type[] = {
		"Reserved",
		"ISO/IEC 11172 Video",
		"ISO/IEC 13818-2 Video",
		"ISO/IEC 11172 Audio",
		"ISO/IEC 13818-3 Audio",
		"ISO/IEC 13818-1 Private Section",
		"ISO/IEC 13818-1 Private PES data packets",
		"ISO/IEC 13522 MHEG",
		"ISO/IEC 13818-1 Annex A DSM CC",
		"ITU-T Rec. H.222.1",
		"ISO/IEC 13818-6 type A",
		"ISO/IEC 13818-6 type B",
		"ISO/IEC 13818-6 type C",
		"ISO/IEC 13818-6 type D",
		"ISO/IEC 13818-1 auxillary",
		"ISO/IEC 13818-7 Audio with ADTS transport syntax",
		"ISO/IEC 14496-2 Visual",
		"ISO/IEC 14496-3 Audio with the LATM transport syntax",
		"ISO/IEC 14496-1 SL-packetized stream or FlexMux stream carried in PES packets",
		"ISO/IEC 14496-1 SL-packetized stream or FlexMux stream carried in ISO/IEC14496_sections",
		"ISO/IEC 13818-6 Synchronized Download Protocol",
	};
	
	if (type < 0x15)
		return stream_type[type];
	else if (type < 0x80)
		return "ISO/IEC 13818-1 reserved";
	else
		return "User Private";
	
}

static void dump_PAT(void* p_data, pat_t* p_pat)
{
	if(p_pat == NULL ||p_data == NULL)
		return ;
	struct program_list* p_program = p_pat->list;
	mpeg_psi_t* p_stream = (mpeg_psi_t*) p_data;
	int num = p_stream->pmt_num;

	printf("\n");
	printf("PAT\n");
	printf("  section_length: %d\n",p_pat->section_length);
	printf("  transport_stream_id : %d\n", p_pat->transport_stream_id);
	printf("  version_number      : %d\n", p_pat->version_number);
	printf("    | program_number @ PMT_PID\n");
	while(p_program)
	{
		printf("    | %14d @ 0x%x (%d)\n",p_program->program_number, p_program->program_map_PID, p_program->program_map_PID);
		p_program = p_program->next;
	}
	printf("  active              : 0x%x\n", p_pat->current_next_indicator);
}

static void dump_CAT(void* p_data, cat_t* p_cat)
{
	CA_descriptor_t* p_descriptor = (CA_descriptor_t*) p_cat->list;
	mpeg_psi_t* p_stream = (mpeg_psi_t*) p_data;
	p_stream->cat.version_number=p_cat->version_number;
	printf("\n");
	printf("CAT\n");
	printf("  version number %d\n",p_cat->version_number);
	while(p_descriptor)
	{
		uint16_t system_id = p_descriptor->CA_system_ID;
		uint16_t emm_pid = p_descriptor->CA_PID;
		//p_stream->ca_num++;
		printf("  cat system id 0x%04x    emm pid 0x%04x\n",system_id,emm_pid);
		p_descriptor = p_descriptor->next;
	}
}

static void dump_TDT(void* p_data, tdt_t* p_tdt)
{
	printf("\n");
	printf("TDT: Time and Date Table\n");
	printf("\tUTC time       : %s\n", convert_UTC(&p_tdt->utc_time));

}

static void dump_TOT(void* p_data, tot_t* p_tot)
{
	printf("\n");
	printf("TOT: Time Offset Table\n");
	printf("\tUTC time       : %s\n", convert_UTC(&p_tot->utc_time));

	dump_descriptors("\t  |  ]", p_tot->time_offset_descriptor_list);
}

static void dump_PMT(void* p_data, pmt_t* p_pmt, uint16_t pid)
{
	struct es_info* p_es = p_pmt->es_list;
	descriptor_t* des;
	mpeg_psi_t* p_stream = (mpeg_psi_t*) p_data;
	
	printf("\n" );
	printf("active PMT\n" );
	printf("  program_number : %d  => pmt pid 0x%x\n", p_pmt->program_number,pid );
	printf("  version_number : %d\n", p_pmt->version_number );
	printf("  PCR_PID        : 0x%x (%d)\n", p_pmt->PCR_PID, p_pmt->PCR_PID);
	dump_descriptors("    ]", p_pmt->desriptor_list);
	printf("  components\n");
	printf("    | type @ elementary_PID\n");
	while(p_es){
		printf("    | 0x%02x (%s) @ 0x%x\n",p_es->stream_type,get_stream_type(p_es->stream_type),p_es->elementary_PID);
		des = p_es->descriptor_list;
		dump_descriptors("    |  ]", des);
		p_es = p_es->next;
	}
}

static void dump_SDT(void* p_data, sdt_t* p_sdt)
{
	if(p_sdt == NULL ||p_data == NULL)
		return ;
	struct service_info* p_service = p_sdt->service_list;
	mpeg_psi_t* p_stream = (mpeg_psi_t*) p_data;

	printf("\n");
	printf("SDT\n");	
	printf("  section_length: %d\n",p_sdt->section_length);
	printf("  transport_stream_id : 0x%x\n", p_sdt->transport_stream_id);  
	printf("  version_number      : %d\n", p_sdt->version_number);  
	printf("  Current next   : %s\n", p_sdt->current_next_indicator ? "yes" : "no");
	printf("  original_network_id : 0x%x\n", p_sdt->original_network_id);  
	printf("    | service_id \n");
	while(p_service)
	{
		printf("    | 0x%04x(%d) \n",p_service->service_id,p_service->service_id);
		printf("        | EIT_schedule_flag 0x%x \n",p_service->EIT_schedule_flag);
		printf("        | EIT_present_following_flag 0x%x \n",p_service->EIT_present_following_flag);
		printf("        | running_status 0x%x \n",p_service->running_status);
		printf("        | free_CA_mode 0x%x \n",p_service->free_CA_mode);
		p_service = p_service->next;
	}

}
static void dump_NIT(void* p_data, nit_t* p_nit)
{
	if(p_nit == NULL ||p_data == NULL)
		return ;
	struct transport_stream_info* p_service = p_nit->stream_list;
	mpeg_psi_t* p_stream = (mpeg_psi_t*) p_data;

	printf("\n");
	printf("NIT\n");	
	printf("  section_length: %d\n",p_nit->section_length);
	printf("  network_id : 0x%x\n", p_nit->network_id);
	printf("  version_number      : %d\n", p_nit->version_number);
	printf("  Current next   : %s\n", p_nit->current_next_indicator ? "yes" : "no");
	dump_descriptors("  [", p_nit->network_desriptor_list);
	printf("    | transport_stream \n");
	while(p_service)
	{
		printf("        | transport_stream_id 0x%x \n",p_service->transport_stream_id);
		printf("            | original_network_id 0x%x \n",p_service->original_network_id);
		dump_descriptors("            | ]", p_service->transport_stream_desriptor_list);
		p_service = p_service->next;
	}
}

void dump_tables(void)
{
	int i =0;
	if(psi.stats.sdt_sections>0)
		psi.has_sdt = 1;
	if(psi.stats.bat_sections>0)
		psi.has_bat = 1;
	if(psi.stats.nit_sections>0)
		psi.has_nit = 1;
	if(psi.stats.tdt_sections>0)
		psi.has_tdt = 1;
	if(psi.stats.tot_sections>0)
		psi.has_tot = 1;
	
	dump_PAT(&psi, &psi.pat);
	if(psi.has_sdt)
		dump_SDT(&psi,&psi.sdt);
	if(psi.ca_num>0)
	{
		dump_CAT(&psi, &psi.cat);
	}
	//pid 
	for(i = 0x10; i < 0x2000 ; i++){
		if (psi.pmt_bitmap[i/64] & ((uint64_t)1<<(i%64)))
		{
			dump_PMT(&psi, &psi.pmt[i],i);
		}
	}
	if(psi.has_nit)
		dump_NIT(&psi, &psi.nit);
	if(psi.has_tdt)
		dump_TDT(&psi, &psi.tdt);
	if(psi.has_tot)
		dump_TOT(&psi, &psi.tot);
}


extern void unregister_pmt_ops(uint16_t pid);

extern void register_pmt_ops(uint16_t pid);

int parse_pat(uint8_t * pbuf, uint16_t buf_size, pat_t * pPAT)
{
	uint16_t section_len = 0;
	uint8_t version_num;
	uint16_t program_num,program_map_PID;
	uint8_t *pdata = pbuf;
	struct program_list *p;

	if (pbuf == NULL || pPAT == NULL)
	{
		return -1;
	}

	if (unlikely(pdata[0] != PAT_TID))
	{
		return -1;
	}
	
	section_len = ((pdata[1] << 8) | pdata[2])  & 0x0FFF;
	if(section_len > 0x3FD) //For pat , maximum
	{
		return -1;
	}

	if (!(pdata[5] & 0x01)) //current_next_indicator
	{
		return -1;
	}
	version_num = (pdata[5] >> 1) & 0x1F;
	if(version_num == pPAT->version_number && pPAT->list !=NULL)
	{
		return -1;
	}
	p = pPAT->list;
	while(p)
	{
		unregister_pmt_ops(p->program_map_PID);
		p = p->next;
	}
	list_remove(pPAT,list,struct program_list);
	//hexdump(pdata, section_len+4);

	pPAT->section_length = section_len;

	//Transport Stream ID
	pdata += 3;
	pPAT->transport_stream_id = TS_READ16(pdata);
	pdata += 2;
	pPAT->version_number = version_num;
	pPAT->current_next_indicator = TS_READ8(pdata) & 0x01;
	pdata += 1;
	pPAT->section_number = TS_READ8(pdata);
	pdata += 1;
	pPAT->last_section_number = TS_READ8(pdata);
	pdata += 1;
	section_len -= (5 + 4); // exclude crc 4bytes
	
	//TODO: limit program total length
	

	while (section_len > 0)
	{
		struct program_list * pl, *next;
		program_num= TS_READ16(pdata);
		pdata+=2;
		program_map_PID =TS_READ16(pdata) & 0x1FFF;
		pdata+=2;
		section_len -= 4;
		if(program_num == 0xFFFF)
		{
			break;
		}
		if(pPAT->program_bitmap[program_num/64] & ((uint64_t)1<<(program_num%64)) )
		{
			//list_modify(pPAT,program_num,program_map_PID);
			list_modify(pPAT,list, struct program_list, program_number, program_num, program_map_PID, program_map_PID);
		}
		else
		{
			//printf("teset1 program_num %x\n",program_num);
			register_pmt_ops(program_map_PID);
			pl = malloc(sizeof(struct program_list));
			pl ->program_number = program_num;
			pl->prev = NULL;
			pl->next = NULL;
			pl->program_map_PID = program_map_PID;
			pPAT->program_bitmap[program_num/64] |= ((uint64_t)1<<(program_num%64));
			list_insert(pPAT,list,struct program_list, program_number, pl);
		}
	}

	return 0;
}


int parse_cat(uint8_t * pbuf, uint16_t buf_size, cat_t * pCAT)
{
	uint16_t section_len = 0;
	uint8_t *pdata = pbuf;

	if (pbuf == NULL || pCAT == NULL)
	{
		return -1;
	}

	if (unlikely(pdata[0] != CAT_TID))
	{
		return -1;
	}

	section_len = ((pdata[1] << 8) | pdata[2])  & 0x0FFF;
	if(section_len > 0x3FD) //For cat , maximum
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

	pCAT->list = parse_descriptors(pdata, section_len);

	return 0;
}



int parse_pmt(uint8_t * pbuf, uint16_t buf_size, pmt_t * pPMT)
{
	int16_t section_len = 0;
	uint8_t version_num;
	uint8_t *pdata = pbuf;

	if (pbuf == NULL || pPMT == NULL)
	{
		return -1;
	}

	if (unlikely(pdata[0] != PMT_TID))
	{
		return -1;
	}

	section_len = (int16_t)(((int16_t)pdata[1] << 8) | pdata[2])  & 0x0FFF;
	if(section_len > 0x3FD) //For pmt , maximum
	{
		return -1;
	}
	version_num = (pdata[5] >> 1) & 0x1F;
	if(version_num == pPMT->version_number && pPMT->es_list!=NULL)
	{
		return -1;
	}

	list_remove(pPMT,es_list,struct es_info);
	
	pPMT->section_length = section_len;

	//Transport Stream ID
	pPMT->program_number = (pdata[3] << 8) | pdata[4];

	pPMT->version_number = version_num;

	if ((pdata[5] & 0x01)==0) //current_next_indicator
	{
		return -1;
	}

	pPMT->section_number = pdata[6];
	pPMT->last_section_number = pdata[7];
	
	section_len -= 5 + 4; 
	pdata += 8;
	
	pPMT->PCR_PID = TS_READ16(pdata) & 0x1FFF;
	section_len -= 2; 
	pdata += 2;
	pPMT->program_info_length = TS_READ16(pdata)&0x0FFF;

	pPMT->es_list = NULL;
	pdata += 2;

	if(pPMT->desriptor_list)
		free_descriptors(pPMT->desriptor_list);

	pPMT->desriptor_list = parse_descriptors(pdata, pPMT->program_info_length);
	section_len -= 2+ pPMT->program_info_length;
	pdata += pPMT->program_info_length;
	
	//printf("section_len %d\n",section_len);

	while (section_len > 0)
	{
		struct es_info *el = malloc(sizeof(struct es_info));
		el->stream_type = TS_READ8(pdata);
		pdata+= 1;
		el->elementary_PID = TS_READ16(pdata)&0x1FFF;
		pdata+= 2;
		el->ES_info_length = TS_READ16(pdata)&0x0FFF;
		el->next = NULL;
		el->prev = NULL;
		pdata += 2;
		el->descriptor_list = parse_descriptors(pdata,(int) el->ES_info_length);
		pdata += el->ES_info_length;
		section_len -= (5+el->ES_info_length);
		list_insert(pPMT,es_list,struct es_info, elementary_PID, el);
		//printf("insert 0x%x\n",el->elementary_PID);
	}

	return 0;
}

int parse_nit(uint8_t * pbuf, uint16_t buf_size, nit_t * pNIT)
{
	int16_t section_len = 0;
	uint8_t version_num;
	uint8_t *pdata = pbuf;

	if (pbuf == NULL || pNIT == NULL)
	{
		return -1;
	}

	if (unlikely(pdata[0] != NIT_OTHER_TID && pdata[0] !=NIT_ACTUAL_TID))
	{
		return -1;
	}

	section_len = (int16_t)((pdata[1] << 8) | pdata[2])  & 0x0FFF;
	if(section_len > 0x3FD) //For nit , maximum
	{
		return -1;
	}
	version_num = (pdata[5] >> 1) & 0x1F;
	if(version_num == pNIT->version_number && pNIT->stream_list != NULL)
	{
		return -1;
	}
	pNIT->section_length = section_len;
	pNIT->version_number = version_num;
	pNIT->network_id = (pdata[3] << 8) | pdata[4];
	pdata += 8;
	pNIT->network_descriptors_length = TS_READ16(pdata) &0xFFF;
	if(pNIT->network_desriptor_list)
		free_descriptors(pNIT->network_desriptor_list);
	list_remove(pNIT, stream_list , struct transport_stream_info );
	pdata += 2 +pNIT->network_descriptors_length;
	pNIT->transport_stream_loop_length = TS_READ16(pdata) &0xFFF;
	section_len -= 10;
	section_len -= pNIT->network_descriptors_length;
	section_len -=4;
	pdata += 2 ;
	while(section_len>0) {
		struct transport_stream_info * more = malloc(sizeof(struct transport_stream_info));
		more->next = NULL;
		more->prev = more;
		more->transport_stream_id = TS_READ16(pdata);
		pdata+=2;
		more->original_network_id = TS_READ16(pdata);
		pdata+=2;
		more->transport_descriptors_length =TS_READ16(pdata);
		pdata+=2;
		more->transport_stream_desriptor_list = parse_descriptors(pdata, (int)more->transport_descriptors_length);
		pdata+= more->transport_descriptors_length;
		section_len -= 6+more->transport_descriptors_length;
		list_insert(pNIT,stream_list,struct transport_stream_info, transport_stream_id, more);
	}
	return 0;
}

int parse_bat(uint8_t * pbuf, uint16_t buf_size, bat_t * pBAT)
{
	int16_t section_len = 0;
	uint8_t version_num;
	uint8_t *pdata = pbuf;

	if (pbuf == NULL || pBAT == NULL)
	{
		return -1;
	}

	if (unlikely(pdata[0] != BAT_TID))
	{
		return -1;
	}

	section_len = (int16_t)((pdata[1] << 8) | pdata[2])  & 0x0FFF;
	if(section_len > 0x3FD) //For bat , maximum
	{
		return -1;
	}
	version_num = (pdata[5] >> 1) & 0x1F;
	if(version_num == pBAT->version_number && pBAT->stream_list != NULL )
	{
		return -1;
	}
	list_remove(pBAT, stream_list, struct transport_stream_info);
	
	pdata += 3;
	pBAT->bouquet_id = TS_READ16(pdata);
	pdata += 2;
	pBAT->section_length = section_len;
	pBAT->version_number = version_num;
	pdata += 3;
	pBAT->bouquet_descriptors_length = TS_READ16(pdata)&0xFFF;
	if(pBAT->bouquet_desriptor_list!=NULL)
		free_descriptors(pBAT->bouquet_desriptor_list);
	pBAT->bouquet_desriptor_list = parse_descriptors(pdata,pBAT->bouquet_descriptors_length);
	pdata += pBAT->bouquet_descriptors_length;
	section_len -= 7;
	section_len -= pBAT->bouquet_descriptors_length;
	pBAT->transport_stream_loop_length = TS_READ16(pdata)&0xFFF;
	pdata += 2;
	section_len -=2;
	section_len -=4;
	while(section_len>0)
	{
		struct transport_stream_info* si = malloc(sizeof(struct transport_stream_info));
		si->transport_stream_id = TS_READ16(pdata);
		pdata+= 2;
		si->original_network_id = TS_READ16(pdata);
		pdata+= 2;
		si->transport_descriptors_length = TS_READ16(pdata)&0xFFF;
		si->next = NULL;
		si->prev = si;
		pdata += 2;
		si->transport_stream_desriptor_list = parse_descriptors(pdata,si->transport_descriptors_length);
		pdata += si->transport_descriptors_length;
		section_len -= (5+si->transport_descriptors_length);
		list_insert(pBAT,stream_list,struct transport_stream_info, transport_stream_id, si);
	}
	
	return 0;
}

int parse_sdt(uint8_t * pbuf, uint16_t buf_size, sdt_t * pSDT)
{
	int16_t section_len = 0,loop_len;
	uint8_t version_num;
	uint8_t *pdata = pbuf;
	uint8_t last_sec,cur_sec;
	uint16_t ts_id;
	if (pbuf == NULL || pSDT == NULL)
	{
		return -1;
	}
	if (unlikely(pdata[0] != SDT_ACTUAL_TID && pdata[0]!=SDT_OTHER_TID))
	{
		return -1;
	}

	section_len = (((int16_t)pdata[1] << 8) | pdata[2])  & 0x0FFF;
	if(section_len > 0x3FD) //For sdt , maximum
	{
		return -1;
	}
	version_num = (pdata[5] >> 1) & 0x1F;	
	pdata += 3;
	ts_id = TS_READ16(pdata);
	pdata += 2;
	pdata += 1;
	cur_sec = TS_READ8(pdata);
	pdata += 1;
	last_sec = TS_READ8(pdata);
	pdata += 1;
	if(version_num != pSDT->version_number || last_sec != pSDT->last_section_number
		||pSDT->service_list == NULL)
	{
		pSDT->transport_stream_id = ts_id;
		pSDT->section_length = section_len;
		pSDT->version_number = version_num;
		pSDT->original_network_id = TS_READ16(pdata);
		pSDT->last_section_number = last_sec;
		list_remove(pSDT, service_list, struct service_info);
	}

	if((pSDT->section_bitmap[cur_sec/64]&((uint64_t)1<<(cur_sec%64))))
	{
		return -1;
	}
	//hexdump(pbuf, buf_size);
	pSDT->section_bitmap[cur_sec/64] |= ((uint64_t)1<<(cur_sec%64));
	pSDT->section_number = cur_sec;
	pdata += 3;
	section_len -= 8;
	section_len -= 4; //crc
	if(buf_size-11-4 < section_len)
	{
		loop_len = buf_size-11-4;
	}
	else
	{
		loop_len = section_len;
	}
	
	while(loop_len>0)
	{
		//printf("ptr %x section len %d\n",pdata,section_len);
		struct service_info* si = malloc(sizeof(struct service_info));
		si->service_id = TS_READ16(pdata);
		pdata+= 2;
		si->EIT_schedule_flag = (TS_READ8(pdata)>>1)&0x1;
		si->EIT_present_following_flag = (TS_READ8(pdata))&0x1;
		pdata+= 1;
		si->running_status = (TS_READ16(pdata)>>13)&0x7;
		si->free_CA_mode = (TS_READ16(pdata)>>12)&0x1;
		si->descriptors_loop_length = TS_READ16(pdata)&0x0FFF;
		si->next = NULL;
		si->prev = NULL;
		pdata += 2;
		si->service_desriptor_list = parse_descriptors(pdata,(int)(si->descriptors_loop_length));
		pdata += si->descriptors_loop_length;
		//printf("desc len %d\n",si->descriptors_loop_length);
		loop_len -=5;
		loop_len -= (si->descriptors_loop_length);
		list_insert(pSDT,service_list,struct service_info, service_id, si);
	}
	
	return 0;
}

static int parse_eit(uint8_t * pbuf, uint16_t buf_size, eit_t * pEIT)
{
	uint16_t section_len = 0;
	uint8_t *pdata = pbuf;
	
	if (pbuf == NULL || pEIT == NULL)
	{
		return -1;
	}

	pdata += 1;
	section_len = TS_READ16(pdata)&0xFFF;

	if(section_len > 0xFFD) //For eit , maximum
	{
		return -1;
	}
	pEIT->section_length = section_len;

	return 0;
}

static int parse_tdt(uint8_t * pbuf, uint16_t buf_size, tdt_t * pTDT)
{
	uint16_t section_len = 0;
	uint8_t *pdata = pbuf;
	
	if (pbuf == NULL || pTDT == NULL)
	{
		return -1;
	}

	if (unlikely(pdata[0] != TDT_TID))
	{
		return -1;
	}

	pdata += 1;
	section_len = TS_READ16(pdata)&0xFFF;
	pTDT->section_length = section_len;
	pdata+=2;
	memcpy(&pTDT->utc_time,pdata, 5);
	return 0;
}

static int parse_tot(uint8_t * pbuf, uint16_t buf_size, tot_t * pTOT)
{
	uint16_t section_len = 0;
	uint8_t *pdata = pbuf;
	
	if (pbuf == NULL || pTOT == NULL)
	{
		return -1;
	}

	if (unlikely(pdata[0] != TOT_TID))
	{
		return -1;
	}

	pdata += 1;
	pTOT->section_length = TS_READ16(pdata)&0xFFF;
	pdata+=2;
	memcpy(&pTOT->utc_time,pdata, 5);
	pdata+= 5;
	pTOT->descriptors_loop_length = TS_READ16(pdata)&0xFFF;
	pdata+=2;
	if(pTOT->time_offset_descriptor_list)
		free_descriptors(pTOT->time_offset_descriptor_list);
	pTOT->time_offset_descriptor_list = parse_descriptors(pdata, (int)pTOT->descriptors_loop_length );
	return 0;
}

static int pat_proc(uint16_t pid,uint8_t *pkt,uint16_t len)
{
	psi.stats.pat_sections ++;
	parse_pat(pkt,len,&psi.pat);
	return 0;
}

static int cat_proc(uint16_t pid,uint8_t *pkt,uint16_t len)
{
	psi.stats.cat_sections ++;
	parse_cat(pkt,len,&psi.cat);
	return 0;
}

static int pmt_proc(uint16_t pid,uint8_t *pkt,uint16_t len)
{
	psi.pmt_bitmap[pid/64] |= ((uint64_t)1<<(pid%64));
	parse_pmt(pkt,len,&(psi.pmt[pid]));
	return 0;
}

static int nit_proc(uint16_t pid, uint8_t *pkt, uint16_t len)
{
	psi.stats.nit_sections ++;
	parse_nit(pkt, len, &(psi.nit));
	return 0;
}

static int sdt_bat_proc(uint16_t pid,uint8_t *pkt,uint16_t len)
{
	if(pkt[0]==BAT_TID)
	{
		psi.stats.bat_sections ++;
		parse_bat(pkt,len,&(psi.bat));
	}
	else if(pkt[0] == SDT_ACTUAL_TID)
	{
		psi.stats.sdt_sections ++;
		parse_sdt(pkt,len,&(psi.sdt));
	}
	return 0;
}
static int eit_proc(uint16_t pid,uint8_t *pkt,uint16_t len)
{
	psi.stats.eit_sections ++;
	parse_eit(pkt, len, &psi.eit);
	return 0;
}

static int tdt_tot_proc(uint16_t pid,uint8_t *pkt,uint16_t len)
{
	if(TDT_TID == pkt[0])
	{
		psi.stats.tdt_sections ++;
		parse_tdt(pkt, len, &psi.tdt);
	}
	else if(TOT_TID == pkt[0])
	{
		psi.stats.tot_sections ++;
		parse_tot(pkt, len, &psi.tot);
	}
	return 0;
}

static int drop_proc(uint16_t pid,uint8_t *pkt,uint16_t len)
{
	return 0;
}

table_ops drop_ops ={
	.table_id = RESERVED_TID,
	.mask = 0x00,
	.table_proc = drop_proc,
};

table_ops pat_ops = {
	.table_id = PAT_TID,
	.mask = 0xFF,
	.table_proc = pat_proc,
};

table_ops cat_ops = {
	.table_id = CAT_TID,
	.mask = 0xFF,
	.table_proc = cat_proc,
};

table_ops pmt_ops = {
	.table_id = PMT_TID,
	.mask = 0xFF,
	.table_proc = pmt_proc,
};

table_ops nit_ops = {
	.table_id = NIT_ACTUAL_TID,
	.mask = 0xFF,
	.table_proc = nit_proc,
};

table_ops sdt_bat_ops = {
	.table_id = BAT_TID,
	.mask = 0xFF,
	.table_proc = sdt_bat_proc,
};

table_ops eit_ops = {
	.table_id = EIT_ACTUAL_TID,
	.mask = 0xFF,
	.table_proc = eit_proc,
};

table_ops tdt_tot_ops = {
	.table_id = TDT_TID,
	.mask = 0xFE,
	.table_proc = tdt_tot_proc,
};


