#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "descriptor.h"
#include "error.h"
#include "list.h"
#include "pes.h"
#include "filter.h"
#include "table.h"
#include "table_priv.h"
#include "ts.h"
#include "utils.h"
#include "result.h"
#include "subtitle.h"
#include "teletext.h"

static mpeg_psi_t psi;

static void init_table_filter(uint16_t pid, uint8_t tableid, uint8_t mask, filter_cb func)
{
	filter_t *f = filter_alloc(pid);
	filter_param_t para;
	para.depth = 1;
	para.coff[0] = tableid;
	para.mask[0] = mask;
	para.negate[0] = 0;
	filter_set(f, &para, func);
}

static void uninit_table_filter(uint16_t pid, uint8_t tableid, uint8_t mask)
{
	filter_param_t para;
	filter_t *f = NULL;
	para.depth = 1;
	para.coff[0] = tableid;
	para.mask[0] = mask;
	para.negate[0] = 0;
	f = filter_lookup(pid, &para);
	if (f) {
		filter_free(f);
	}
}

static int psi_table_init(void)
{
	int i = 0;

	memset(&psi, 0, sizeof(psi));

	memset(psi.sdt_actual.sdt_header.section_bitmap, 0, sizeof(uint64_t) * 4);
	psi.sdt_actual.sdt_header.version_number = 0x1F;
	memset(psi.sdt_actual.sdt_header.sections, 0, sizeof(struct section_node *) * MAX_SECTION_NUM);


	list_head_init(&(psi.pat_list));
	list_head_init(&(psi.cat_list));
	for (i = 0; i < 8192; i++) {
		list_head_init(&(psi.pmt[i].h));
		list_head_init(&(psi.pmt[i].list));
	}
	list_head_init(&(psi.nit_actual.h));
	list_head_init(&(psi.nit_actual.list));
	list_head_init(&(psi.nit_other.h));
	list_head_init(&(psi.nit_other.list));
	list_head_init(&(psi.eit_actual.h));
	list_head_init(&(psi.eit_other.h));
	list_head_init(&(psi.bat.h));
	list_head_init(&(psi.bat.list));
	list_head_init(&(psi.sdt_actual.h));
	list_head_init(&(psi.sdt_other.h));
	list_head_init(&(psi.tot.list));
	list_head_init(&(psi.tsdt.list));

	/* ATSC PSIP table lists (own global in src/atsc_psip.c) */
	atsc_init_tables();
	return 0;
}


void dump_section_header(const char *table_name, struct table_header *hdr)
{
	if (hdr == NULL)
		return;
	rout(0, table_name, NULL);
	rout(1,"section_length", "%d", hdr->section_length);
	if (hdr->section_syntax_indicator == 1)
		rout(1,"transport_stream_id", "%d", hdr->table_id_ext);
	rout(1, "version_number", "%d", hdr->version_number);
	rout(1, "active", "0x%x", hdr->current_next_indicator);
}

static void dump_pat(pat_t *p_pat)
{
	if (p_pat == NULL)
		return;

	struct program_node *pn = NULL;

	dump_section_header("PAT", &p_pat->pat_header);
	rout(1, "program_number @ PMT_PID", NULL);
	list_for_each(&p_pat->h, pn, n)
	{
		rout(1, NULL, "%14d @ 0x%x (%d)", pn->program_number, pn->program_map_PID, pn->program_map_PID);
	}
}

static void dump_cat(cat_t *p_cat)
{
	dump_section_header("CAT", &p_cat->cat_header);
	rout(1, "ca systems", NULL);
	if (!list_empty(&(p_cat->list)))
		dump_descriptors(2, &(p_cat->list));
}

static void dump_tsdt(tsdt_t *p_tsdt)
{
	dump_section_header("TSDT", &p_tsdt->tsdt_header);
	if (!list_empty(&(p_tsdt->list)))
	{
		dump_descriptors(2, &p_tsdt->list);
	}
}

static void dump_pmt(pmt_t *p_pmt, uint16_t pid)
{
	struct es_node *pn = NULL;

	rout(0, "active PMT", NULL);
	rout(1, "program_number", "%d  => pmt pid 0x%x", p_pmt->program_number, pid);
	rout(1, "version_number", "%d", p_pmt->pmt_header.version_number);
	rout(1, "PCR_PID", "0x%x (%d)", p_pmt->PCR_PID, p_pmt->PCR_PID);
	dump_descriptors(2, &(p_pmt->list));
	rout(1, "components", NULL);
	rout(2, "type @ elementary_PID", NULL);
	list_for_each(&(p_pmt->h), pn, n)
	{
		char buff[512];
		snprintf(buff, 512, "0x%02x (%s) @ 0x%x", pn->stream_type, get_stream_type(pn->stream_type), pn->elementary_PID);
		rout(3, buff, NULL);
		if (!list_empty(&pn->list)) {
			dump_descriptors(4, &(pn->list));
		}
	}
}

/* DVB SI (NIT/BAT/SDT/EIT/TDT/TOT) and ATSC PSIP dumps now live in
 * src/dvb_si.c and src/atsc_psip.c respectively. */

static void convert_pids_to_tables(void)
{
	struct tsa_config *tsaconf = get_config();
	for (int i = 0; i < 8192; i ++) {
		if (tsaconf->pids[i]) {
			switch (i) {
				case PAT_PID:
					tsaconf->tables |= PAT_SHOW;
					break;
				case CAT_PID:
					tsaconf->tables |= CAT_SHOW;
					break;
				case TSDT_PID:
					tsaconf->tables |= TSDT_SHOW;
					break;
				case NIT_PID:
					tsaconf->tables |= NIT_SHOW;
					break;
				case SDT_PID:
					tsaconf->tables |= SDT_SHOW;
					tsaconf->tables |= BAT_SHOW;
					break;
				case EIT_PID:
				tsaconf->tables |= EIT_SHOW;
				break;
			case TDT_PID:
				tsaconf->tables |= TDT_SHOW;
				break;
			case MGT_PID:
				tsaconf->tables |= PSIP_SHOW;
				break;
				default:
					tsaconf->tables |= PMT_SHOW;
					break;
			}

		}
	}
}

void dump_tables(void)
{
	convert_pids_to_tables();
	struct tsa_config *tsaconf = get_config();
	if (tsaconf->brief == 0)
		return;

	/*show all tables in default */
	if (tsaconf->tables == 0)
		tsaconf->tables = (uint16_t)(PAT_SHOW | CAT_SHOW | PMT_SHOW | TSDT_SHOW |
					    NIT_SHOW | SDT_SHOW | BAT_SHOW | TDT_SHOW |
					    EIT_SHOW | PSIP_SHOW);

	pat_t *pat = NULL, *pat_next = NULL;
	if (psi.stats.pat_sections && (tsaconf->tables & PAT_SHOW)) {
		list_for_each_safe(&(psi.pat_list), pat, pat_next, n) {
			dump_pat(pat);
		}
	}

	cat_t *cat = NULL, *cat_next = NULL;
	if (psi.ca_num > 0 && (tsaconf->tables & CAT_SHOW)) {
		list_for_each_safe(&(psi.cat_list), cat, cat_next, n) {
			dump_cat(cat);
		}
	}

	if (psi.stats.tsdt_sections && (tsaconf->tables & TSDT_SHOW))
		dump_tsdt(&psi.tsdt);

	// pid
	if (tsaconf->tables & PMT_SHOW) {
		for (int i = 0x10; i < 0x2000; i++) {
			if (bitmap64_get(psi.pmt_bitmap, i)) {
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
	
	if (psi.stats.eit_actual_sections && (tsaconf->tables & EIT_SHOW))
		dump_eit(&psi.eit_actual);
	if (psi.stats.eit_other_sections && (tsaconf->tables & EIT_SHOW))
		dump_eit(&psi.eit_other);

	if (atsc_tables_seen() && (tsaconf->tables & PSIP_SHOW))
		dump_atsc_tables();

}

static void clear_sections(struct section_node *nodes, int num)
{
	if (num > MAX_SECTION_NUM)
		num = MAX_SECTION_NUM;
	for (int i = 0; i < num; i ++)
	{
		if (nodes[i].len != 0 && nodes[i].ptr != NULL)
			free(nodes[i].ptr);
		nodes[i].len = 0;
	}
}

static void free_table_header(struct table_header *h)
{
	if (h->private_data_byte)
		free(h->private_data_byte);
	clear_sections(h->sections, h->last_section_number + 1);
}

/* free a descriptor-bearing list whose nodes are of the same type */
#define free_list_nodes(head, node, next)				\
	list_for_each_safe((head), (node), (next), n)		\
	{													\
		free_descriptors(&(node)->list);				\
		list_del(&(node)->n);							\
		free(node);										\
	}

void free_tables(void)
{
	int i = 0;
	cat_t *catn = NULL, *cnext = NULL;
	pat_t *patn = NULL, *pnext = NULL;
	struct program_node *pn = NULL, *pat_next = NULL;
	struct es_node *no = NULL, *pmt_next = NULL;
	struct service_node *sn = NULL, *sdt_next = NULL;
	struct transport_stream_node *tn = NULL, *nit_next = NULL, *bat_next = NULL;
	struct event_node *en = NULL, *eit_next = NULL;

	// clear cat
	if (psi.ca_num > 0) {
		list_for_each_safe(&psi.cat_list, catn, cnext, n) {
			free_descriptors(&catn->list);
			free_table_header(&catn->cat_header);
		}
	}

	// pid
	for (i = 0; i < 0x2000; i++) {
		if (bitmap64_get(psi.pmt_bitmap, i)) {
			list_for_each_safe(&(psi.pmt[i].h), no, pmt_next, n)
			{
				free_descriptors(&(no->list));
				list_del(&(no->n));
				free(no);
			}
			free_descriptors(&(psi.pmt[i].list));
			free_table_header(&psi.pmt[i].pmt_header);
			bitmap64_clear(psi.pmt_bitmap, i);
		}
	}
	/* clear pat sections */
	if (psi.stats.pat_sections){
		list_for_each_safe(&psi.pat_list, patn, pnext, n) {
			list_for_each_safe(&patn->h, pn, pat_next, n)
			{
				unregister_pmt_ops(pn->program_map_PID);
				list_del(&pn->n);
				free(pn);
			}
		
			free_table_header(&patn->pat_header);
		}
	}

	if (psi.stats.sdt_actual_sections) {
		if (!list_empty(&(psi.sdt_actual.h)))
			free_list_nodes(&psi.sdt_actual.h, sn, sdt_next);
		free_table_header(&psi.sdt_actual.sdt_header);
	}
	sdt_next = NULL;
	if (psi.stats.sdt_other_sections) {
		if (!list_empty(&(psi.sdt_other.h)))
			free_list_nodes(&psi.sdt_other.h, sn, sdt_next);
		free_table_header(&psi.sdt_other.sdt_header);
	}
	if (psi.stats.nit_actual_sections) {
		if (!list_empty(&(psi.nit_actual.list)))
			free_descriptors(&(psi.nit_actual.list));
		if (!list_empty(&(psi.nit_actual.h)))
			free_list_nodes(&psi.nit_actual.h, tn, nit_next);
		free_table_header(&psi.nit_actual.nit_header);
	}
	nit_next = NULL;
	if (psi.stats.nit_other_sections) {
		if (!list_empty(&(psi.nit_other.list)))
			free_descriptors(&(psi.nit_other.list));
		if (!list_empty(&(psi.nit_other.h)))
			free_list_nodes(&psi.nit_other.h, tn, nit_next);
		free_table_header(&psi.nit_other.nit_header);
	}
	if (psi.stats.bat_sections) {
		if (!list_empty(&(psi.bat.list)))
			free_descriptors(&(psi.bat.list));
		if (!list_empty(&(psi.bat.h)))
			free_list_nodes(&psi.bat.h, tn, bat_next);
		free_table_header(&psi.bat.bat_header);
	}
	if (psi.stats.eit_actual_sections) {
		if (!list_empty(&(psi.eit_actual.h)))
			free_list_nodes(&psi.eit_actual.h, en, eit_next);
		free_table_header(&psi.eit_actual.eit_header);
	}
	eit_next = NULL;
	if (psi.stats.eit_other_sections) {
		if (!list_empty(&(psi.eit_other.h)))
			free_list_nodes(&psi.eit_other.h, en, eit_next);
		free_table_header(&psi.eit_other.eit_header);
	}

	if (psi.stats.tot_sections) {
		if (!list_empty(&(psi.tot.list)))
			free_descriptors(&(psi.tot.list));
	}

	unregister_scte_ops();
	unregister_pes_ops();

}

static uint8_t * concat_sections(struct section_node *nodes, int total_length, int num)
{
	int len = 0;
	uint8_t *ret = calloc(1, total_length);
	if (ret == NULL)
		return NULL;
	for (int i = 0; i < num && len < total_length; i ++) {
		memcpy(ret + len, nodes[i].ptr, nodes[i].len); 
		len += nodes[i].len;
	}
	if (len != total_length) {
		free(ret);
		return NULL;
	}
	clear_sections(nodes, num);
	return ret;
}

int check_section_header_version(uint8_t *pbuf, uint16_t bufsize, uint8_t cur_version) {
	if (unlikely(pbuf == NULL)) {
		return -1;
	}
	uint8_t *pdata = pbuf + 1;
	uint8_t section_syntax_indicator = TS_READ_BIT(pdata, 7);
	if (section_syntax_indicator == 0) {
		// not a section header
		return 0;
	}
	pdata += 4;
	uint8_t version = TS_READ8_BITS(pdata, 5, 1);
	if (cur_version == version) {
		// same version, continue process
		return 0;
	} else if (version > cur_version || 
			(cur_version == 0x1F && version != 0x1F)) {
		// new version
		return 1;
	}
	// older version ? skip
	return 2;

}

/* refer to wiki of psi, also see iso 13818-1 Table 2-30 */
/* return 0 when parse a full section done */
int parse_section_header(uint8_t *pbuf, uint16_t buf_size, struct table_header *ptable)
{
	if (unlikely(pbuf == NULL || ptable == NULL)) {
		return NULL_PTR;
	}
	uint16_t section_len = 0;
	uint8_t *pdata = pbuf;
	
	uint8_t tableid = TS_READ8(pdata);
	pdata += 1;
	/* A flag indicates if the syntax section follows the section length,
	 the PAT, PMT, and CAT all set this to 1 */
	uint8_t section_syntax_indicator = TS_READ_BIT(pdata, 7);

	// the PAT, PMT, and CAT all set this to 0, others set this to 1
	uint8_t private_bit = TS_READ_BIT(pdata, 6);

	//skip two bits for reserved

	/* section length  the first two bits of which shall be '00'.
	 The remaining 10 bits specify the number of bytes of the section, 
	 starting immediately following the section_length field, and 
	 including the CRC. The value in this field shall not exceed 1021 (0x3FD).*/
	section_len = TS_READ16(pdata) & 0x0FFF;
	pdata += 2;
	if (section_syntax_indicator == 1 && (section_len > 0x3FD)) {
		return INVALID_SEC_LEN;
	} else if (section_syntax_indicator == 0 && (section_len > 0xFFD)) {
		return INVALID_SEC_LEN;
	}
	
	if (section_syntax_indicator == 0) {
		ptable->section_syntax_indicator = 0;
		ptable->table_id = tableid;
		ptable->private_bit = private_bit;
		ptable->section_length = section_len;
		if (ptable->sections[0].ptr)
			free(ptable->sections[0].ptr);
		ptable->sections[0].len = buf_size - 3;
		ptable->sections[0].ptr = malloc(buf_size - 3);
		if (!ptable->sections[0].ptr) {
			return ENOMEM;
		}
		memcpy(ptable->sections[0].ptr, pdata, buf_size - 3);
		ptable->private_data_byte = ptable->sections[0].ptr;
	} else {

		uint8_t current_next_indicator;
		uint8_t version_num, last_sec, cur_sec;
		uint16_t tableid_ext;  //PAT uses this for tsid and PMT use this for program number

		tableid_ext = TS_READ16(pdata);
		pdata += 2;
		version_num = TS_READ8_BITS(pdata, 5, 1);
		current_next_indicator = TS_READ_BIT(pdata, 0);
		pdata += 1;
		cur_sec =  TS_READ8(pdata);
		pdata += 1;
		last_sec =  TS_READ8(pdata);
		pdata += 1;
		
		/* syntax section, table data. concat them if there are multiple sections */
		ptable->version_number = version_num;
		ptable->last_section_number = last_sec;
		ptable->table_id = tableid;
		ptable->private_bit = private_bit;
		ptable->section_syntax_indicator = section_syntax_indicator;

		ptable->section_length = section_len;
		ptable->table_id_ext = tableid_ext;
		ptable->current_next_indicator = current_next_indicator;

		//old data come again, ignore
		if (bitmap64_get(ptable->section_bitmap, cur_sec)) {
			return DUPLICATE_DATA;
		}
		bitmap64_set(ptable->section_bitmap, cur_sec);


		ptable->sections[cur_sec].len = buf_size - 3;
		ptable->sections[cur_sec].ptr = malloc(buf_size - 3);
		if (!ptable->sections[cur_sec].ptr) {
			return ENOMEM;
		}
		memcpy(ptable->sections[cur_sec].ptr, pdata, buf_size - 8);
		
		/*tell us buffering*/
		if(bitmap64_full(ptable->section_bitmap, last_sec) != 0)
			return 1;

		if (ptable->private_data_byte != NULL) {
			free(ptable->private_data_byte);
		}
		ptable->private_data_byte = concat_sections(ptable->sections, ptable->section_length,
				 ptable->last_section_number + 1);
	}
	return 0;
}

int parse_pat(uint8_t *pbuf, uint16_t buf_size)
{
	uint16_t section_len = 0;
	uint8_t *pdata = NULL;
	struct program_node *pn = NULL, *next = NULL;
	pat_t *p_pat = psi.pat;
	uint8_t cur_version = 0x1F;
	if (p_pat) {
		cur_version = p_pat->pat_header.version_number;
	}

	int ret = check_section_header_version(pbuf, buf_size, cur_version);
	if (ret == 1) {
		// new version, alloc new struct
		p_pat = calloc(1, sizeof(pat_t));
		if (!p_pat) {
			return ENOMEM;
		}
		list_head_init(&(p_pat->h));
		p_pat->pat_header.version_number = 0x1F;
		psi.pat = p_pat;
		list_add_tail(&psi.pat_list, &p_pat->n);
	} else if (ret != 0){
		// skip process sections
		return ret;
	}

	// else it is the sections for the same version, do concat
	ret = parse_section_header(pbuf, buf_size, &p_pat->pat_header);
	if (ret != 0) {
		return ret;
	}

	// TODO: limit program total length
	section_len = p_pat->pat_header.section_length;
	pdata = p_pat->pat_header.private_data_byte;

	section_len -= (5 + 4);
	while (section_len > 0) {
		uint16_t program_num, program_map_PID;

		program_num = TS_READ16(pdata);
		pdata += 2;
		program_map_PID = TS_READ16(pdata) & 0x1FFF;
		pdata += 2;
		section_len -= 4;
		if (program_num == 0xFFFF) {
			break;
		}
		if (p_pat->program_bitmap[program_num / 64] & ((uint64_t)1 << (program_num % 64))) {
			list_for_each_safe(&(p_pat->h), pn, next, n)
			{
				if (pn->program_number == program_num) {
					pn->program_map_PID = program_map_PID;
				}
			}
		} else {
			register_pmt_ops(program_map_PID);
			pn = calloc(1, sizeof(struct program_node));
			if (!pn) {
				return ENOMEM;
			}
			pn->program_number = program_num;
			pn->program_map_PID = program_map_PID;
			p_pat->program_bitmap[program_num / 64] |= ((uint64_t)1 << (program_num % 64));
			list_add_tail(&(p_pat->h), &(pn->n));
		}
	}

	return 0;
}

int parse_cat(uint8_t *pbuf, uint16_t buf_size)
{
	uint16_t section_len = 0;
	uint8_t *pdata = pbuf;
	cat_t *p_cat = psi.cat;
	uint8_t cur_version = 0x1F;
	if (p_cat) {
		cur_version = p_cat->cat_header.version_number;
	}

	int ret = check_section_header_version(pbuf, buf_size, cur_version);
	if (ret == 1) {
		// new version, alloc new struct
		p_cat = calloc(1, sizeof(cat_t));
		if (!p_cat) {
			return ENOMEM;
		}
		list_head_init(&(p_cat->list));
		p_cat->cat_header.version_number = 0x1F;
		psi.cat = p_cat;
		list_add_tail(&psi.cat_list, &p_cat->n);
	} else if (ret != 0){
		// skip process sections
		return ret;
	}

	ret = parse_section_header(pbuf, buf_size, &p_cat->cat_header);
	if (ret != 0)
		return ret;

	// // Transport Stream ID
	// ts_id = p_cat->cat_header.table_id_ext;
	section_len = p_cat->cat_header.section_length;

	pdata = p_cat->cat_header.private_data_byte;
	section_len -= (5 + 4);

	//clear descriptors
	if (!list_empty(&(p_cat->list))) {
		free_descriptors(&(p_cat->list));
	}
	
	parse_descriptors(&(p_cat->list), pdata, section_len);

	return 0;
}

int parse_tsdt(uint8_t *pbuf, uint16_t buf_size, tsdt_t *p_tsdt)
{
	uint16_t section_len = 0;
	uint8_t *pdata = pbuf;

	int ret = parse_section_header(pbuf, buf_size, &p_tsdt->tsdt_header);
	if (ret != 0)
		return ret;

	section_len = p_tsdt->tsdt_header.section_length;

	pdata = p_tsdt->tsdt_header.private_data_byte;

	section_len -= (5 + 4);

	if (!list_empty(&(p_tsdt->list))) {
		free_descriptors(&(p_tsdt->list));
	}

	parse_descriptors(&(p_tsdt->list), pdata, section_len);

	return 0;
}

int parse_pmt(uint8_t *pbuf, uint16_t buf_size, pmt_t *p_pmt)
{
	int16_t section_len = 0;
	uint8_t *pdata = NULL;
	struct es_node *pn = NULL, *next = NULL;

	int ret = parse_section_header(pbuf, buf_size, &p_pmt->pmt_header);
	if (ret != 0)
		return ret;

	if (!list_empty(&(p_pmt->h))) {
		list_for_each_safe(&(p_pmt->h), pn, next, n)
		{
			if (!list_empty(&(pn->list)))
				free_descriptors(&(pn->list));
			list_del(&(pn->n));
			free(pn);
		}
	}

	if (!list_empty(&(p_pmt->list)))
		free_descriptors(&(p_pmt->list));

	section_len = p_pmt->pmt_header.section_length;

	// Transport Stream ID
	p_pmt->program_number = p_pmt->pmt_header.table_id_ext;

	section_len -= 5 + 4;
	pdata = p_pmt->pmt_header.private_data_byte;

	p_pmt->PCR_PID = TS_READ16(pdata) & 0x1FFF;
	pdata += 2;
	section_len -= 2;
	p_pmt->program_info_length = TS_READ16(pdata) & 0x0FFF;
	pdata += 2;

	parse_descriptors(&(p_pmt->list), pdata, p_pmt->program_info_length);
	pdata += p_pmt->program_info_length;
	section_len -= 2 + p_pmt->program_info_length;

	while (section_len > 0) {
		pn = calloc(1, sizeof(struct es_node));
		if (!pn) {
			return ENOMEM;
		}
		list_head_init(&(pn->list));
		list_node_init(&(pn->n));
		pn->stream_type = TS_READ8(pdata);
		pdata += 1;
		pn->elementary_PID = TS_READ16(pdata) & 0x1FFF;
		pdata += 2;
		pn->ES_info_length = TS_READ16(pdata) & 0x0FFF;
		pdata += 2;
		parse_descriptors(&(pn->list), pdata, (int)pn->ES_info_length);
		if (pn->stream_type == STREAM_TYPE_MPEG2_SECTIONS) {
			register_section_ops(pn->elementary_PID, 0, NULL);
		} else if (pn->stream_type >= 0x08 && pn->stream_type <= 0x0D) {
			register_section_ops(pn->elementary_PID, 0, NULL);
		} else if (pn->stream_type == 0x86) {
			register_scte_ops(pn->elementary_PID);
		} else {
			register_pes_ops(pn->elementary_PID, pn->stream_type);
			if (has_descritpor_tag(&(pn->list), 0x59)) {
				register_pes_data_callback(pn->elementary_PID, pn->stream_type, parse_subtitle, 0x59);
			} else if (has_descritpor_tag(&(pn->list), 0x56)) {
				register_pes_data_callback(pn->elementary_PID, pn->stream_type, parse_teletext, 0x56);
			}
		}
		pdata += pn->ES_info_length;
		section_len -= (5 + pn->ES_info_length);
		list_add_tail(&(p_pmt->h), &(pn->n));
	}

	return 0;
}


static int pat_proc(__maybe_unused uint16_t pid, uint8_t *pkt, uint16_t len)
{
	psi.stats.pat_sections ++;
	parse_pat(pkt, len);
	return 0;
}

static int cat_proc(__maybe_unused uint16_t pid, uint8_t *pkt, uint16_t len)
{
	psi.stats.cat_sections ++;
	parse_cat(pkt, len);
	descriptor_t *ca = NULL;
	psi.ca_num = 0;
	list_for_each(&psi.cat->list, ca, n) {
		psi.ca_num ++;
	}
	return 0;
}

static int tsdt_proc(__maybe_unused uint16_t pid, uint8_t *pkt, uint16_t len)
{
	psi.stats.tsdt_sections++;
	parse_tsdt(pkt, len, &psi.tsdt);
	return 0;
}

static int pmt_proc(uint16_t pid, uint8_t *pkt, uint16_t len)
{
	parse_pmt(pkt, len, &(psi.pmt[pid]));
	return 0;
}

static int nit_proc(__maybe_unused uint16_t pid, uint8_t *pkt, uint16_t len)
{
	if(pkt[0] == NIT_ACTUAL_TID) {
		psi.stats.nit_actual_sections ++;
		parse_nit(pkt, len, &(psi.nit_actual));
	}else if(pkt[0] == NIT_OTHER_TID){
		psi.stats.nit_other_sections ++;
		parse_nit(pkt, len, &(psi.nit_other));
	}
	return 0;
}

static int sdt_bat_proc(__maybe_unused uint16_t pid, uint8_t *pkt, uint16_t len)
{
	switch (pkt[0]) {
	case BAT_TID:
		psi.stats.bat_sections ++;
		parse_bat(pkt, len, &(psi.bat));
		break;
	case SDT_ACTUAL_TID:
		psi.stats.sdt_actual_sections ++;
		parse_sdt(pkt, len, &(psi.sdt_actual));
		break;
	case SDT_OTHER_TID:
		psi.stats.sdt_other_sections ++;
		parse_sdt(pkt, len, &(psi.sdt_other));
		break;
	default:
		break;
	}
	return 0;
}

static int eit_proc(__maybe_unused uint16_t pid, uint8_t *pkt, uint16_t len)
{
	switch (pkt[0]) {
	case EIT_ACTUAL_TID:
		psi.stats.eit_actual_sections ++;
		parse_eit(pkt, len, &psi.eit_actual);
		break;
	case EIT_OTHER_TID:
		psi.stats.eit_other_sections ++;
		parse_eit(pkt, len, &psi.eit_other);
		break;
	default:
		break;
	}
	return 0;
}

static int tdt_tot_proc(__maybe_unused uint16_t pid, uint8_t *pkt, uint16_t len)
{
	switch (pkt[0]) {
	case TDT_TID:
		psi.stats.tdt_sections++;
		parse_tdt(pkt, len, &psi.tdt);
		break;
	case TOT_TID:
		psi.stats.tot_sections++;
		parse_tot(pkt, len, &psi.tot);
		break;
	}
	return 0;
}


static int default_proc(uint16_t pid, uint8_t *pkt, uint16_t len)
{
	switch (pid)
	{
		case PAT_PID:
			pat_proc(pid, pkt, len);
			break;
		case CAT_PID:
			cat_proc(pid, pkt, len);
			break;
		case NIT_PID:
			nit_proc(pid, pkt, len);
			break;
		case EIT_PID:
			eit_proc(pid, pkt, len);
			break;
		case SDT_PID:
			sdt_bat_proc(pid, pkt, len);
			break;
		case TDT_PID:
			tdt_tot_proc(pid, pkt, len);
			break;
		default:
			break;
	}
	return 0;
}

void init_table_ops(void)
{
	struct tsa_config *tsaconf = get_config();
	int pid = 0;
	psi_table_init();
	for (int i = 0; i < TS_MAX_PID; i ++) {
		if (tsaconf->pids[i] == 1) {
			pid = 1;
			init_table_filter(i, 0, 0, default_proc);
		}
	}
	if (pid == 1)
		return;

	init_table_filter(PAT_PID, PAT_TID, 0xFF, pat_proc);
	init_table_filter(CAT_PID, CAT_TID, 0xFF, cat_proc);
	init_table_filter(TSDT_PID, TSDT_TID, 0xFF, tsdt_proc);

	//filter nit actual and other at same time
	init_table_filter(NIT_PID, NIT_ACTUAL_TID, 0xFE, nit_proc);
	//filter eit actual and other at same time
	init_table_filter(EIT_PID, EIT_ACTUAL_TID, 0xFE, eit_proc);

	init_table_filter(SDT_PID, SDT_ACTUAL_TID, 0xFF, sdt_bat_proc);
	init_table_filter(SDT_PID, SDT_OTHER_TID, 0xFF, sdt_bat_proc);
	init_table_filter(BAT_PID, BAT_TID, 0xFF, sdt_bat_proc);

	/*filter tdt and tot at same time*/
	init_table_filter(TDT_PID, TDT_TID, 0xFF, tdt_tot_proc);
	init_table_filter(TOT_PID, TOT_TID, 0xFF, tdt_tot_proc);

	/* All PSIP tables share PID 0x1FFB.  Each table has its own table_id, so
	 * register them individually (the filter dispatches O(1) on table_id). */
	init_table_filter(MGT_PID, MGT_TID, 0xFF, atsc_psip_proc);
	init_table_filter(MGT_PID, TVCT_TID, 0xFF, atsc_psip_proc);
	init_table_filter(MGT_PID, CVCT_TID, 0xFF, atsc_psip_proc);
	init_table_filter(MGT_PID, RRT_TID, 0xFF, atsc_psip_proc);
	init_table_filter(MGT_PID, ETT_TID, 0xFF, atsc_psip_proc);
	init_table_filter(MGT_PID, EIT_TID, 0xFF, atsc_psip_proc);
	init_table_filter(MGT_PID, STT_TID, 0xFF, atsc_psip_proc);
	init_table_filter(MGT_PID, DCCT_TID, 0xFF, atsc_psip_proc);
	init_table_filter(MGT_PID, DCCSCT_TID, 0xFF, atsc_psip_proc);
	/* PID 0x1FFB is not < 0x20, so mark it as a section PID so the demux
	 * reassembles sections on it (all PSIP tables share this PID). */
	psi.section_bitmap[MGT_PID / 64] |= ((uint64_t)1 << (MGT_PID % 64));
}

void uninit_table_ops(void)
{
	uninit_table_filter(PAT_PID, PAT_TID, 0xFF);
	uninit_table_filter(CAT_PID, CAT_TID, 0xFF);
	uninit_table_filter(TSDT_PID, TSDT_TID, 0xFF);
	uninit_table_filter(NIT_PID, NIT_ACTUAL_TID, 0xFE);
	uninit_table_filter(EIT_PID, EIT_ACTUAL_TID, 0xFE);
	uninit_table_filter(SDT_PID, SDT_ACTUAL_TID, 0xFF);
	uninit_table_filter(SDT_PID, SDT_OTHER_TID, 0xFF);
	uninit_table_filter(BAT_PID, BAT_TID, 0xFF);
	uninit_table_filter(TDT_PID, TDT_TID, 0xFF);
	uninit_table_filter(TOT_PID, TOT_TID, 0xFF);

	uninit_table_filter(MGT_PID, MGT_TID, 0xFF);
	uninit_table_filter(MGT_PID, TVCT_TID, 0xFF);
	uninit_table_filter(MGT_PID, CVCT_TID, 0xFF);
	uninit_table_filter(MGT_PID, RRT_TID, 0xFF);
	uninit_table_filter(MGT_PID, ETT_TID, 0xFF);
	uninit_table_filter(MGT_PID, EIT_TID, 0xFF);
	uninit_table_filter(MGT_PID, STT_TID, 0xFF);
	uninit_table_filter(MGT_PID, DCCT_TID, 0xFF);
	uninit_table_filter(MGT_PID, DCCSCT_TID, 0xFF);
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


void register_section_ops(uint16_t pid, uint8_t tableid, filter_cb callback)
{
	if (pid == NIT_PID)
		return;
	if ((psi.section_bitmap[pid / 64] & ((uint64_t)1 << (pid % 64))) == 0) {
		psi.section_bitmap[pid / 64] |= ((uint64_t)1 << (pid % 64));
		if (callback) {
			init_table_filter(pid, tableid, 0xFF, callback);
		} else {
			init_table_filter(pid, tableid, 0xFF, default_proc);
		}
		
	}
}

void unregister_section_ops(uint16_t pid)
{
	psi.section_bitmap[pid / 64] &= ~((uint64_t)1 << (pid % 64));
	uninit_table_filter(pid, 0, 0);
}

bool check_section_pid(uint16_t pid)
{
	//other sections like DSM-CC
	if (psi.section_bitmap[pid / 64] & ((uint64_t)1 << (pid % 64)))
		return true;
	return false;
}
