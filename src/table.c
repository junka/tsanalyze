#include <stdio.h>
#include <stdint.h>
#include "ts.h"
#include "table.h"

mpeg_psi_t psi = {0};

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

static void dump_PAT(void* p_data, pat_t* p_pat)
{
	if(p_pat == NULL ||p_data == NULL)
		return ;
	struct program_list* p_program = p_pat->list;
	mpeg_psi_t* p_stream = (mpeg_psi_t*) p_data;
	int num = p_stream->pmt_num;

	//p_stream->pat->version_number = p_pat->version_number;
	//p_stream->pat->transport_stream_id = p_pat->transport_stream_id;

	printf(  "\n");
	printf(  "PAT\n");
	printf(  "  section_length: %d\n",p_pat->section_length);
	printf(  "  transport_stream_id : %d\n", p_pat->transport_stream_id);  
	printf(  "  version_number      : %d\n", p_pat->version_number);  
	printf(  "    | program_number @ PMT_PID\n");
	while(p_program)
	{
		printf("    | %14d @ 0x%x (%d)\n",p_program->program_number, p_program->program_map_PID, p_program->program_map_PID);
		p_program = p_program->next;
	}
	printf("  active              : %d\n", p_pat->current_next_indicator);

}

static void dump_CAT(void* p_data, cat_t* p_cat)
{
	CA_descriptor_t* p_descriptor = p_cat->list;
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
	//ts_stream_t* p_stream = (ts_stream_t*) p_data;

	printf("\n");
	printf("TDT: Time and Date Table\n");

	//printf("\tVersion number : %d\n", p_tot->version);
	//printf("\tCurrent next   : %s\n", p_tot->b_current_next ? "yes" : "no");
	printf("\tUTC time       : %lu\n", p_tdt->utc_time);

	//dump_descriptors("\t  |  ]", p_tot->time_offset_descriptor_list);
}

static void dump_TOT(void* p_data, tot_t* p_tot)
{
	//ts_stream_t* p_stream = (ts_stream_t*) p_data;

	printf("\n");
	printf("TOT: Time Offset Table\n");

	//printf("\tVersion number : %d\n", p_tot->version);
	//printf("\tCurrent next   : %s\n", p_tot->b_current_next ? "yes" : "no");
	printf("\tUTC time       : %lu\n", p_tot->utc_time);

	dump_descriptors("\t  |  ]", p_tot->time_offset_descriptor_list);
}

#if 0
static void handle_subtable(void *p_psi, uint8_t i_table_id, uint16_t i_extension,
					void *p_data)
{
	switch (i_table_id)
	{
		case 0x70: /* TDT */
		case 0x73: /* TOT only */
			break;
		default:
			break;
	}

}
#endif


static void dump_PMT(void* p_data, pmt_t* p_pmt, uint16_t pid)
{
	struct es_info* p_es = p_pmt->es_list;
	descriptor_t* des;
	mpeg_psi_t* p_stream = (mpeg_psi_t*) p_data;
	
	printf("\n" );
	printf( "active PMT\n" );
	printf( "  program_number : %d  => pmt pid 0x%x\n", p_pmt->program_number,pid );
	printf( "  version_number : %d\n", p_pmt->version_number );
	printf( "  PCR_PID        : 0x%x (%d)\n", p_pmt->PCR_PID, p_pmt->PCR_PID);
	dump_descriptors("    ]", p_pmt->desriptor_list);
	printf( "    | type @ elementary_PID\n");
	while(p_es){
		printf( "    | 0x%02x @ 0x%x\n",p_es->stream_type,p_es->elementary_PID);
		des = p_es->descriptor_list;
		while(des)
		{
			printf( "    | 0x%02x (%s) @ 0x%x (%d)\n", p_es->stream_type, get_stream_type(p_es->stream_type),p_es->elementary_PID, p_es->elementary_PID);
			dump_descriptors("    |  ]", des);
			des = des->next;
		}
		p_es = p_es->next;
	}
}

void dump_tables(void)
{
	int i =0;
	dump_PAT(&psi, &psi.pat);
	if(psi.ca_num>0)
	{
		dump_CAT(&psi, &psi.cat);
	}
	//pid 
	for(i = 0x10; i < 0x2000 ; i++){
		if (psi.pmt_bitmap[i/64] & (1<<(i%64)))
		{
			dump_PMT(&psi, &psi.pmt[i],i);
		}
	}
	dump_TDT(&psi, &psi.tdt);
	dump_TOT(&psi, &psi.tdt);
}

static inline void list_modify(pat_t *  pPAT, uint16_t program_number, uint16_t pid)
{
	struct program_list* pl = pPAT->list;
	while(pl->program_number!= program_number)
	{
		pl = pl->next ;
	}
	if(pl->program_map_PID != pid)
		pl->program_map_PID = pid;
}
extern void unregister_pmt_ops(uint16_t pid);

static inline void list_remove(pat_t *  pPAT)
{
	struct program_list *p = pPAT->list;
	struct program_list *n ;
	while(p)
	{
		n = p->next;
		unregister_pmt_ops(p->program_map_PID);
		free(p);
		p = n;
	}
	pPAT->list = NULL;
}

static inline void list_insert(pat_t *  pPAT, struct program_list *n)
{
	if(pPAT->list == NULL)
	{
		pPAT->list = n;
		return ;
	}
	struct program_list *h = pPAT->list;
	if(n->program_number < h->program_number)
	{
		n->next = h;
		h->prev = n;
		pPAT->list = n;
		return ;
		
	}
	while(h->next && h->program_number < n->program_number)
	{
		h = h->next;
	}
	if(h->program_number ==n->program_number)
	{
		return;
	}
	n->prev = h;
	n->next = h->next;
	h->next = n;
	return ;
}

extern void register_pmt_ops(uint16_t pid);

int parse_pat(uint8_t * pbuf, uint16_t buf_size, pat_t * pPAT)
{
	uint16_t section_len = 0;
	uint8_t version_num;
	uint16_t program_num,program_map_PID;
	uint8_t *pdata = pbuf;

	if (pbuf == NULL || pPAT == NULL)
	{
		return -1;
	}

	if (pdata[0] != PAT_TID)
	{
		return -1;
	}

	section_len = ((pdata[1] << 8) | pdata[2])  & 0x0FFF;
	if(section_len > 0x3FD)
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
	list_remove(pPAT);
	//hexdump(pdata, section_len+4);

	//if ((section_len + 3) != buf_size)
	//{
	//	return -1;
	//}
	pPAT->section_length = section_len;

	//Transport Stream ID
	pPAT->transport_stream_id = (pdata[3] << 8) | pdata[4];
	pPAT->version_number = version_num;
	pPAT->current_next_indicator = pdata[5] & 0x01;
	pPAT->section_number = pdata[6];
	pPAT->last_section_number = pdata[7];

	section_len -= (5 + 4); // exclude crc 4bytes
	pdata += 8;
	
	//TODO: limit program total length
	

	while (section_len > 0)
	{
		struct program_list * pl, *next;
		program_num= (pdata[0] << 8) | pdata[1];
		program_map_PID = ((pdata[2] << 8) | pdata[3]) & 0x1FFF;
		if(program_num == 0xFFFF)
		{
			pdata += 4;
			section_len -= 4;
			break;
		}
		if(pPAT->program_bitmap[program_num/64] & (1<<(program_num%64)) )
		{
			//printf("teset\n");
			list_modify(pPAT,program_num,program_map_PID);
		}
		else
		{
			//printf("teset1 program_num %x\n",program_num);
			register_pmt_ops(program_map_PID);
			pl = malloc(sizeof(struct program_list));
			pl ->program_number = program_num;
			pl->prev = pl;
			pl->next = NULL;
			pl->program_map_PID = program_map_PID;
			pPAT->program_bitmap[program_num/64] |= (1<<(program_num%64));
			list_insert(pPAT,pl);
		}
		
		pdata += 4;
		section_len -= 4;
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
static inline void pmt_list_remove(pmt_t *  pPMT)
{
	struct es_info *p = pPMT->es_list;
	struct es_info *n ;
	while(p)
	{
		n = p->next;
		free(p);
		p = n;
	}
	pPMT->es_list = NULL;
}

static inline void pmt_list_insert(pmt_t *  pPMT, struct es_info *n)
{
	if(pPMT->es_list == NULL)
	{
		pPMT->es_list = n;
		return ;
	}
	struct es_info *h = pPMT->es_list;
	if(n->elementary_PID < h->elementary_PID)
	{
		n->next = h;
		h->prev = n;
		pPMT->es_list = n;
		return ;
		
	}
	while(h->next && h->elementary_PID < n->elementary_PID)
	{
		h = h->next;
	}
	
	if(h->elementary_PID ==n->elementary_PID)
	{
		//NOT enter here
		return;
	}
	n->prev = h;
	n->next = h->next;
	h->next = n;
	return ;
}

int parse_pmt(uint8_t * pbuf, uint16_t buf_size, pmt_t * pPMT)
{
	uint16_t section_len = 0;
	uint8_t version_num;
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
	version_num = (pdata[5] >> 1) & 0x1F;
	if(version_num == pPMT->version_number && pPMT->es_list!=NULL)
	{
		return -1;
	}

	pmt_list_remove(pPMT);
	
	pPMT->section_length = section_len;

	//Transport Stream ID
	pPMT->program_number = (pdata[3] << 8) | pdata[4];

	pPMT->version_number = version_num;

	if (!(pdata[5] & 0x01)) //current_next_indicator
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
	pPMT->desriptor_list = NULL;
	//TODO: descriptor process
	section_len -= 2+ pPMT->program_info_length; 
	pdata += 2+ pPMT->program_info_length;

	while (section_len > 0)
	{
		struct es_info *el = malloc(sizeof(struct es_info));
		el->stream_type = TS_READ8(pdata);
		pdata+= 1;
		el->elementary_PID = TS_READ16(pdata)&0x1FFF;
		pdata+= 2;
		el->ES_info_length = TS_READ16(pdata)&0x0FFF;
		el->next = NULL;
		el->prev = el;
		pdata += 2+el->ES_info_length ;
		section_len -= (5+el->ES_info_length);
		//TODO: descriptor process
		pmt_list_insert(pPMT, el);
	}

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
	parse_cat(pkt,len,&psi.cat);
	return 0;
}

static int pmt_proc(uint16_t pid,uint8_t *pkt,uint16_t len)
{
	psi.pmt_bitmap[pid/64] |= (1<<(pid%64));
	//printf("pid %d pmt bit map %d\n",pid ,psi.pmt_bitmap[pid/64] );
	parse_pmt(pkt,len,&(psi.pmt[pid]));
	return 0;
}

static int drop_proc(uint16_t pid,uint8_t *pkt,uint16_t len)
{
	return 0;
}

table_ops drop_ops ={
	.table_id = RESERVED_TID,
	.table_proc = drop_proc,
};

table_ops pat_ops = {
	.table_id = PAT_TID,
	.table_proc = pat_proc,
};

table_ops cat_ops = {
	.table_id = CAT_TID,
	.table_proc = cat_proc,
};

table_ops pmt_ops = {
	.table_id = PMT_TID,
	.table_proc = pmt_proc,
};


