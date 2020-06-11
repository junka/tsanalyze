#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "err.h"
#include "filter.h"
#include "table.h"
#include "ts.h"
#include "utils.h"
#include "result.h"

static mpeg_psi_t psi;

int psi_table_init(void)
{
	int i = 0;
	list_head_init(&(psi.pat.h));
	list_head_init(&(psi.cat.list));
	for (i = 0; i < 8192; i++) {
		list_head_init(&(psi.pmt[i].h));
		list_head_init(&(psi.pmt[i].list));
	}
	list_head_init(&(psi.nit_actual.h));
	list_head_init(&(psi.nit_actual.list));
	list_head_init(&(psi.nit_other.h));
	list_head_init(&(psi.nit_other.list));
	list_head_init(&(psi.eit.h));
	list_head_init(&(psi.bat.h));
	list_head_init(&(psi.bat.list));
	list_head_init(&(psi.sdt_actual.h));
	list_head_init(&(psi.sdt_other.h));
	list_head_init(&(psi.tot.list));
	return 0;
}

static char const *get_stream_type(uint8_t type)
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

static void dump_pat(pat_t *p_pat)
{
	if (p_pat == NULL)
		return;

	struct program_node *pn = NULL;

	rout(0,"PAT");
	rout(1,"section_length: %d", p_pat->section_length);
	rout(1,"transport_stream_id : %d", p_pat->transport_stream_id);
	rout(1,"version_number      : %d", p_pat->version_number);
	rout(2,"program_number @ PMT_PID");
	list_for_each(&p_pat->h, pn, n)
	{
		rout(2,"%14d @ 0x%x (%d)", pn->program_number, pn->program_map_PID, pn->program_map_PID);
	}
	rout(1,"  active              : 0x%x", p_pat->current_next_indicator);
}

static void dump_cat(cat_t *p_cat)
{
	descriptor_t *pn = NULL;
	CA_descriptor_t *ca = NULL;

	rout(0,"CAT");
	rout(1,"version number %d", p_cat->version_number);
	list_for_each(&(p_cat->list), pn, n)
	{
		ca = (CA_descriptor_t *)pn;
		uint16_t system_id = ca->CA_system_ID;
		uint16_t emm_pid = ca->CA_PID;
		// p_stream->ca_num++;
		rout(2,"cat system id 0x%04x    emm pid 0x%04x", system_id, emm_pid);
	}
}

static void dump_tdt(tdt_t *p_tdt)
{
	rout(0,"TDT: Time and Date Table");
	rout(1,"UTC time       : %s", convert_UTC(&p_tdt->utc_time));
}

static void dump_tot(tot_t *p_tot)
{
	rout(0, "TOT: Time Offset Table");
	rout(1,"UTC time       : %s", convert_UTC(&p_tot->utc_time));

	dump_descriptors(2, &(p_tot->list));
}

static void dump_pmt(pmt_t *p_pmt, uint16_t pid)
{
	struct es_node *pn = NULL;

	rout(0,"active PMT");
	rout(1,"program_number : %d  => pmt pid 0x%x", p_pmt->program_number, pid);
	rout(1,"version_number : %d", p_pmt->version_number);
	rout(1,"PCR_PID        : 0x%x (%d)", p_pmt->PCR_PID, p_pmt->PCR_PID);
	dump_descriptors(2, &(p_pmt->list));
	rout(1,"components");
	rout(2,"type @ elementary_PID");
	list_for_each(&(p_pmt->h), pn, n)
	{
		rout(2,"0x%02x (%s) @ 0x%x", pn->stream_type, get_stream_type(pn->stream_type), pn->elementary_PID);
		dump_descriptors(3, &(pn->list));
	}
}

static void dump_sdt(sdt_t *p_sdt)
{
	if (p_sdt == NULL)
		return;
	struct service_node *pn = NULL;

	if(p_sdt->table_id == SDT_ACTUAL_TID)
		rout(0,"SDT ACTUAL tid 0x%x", SDT_ACTUAL_TID);
	else if (p_sdt->table_id == SDT_OTHER_TID)
		rout(0,"SDT OTHER tid 0x%x", SDT_OTHER_TID);

	rout(1,"transport_stream_id : 0x%x", p_sdt->transport_stream_id);
	rout(1,"section_length: %d", p_sdt->section_length);
	rout(1,"version_number      : %d", p_sdt->version_number);
	rout(1,"Current next   : %s", p_sdt->current_next_indicator ? "yes" : "no");
	rout(1,"original_network_id : 0x%x", p_sdt->original_network_id);
	if (!list_empty(&(p_sdt->h)))
	{
		list_for_each(&(p_sdt->h), pn, n)
		{
			rout(2,"service_id 0x%04x(%d) ", pn->service_id, pn->service_id);
			rout(3,"EIT_schedule_flag 0x%x ", pn->EIT_schedule_flag);
			rout(3,"EIT_present_following_flag 0x%x ", pn->EIT_present_following_flag);
			rout(3,"running_status 0x%x ", pn->running_status);
			rout(3,"free_CA_mode 0x%x ", pn->free_CA_mode);
			dump_descriptors(4, &(pn->list));
		}
	}
}

static void dump_bat(bat_t *p_bat)
{
	if (p_bat == NULL)
		return;
	struct transport_stream_node *pn = NULL;

	rout(0,"BAT");
	rout(1,"section_length: %d", p_bat->section_length);
	rout(1,"bouquet_id : 0x%x", p_bat->bouquet_id);
	rout(1,"version_number      : %d", p_bat->version_number);
	rout(1,"Current next   : %s", p_bat->current_next_indicator ? "yes" : "no");
	dump_descriptors(2, &(p_bat->list));
	if(p_bat->transport_stream_loop_length)
	{
		rout(1,"transport_streams: ");
		list_for_each(&(p_bat->h), pn, n)
		{
			rout(2,"0x%04x(%d) ", pn->transport_stream_id, pn->transport_stream_id);
			rout(3,"original_network_id %x ", pn->original_network_id);
			if(pn->transport_descriptors_length)
				dump_descriptors(4, &(pn->list));
		}
	}
}

static void dump_nit(nit_t *p_nit)
{
	if (p_nit == NULL)
		return;
	struct transport_stream_node *pn = NULL;

	if (p_nit->table_id == NIT_ACTUAL_TID)
		rout(0,"NIT ACTUAL tid 0x%x", p_nit->table_id);
	if (p_nit->table_id == NIT_OTHER_TID)
		rout(0,"NIT OTHER tid 0x%x", p_nit->table_id);
	rout(1,"section_length: %d", p_nit->section_length);
	rout(1,"network_id : 0x%x", p_nit->network_id);
	rout(1,"version_number      : %d", p_nit->version_number);
	rout(1,"Current next   : %s", p_nit->current_next_indicator ? "yes" : "no");
	dump_descriptors(2, &(p_nit->list));
	rout(2, "transport_stream ");
	list_for_each(&(p_nit->h), pn, n)
	{
		rout(3,"transport_stream_id 0x%x ", pn->transport_stream_id);
		rout(4,"original_network_id 0x%x ", pn->original_network_id);
		dump_descriptors(5, &(pn->list));
	}
}

void dump_tables(void)
{
	struct tsa_config *tsaconf = get_config();
	if (tsaconf->brief == 0)
		return;
	if (tsaconf->tables == 0)
		tsaconf->tables = UINT8_MAX;

	int i = 0;

	if (psi.stats.pat_sections && (tsaconf->tables & PAT_SHOW))
		dump_pat(&psi.pat);
	if (psi.ca_num > 0 && (tsaconf->tables & CAT_SHOW)) {
		dump_cat(&psi.cat);
	}
	// pid
	if (tsaconf->tables & PMT_SHOW) {
		for (i = 0x10; i < 0x2000; i++) {
			if (psi.pmt_bitmap[i / 64] & ((uint64_t)1 << (i % 64))) {
				dump_pmt(&psi.pmt[i], i);
			}
		}
	}

	if (psi.stats.sdt_actual_sections && (tsaconf->tables & SDT_SHOW))
		dump_sdt(&psi.sdt_actual);
	if (psi.stats.sdt_other_sections && (tsaconf->tables & SDT_SHOW))
		dump_sdt(&psi.sdt_other);
	if (psi.stats.nit_actual_sections && (tsaconf->tables & NIT_SHOW))
		dump_nit(&psi.nit_actual);
	if (psi.stats.nit_other_sections && (tsaconf->tables & NIT_SHOW))
		dump_nit(&psi.nit_other);
	if (psi.stats.bat_sections && (tsaconf->tables & BAT_SHOW))
		dump_bat(&psi.bat);
	if (psi.stats.tdt_sections && (tsaconf->tables & TDT_SHOW))
		dump_tdt(&psi.tdt);
	if (psi.stats.tot_sections && (tsaconf->tables & TDT_SHOW))
		dump_tot(&psi.tot);
	
	res_close();
}

void unregister_pmt_ops(uint16_t pid);

void register_pmt_ops(uint16_t pid);

int parse_pat(uint8_t *pbuf, uint16_t buf_size, pat_t *pPAT)
{
	uint16_t section_len = 0;
	uint8_t version_num;
	uint16_t program_num, program_map_PID;
	uint8_t *pdata = pbuf;
	struct program_node *pn = NULL, *next = NULL;

	if (unlikely(pbuf == NULL || pPAT == NULL)) {
		return NULL_PTR;
	}

	pPAT->table_id = TS_READ8(pdata);
	if (unlikely(pPAT->table_id  != PAT_TID)) {
		return INVALID_TID;
	}

	section_len = ((pdata[1] << 8) | pdata[2]) & 0x0FFF;
	if (unlikely(section_len > 0x3FD)) // For pat , maximum
	{
		return INVALID_SEC_LEN;
	}
	if (!(pdata[5] & 0x01)) // current_next_indicator
	{
		return -1;
	}
	version_num = (pdata[5] >> 1) & 0x1F;
	if (version_num == pPAT->version_number && !list_empty(&(pPAT->h))) {
		return -1;
	}
	if (!list_empty(&(pPAT->h))) {
		list_for_each_safe(&pPAT->h, pn, next, n)
		{
			unregister_pmt_ops(pn->program_map_PID);
			list_del(&pn->n);
			free(pn);
		}
	}

	pPAT->section_length = section_len;

	// Transport Stream ID
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

	// TODO: limit program total length

	while (section_len > 0) {
		program_num = TS_READ16(pdata);
		pdata += 2;
		program_map_PID = TS_READ16(pdata) & 0x1FFF;
		pdata += 2;
		section_len -= 4;
		if (program_num == 0xFFFF) {
			break;
		}
		if (pPAT->program_bitmap[program_num / 64] & ((uint64_t)1 << (program_num % 64))) {
			list_for_each_safe(&(pPAT->h), pn, next, n)
			{
				if (pn->program_number == program_num) {
					pn->program_map_PID = program_map_PID;
				}
			}
		} else {
			// rout("teset1 program_num %x",program_num);
			register_pmt_ops(program_map_PID);
			pn = malloc(sizeof(struct program_node));
			pn->program_number = program_num;
			pn->program_map_PID = program_map_PID;
			pPAT->program_bitmap[program_num / 64] |= ((uint64_t)1 << (program_num % 64));
			list_add(&(pPAT->h), &(pn->n));
		}
	}

	return 0;
}

int parse_cat(uint8_t *pbuf, uint16_t buf_size, cat_t *pCAT)
{
	uint16_t section_len = 0;
	uint8_t *pdata = pbuf;

	if (unlikely(pbuf == NULL || pCAT == NULL)) {
		return NULL_PTR;
	}

	pCAT->table_id = TS_READ8(pdata);
	if (unlikely(pdata[0] != CAT_TID)) {
		return -1;
	}

	section_len = ((pdata[1] << 8) | pdata[2]) & 0x0FFF;
	if (unlikely(section_len > 0x3FD)) // For cat , maximum
	{
		return -1;
	}
	pCAT->section_length = section_len;

	// Transport Stream ID
	pCAT->transport_stream_id = (pdata[3] << 8) | pdata[4];

	pCAT->version_number = (pdata[5] >> 1) & 0x1F;

	if (!(pdata[5] & 0x01)) // current_next_indicator
	{
		return -1;
	}

	pCAT->section_number = pdata[6];
	pCAT->last_section_number = pdata[7];

	section_len -= 5 + 4;
	pdata += 8;

	// list_add(&(pCAT->list),
	parse_descriptors(&(pCAT->list), pdata, section_len);

	return 0;
}

int parse_pmt(uint8_t *pbuf, uint16_t buf_size, pmt_t *pPMT)
{
	int16_t section_len = 0;
	uint8_t version_num;
	uint8_t *pdata = pbuf;
	struct es_node *pn = NULL, *next = NULL;

	if (unlikely(pbuf == NULL || pPMT == NULL)) {
		return -1;
	}

	pPMT->table_id = TS_READ8(pdata);
	if (unlikely(pPMT->table_id != PMT_TID)) {
		return -1;
	}
	section_len = (int16_t)(((int16_t)pdata[1] << 8) | pdata[2]) & 0x0FFF;
	if (unlikely(section_len > 0x3FD)) // For pmt , maximum
	{
		return -1;
	}
	version_num = (pdata[5] >> 1) & 0x1F;
	if (version_num == pPMT->version_number && !list_empty(&(pPMT->h))) {
		return -1;
	}
	if (!list_empty(&(pPMT->h))) {
		list_for_each_safe(&(pPMT->h), pn, next, n)
		{
			list_del(&(pn->n));
			free(pn);
		}
	}

	pPMT->section_length = section_len;
	// Transport Stream ID
	pPMT->program_number = (pdata[3] << 8) | pdata[4];
	pPMT->version_number = version_num;

	if ((pdata[5] & 0x01) == 0) // current_next_indicator
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
	pPMT->program_info_length = TS_READ16(pdata) & 0x0FFF;

	pdata += 2;

	if (!list_empty(&(pPMT->list)))
		free_descriptors(&(pPMT->list));

	parse_descriptors(&(pPMT->list), pdata, pPMT->program_info_length);
	section_len -= 2 + pPMT->program_info_length;
	pdata += pPMT->program_info_length;

	// rout("section_len %d",section_len);

	while (section_len > 0) {
		pn = malloc(sizeof(struct es_node));
		list_head_init(&(pn->list));
		pn->stream_type = TS_READ8(pdata);
		pdata += 1;
		pn->elementary_PID = TS_READ16(pdata) & 0x1FFF;
		pdata += 2;
		pn->ES_info_length = TS_READ16(pdata) & 0x0FFF;
		pdata += 2;
		parse_descriptors(&(pn->list), pdata, (int)pn->ES_info_length);
		pdata += pn->ES_info_length;
		section_len -= (5 + pn->ES_info_length);
		list_add(&(pPMT->h), &(pn->n));
	}

	return 0;
}

int parse_nit(uint8_t *pbuf, uint16_t buf_size, nit_t *pNIT)
{
	int16_t section_len = 0;
	uint8_t version_num;
	uint8_t *pdata = pbuf;
	struct transport_stream_node *pn = NULL, *next = NULL;

	if (unlikely(pbuf == NULL || pNIT == NULL)) {
		return NULL_PTR;
	}

	if (unlikely(pdata[0] != NIT_OTHER_TID && pdata[0] != NIT_ACTUAL_TID)) {
		return INVALID_TID;
	}

	pNIT->table_id = TS_READ8(pdata);
	section_len = (int16_t)((pdata[1] << 8) | pdata[2]) & 0x0FFF;
	if (unlikely(section_len > 0x3FD)) // For nit , maximum
	{
		return INVALID_SEC_LEN;
	}
	version_num = (pdata[5] >> 1) & 0x1F;
	if (unlikely(version_num == pNIT->version_number && !list_empty(&(pNIT->h)))) {
		return -1;
	}
	pNIT->section_length = section_len;
	pNIT->version_number = version_num;
	pNIT->network_id = (pdata[3] << 8) | pdata[4];
	pdata += 8;
	pNIT->network_descriptors_length = TS_READ16(pdata) & 0xFFF;
	if (!list_empty(&(pNIT->list)))
		free_descriptors(&(pNIT->list));
	if (!list_empty(&(pNIT->h))) {
		list_for_each_safe(&(pNIT->h), pn, next, n)
		{
			list_del(&(pn->n));
			free(pn);
		}
	}
	pdata += 2 + pNIT->network_descriptors_length;
	pNIT->transport_stream_loop_length = TS_READ16(pdata) & 0xFFF;
	section_len -= 10;
	section_len -= pNIT->network_descriptors_length;
	section_len -= 4;
	pdata += 2;
	while (section_len > 0) {
		pn = malloc(sizeof(struct transport_stream_node));
		list_head_init(&(pn->list));
		pn->transport_stream_id = TS_READ16(pdata);
		pdata += 2;
		pn->original_network_id = TS_READ16(pdata);
		pdata += 2;
		pn->transport_descriptors_length = TS_READ16(pdata);
		pdata += 2;
		parse_descriptors(&(pn->list), pdata, (int)pn->transport_descriptors_length);
		pdata += pn->transport_descriptors_length;
		section_len -= 6 + pn->transport_descriptors_length;
		list_add(&(pNIT->h), &(pn->n));
	}
	return 0;
}

int parse_bat(uint8_t *pbuf, uint16_t buf_size, bat_t *pBAT)
{
	int16_t section_len = 0;
	uint8_t version_num;
	uint8_t *pdata = pbuf;
	struct transport_stream_node *pn = NULL, *next = NULL;

	if (unlikely(pbuf == NULL || pBAT == NULL)) {
		return NULL_PTR;
	}

	if (unlikely(pdata[0] != BAT_TID)) {
		return INVALID_TID;
	}

	pBAT->table_id = TS_READ8(pdata);
	section_len = (int16_t)((pdata[1] << 8) | pdata[2]) & 0x0FFF;
	if (unlikely(section_len > 0x3FD)) // For bat , maximum
	{
		return INVALID_SEC_LEN;
	}
	version_num = (pdata[5] >> 1) & 0x1F;
	if (version_num == pBAT->version_number && !list_empty(&(pBAT->h))) {
		return -1;
	}
	if (!list_empty(&(pBAT->h))) {
		list_for_each_safe(&(pBAT->h), pn, next, n)
		{
			list_del(&(pn->n));
			free(pn);
		}
	}

	pdata += 3;
	pBAT->bouquet_id = TS_READ16(pdata);
	pdata += 2;
	pBAT->section_length = section_len;
	pBAT->version_number = version_num;
	pdata += 3;
	pBAT->bouquet_descriptors_length = TS_READ16(pdata) & 0xFFF;
	pdata += 2;
	if (!list_empty(&(pBAT->list)))
		free_descriptors(&(pBAT->list));
	parse_descriptors(&(pBAT->list), pdata, pBAT->bouquet_descriptors_length);
	pdata += pBAT->bouquet_descriptors_length;
	section_len -= 7;
	section_len -= pBAT->bouquet_descriptors_length;
	pBAT->transport_stream_loop_length = TS_READ16(pdata) & 0xFFF;
	pdata += 2;
	section_len -= 2;
	section_len -= 4;
	while (section_len > 0) {
		pn = malloc(sizeof(struct transport_stream_node));
		list_head_init(&(pn->list));
		pn->transport_stream_id = TS_READ16(pdata);
		pdata += 2;
		pn->original_network_id = TS_READ16(pdata);
		pdata += 2;
		pn->transport_descriptors_length = TS_READ16(pdata) & 0xFFF;
		pdata += 2;
		parse_descriptors(&(pn->list), pdata, pn->transport_descriptors_length);
		pdata += pn->transport_descriptors_length;
		section_len -= (6 + pn->transport_descriptors_length);
		list_add(&(pBAT->h), &(pn->n));
	}

	return 0;
}

int parse_sdt(uint8_t *pbuf, uint16_t buf_size, sdt_t *pSDT)
{
	int16_t section_len = 0, loop_len;
	uint8_t version_num;
	uint8_t *pdata = pbuf;
	uint8_t last_sec, cur_sec;
	uint16_t ts_id;
	struct service_node *pn = NULL, *next = NULL;

	if (unlikely(pbuf == NULL || pSDT == NULL)) {
		return NULL_PTR;
	}
	if (unlikely(pdata[0] != SDT_ACTUAL_TID && pdata[0] != SDT_OTHER_TID)) {
		return INVALID_TID;
	}

	pSDT->table_id = TS_READ8(pdata);
	section_len = (((int16_t)pdata[1] << 8) | pdata[2]) & 0x0FFF;
	if (unlikely(section_len > 0x3FD)) // For sdt , maximum
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
	if (version_num == pSDT->version_number && last_sec == pSDT->last_section_number && !list_empty(&(pSDT->h))) {
		// printf("error here");
		return -1;
	}
	pSDT->transport_stream_id = ts_id;
	pSDT->section_length = section_len;
	pSDT->version_number = version_num;
	pSDT->original_network_id = TS_READ16(pdata);
	pSDT->last_section_number = last_sec;
	if (!list_empty(&(pSDT->h))) {
		list_for_each_safe(&(pSDT->h), pn, next, n)
		{
			list_del(&(pn->n));
			free(pn);
		}
	}

	if ((pSDT->section_bitmap[cur_sec / 64] & ((uint64_t)1 << (cur_sec % 64)))) {
		return -1;
	}
	// hexdump(pbuf, buf_size);
	pSDT->section_bitmap[cur_sec / 64] |= ((uint64_t)1 << (cur_sec % 64));
	pSDT->section_number = cur_sec;
	pdata += 3;
	section_len -= 8;
	section_len -= 4; // crc
	if (buf_size - 11 - 4 < section_len) {
		loop_len = buf_size - 11 - 4;
	} else {
		loop_len = section_len;
	}

	while (loop_len > 0) {
		// printf("ptr %x section len %d", pdata, section_len);
		pn = malloc(sizeof(struct service_node));
		list_head_init(&(pn->list));
		pn->service_id = TS_READ16(pdata);
		pdata += 2;
		pn->EIT_schedule_flag = (TS_READ8(pdata) >> 1) & 0x1;
		pn->EIT_present_following_flag = (TS_READ8(pdata)) & 0x1;
		pdata += 1;
		pn->running_status = (TS_READ16(pdata) >> 13) & 0x7;
		pn->free_CA_mode = (TS_READ16(pdata) >> 12) & 0x1;
		pn->descriptors_loop_length = TS_READ16(pdata) & 0x0FFF;
		pdata += 2;

		parse_descriptors(&(pn->list), pdata, (int)(pn->descriptors_loop_length));
		pdata += pn->descriptors_loop_length;
		// printf("desc len %d",si->descriptors_loop_length);
		loop_len -= (5 + pn->descriptors_loop_length);
		list_add(&(pSDT->h), &(pn->n));
	}

	return 0;
}

static int parse_eit(uint8_t *pbuf, uint16_t buf_size, eit_t *pEIT)
{
	uint16_t section_len = 0;
	uint8_t *pdata = pbuf;
	// struct event_node *pn = NULL;

	if (unlikely(pbuf == NULL || pEIT == NULL)) {
		return -1;
	}

	pdata += 1;
	section_len = TS_READ16(pdata) & 0xFFF;

	if (section_len > 0xFFD) // For eit , maximum
	{
		return -1;
	}
	pEIT->section_length = section_len;

	return 0;
}

static int parse_tdt(uint8_t *pbuf, uint16_t buf_size, tdt_t *pTDT)
{
	uint16_t section_len = 0;
	uint8_t *pdata = pbuf;

	if (unlikely(pbuf == NULL || pTDT == NULL)) {
		return -1;
	}

	if (unlikely(pdata[0] != TDT_TID)) {
		return -1;
	}

	pdata += 1;
	section_len = TS_READ16(pdata) & 0xFFF;
	pTDT->section_length = section_len;
	pdata += 2;
	memcpy(&pTDT->utc_time, pdata, 5);
	return 0;
}

static int parse_tot(uint8_t *pbuf, uint16_t buf_size, tot_t *pTOT)
{
	uint8_t *pdata = pbuf;

	if (unlikely(pbuf == NULL || pTOT == NULL)) {
		return -1;
	}

	if (unlikely(pdata[0] != TOT_TID)) {
		return -1;
	}

	pdata += 1;
	pTOT->section_length = TS_READ16(pdata) & 0xFFF;
	pdata += 2;
	memcpy(&pTOT->utc_time, pdata, 5);
	pdata += 5;
	pTOT->descriptors_loop_length = TS_READ16(pdata) & 0xFFF;
	pdata += 2;
	if (!list_empty(&(pTOT->list)))
		free_descriptors(&(pTOT->list));
	parse_descriptors(&(pTOT->list), pdata, (int)pTOT->descriptors_loop_length);
	return 0;
}

static int pat_proc(__attribute__((unused)) uint16_t pid, uint8_t *pkt, uint16_t len)
{
	psi.stats.pat_sections++;
	parse_pat(pkt, len, &psi.pat);
	return 0;
}

static int cat_proc(__attribute__((unused)) uint16_t pid, uint8_t *pkt, uint16_t len)
{
	psi.stats.cat_sections++;
	parse_cat(pkt, len, &psi.cat);
	return 0;
}

static int pmt_proc(uint16_t pid, uint8_t *pkt, uint16_t len)
{
	parse_pmt(pkt, len, &(psi.pmt[pid]));
	return 0;
}

static int nit_proc(__attribute__((unused)) uint16_t pid, uint8_t *pkt, uint16_t len)
{
	if(pkt[0] == NIT_ACTUAL_TID) {
		psi.stats.nit_actual_sections++;
		parse_nit(pkt, len, &(psi.nit_actual));
	}else if(pkt[0] == NIT_OTHER_TID){
		psi.stats.nit_other_sections++;
		parse_nit(pkt, len, &(psi.nit_other));
	}
	return 0;
}

static int sdt_bat_proc(__attribute__((unused)) uint16_t pid, uint8_t *pkt, uint16_t len)
{
	if (pkt[0] == BAT_TID) {
		psi.stats.bat_sections++;
		parse_bat(pkt, len, &(psi.bat));
	} else if (pkt[0] == SDT_ACTUAL_TID) {
		psi.stats.sdt_actual_sections++;
		parse_sdt(pkt, len, &(psi.sdt_actual));
	} else if (pkt[0] == SDT_OTHER_TID) {
		psi.stats.sdt_other_sections++;
		parse_sdt(pkt, len, &(psi.sdt_other));

	}
	return 0;
}

static int eit_proc(__attribute__((unused)) uint16_t pid, uint8_t *pkt, uint16_t len)
{
	psi.stats.eit_sections++;
	parse_eit(pkt, len, &psi.eit);
	return 0;
}

static int tdt_tot_proc(__attribute__((unused)) uint16_t pid, uint8_t *pkt, uint16_t len)
{
	if (TDT_TID == pkt[0]) {
		psi.stats.tdt_sections++;
		parse_tdt(pkt, len, &psi.tdt);
	} else if (TOT_TID == pkt[0]) {
		psi.stats.tot_sections++;
		parse_tot(pkt, len, &psi.tot);
	}
	return 0;
}

static void init_table_filter(uint16_t pid, uint8_t tableid, uint8_t mask, filter_cb func)
{
	filter_t *f = filter_alloc(pid);
	filter_param_t para;
	para.depth = 1;
	para.coff[0] = tableid;
	para.mask[0] = mask;
	para.negete[0] = 0;
	filter_set(f, &para, func);
}
static void uninit_table_filter(uint16_t pid, uint8_t tableid, uint8_t mask)
{
	filter_param_t para;
	filter_t *f = NULL;
	para.depth = 1;
	para.coff[0] = tableid;
	para.mask[0] = mask;
	para.negete[0] = 0;
	f = filter_lookup(pid, &para);
	if (f) {
		filter_free(f);
	}
}

void init_table_ops(void)
{
	psi_table_init();

	init_table_filter(PAT_PID, PAT_TID, 0xFF, pat_proc);
	init_table_filter(CAT_PID, CAT_TID, 0xFF, cat_proc);

	//filter nit actual and other at same time
	init_table_filter(NIT_PID, NIT_ACTUAL_TID, 0xFE, nit_proc);
	init_table_filter(EIT_PID, EIT_ACTUAL_TID, 0xFF, eit_proc);
	init_table_filter(SDT_PID, SDT_ACTUAL_TID, 0xFF, sdt_bat_proc);
	init_table_filter(SDT_PID, SDT_OTHER_TID, 0xFF, sdt_bat_proc);
	init_table_filter(BAT_PID, BAT_TID, 0xFF, sdt_bat_proc);

	//filter tdt and tot at same time
	init_table_filter(TDT_PID, TDT_TID, 0xFF, tdt_tot_proc);
	init_table_filter(TOT_PID, TOT_TID, 0xFF, tdt_tot_proc);
}

void uninit_table_ops(void)
{
	struct program_node *pn = NULL, *next = NULL;
	if (!list_empty(&(psi.pat.h))) {
		list_for_each_safe(&psi.pat.h, pn, next, n)
		{
			unregister_pmt_ops(pn->program_map_PID);
			list_del(&pn->n);
			free(pn);
		}
	}
	uninit_table_filter(PAT_PID, PAT_TID, 0xFF);
	uninit_table_filter(CAT_PID, CAT_TID, 0xFF);
	uninit_table_filter(NIT_PID, NIT_ACTUAL_TID, 0xFF);
	uninit_table_filter(EIT_PID, EIT_ACTUAL_TID, 0xFF);
	uninit_table_filter(BAT_PID, BAT_TID, 0xFF);
	uninit_table_filter(TDT_PID, TDT_TID, 0xFF);
	uninit_table_filter(TOT_PID, TOT_TID, 0xFF);
}

void register_pmt_ops(uint16_t pid)
{
	if (pid == NIT_PID)
		return;
	psi.pmt_bitmap[pid / 64] |= ((uint64_t)1 << (pid % 64));
	init_table_filter(pid, PMT_TID, 0xFF, pmt_proc);
}

void unregister_pmt_ops(uint16_t pid)
{
	psi.pmt_bitmap[pid / 64] &= ~((uint64_t)1 << (pid % 64));
	uninit_table_filter(pid, PMT_TID, 0xFF);
}

bool check_pmt_pid(uint16_t pid)
{
	if (psi.pmt_bitmap[pid / 64] & ((uint64_t)1 << (pid % 64)))
		return true;
	return false;
}