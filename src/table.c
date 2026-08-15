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
#include "ts.h"
#include "utils.h"
#include "result.h"
#include "subtitle.h"
#include "teletext.h"

static mpeg_psi_t psi;
static atsc_psip_t psip;

/* marks which PSIP tables were successfully parsed (see psip_proc) */
#define PSIP_MGT   (1 << 0)
#define PSIP_TVCT  (1 << 1)
#define PSIP_CVCT  (1 << 2)
#define PSIP_RRT   (1 << 3)
#define PSIP_EIT   (1 << 4)
#define PSIP_ETT   (1 << 5)
#define PSIP_STT   (1 << 6)
#define PSIP_DCCT  (1 << 7)
#define PSIP_DCCSCT (1 << 8)
static uint16_t psip_seen_mask;


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

	/* ATSC PSIP table lists */
	memset(&psip, 0, sizeof(psip));
	list_head_init(&psip.mgt.list);
	list_head_init(&psip.tvct.list);
	list_head_init(&psip.cvct.list);
	list_head_init(&psip.rrt.list);
	list_head_init(&psip.stt.list);
	list_head_init(&psip.dcct.list);
	list_head_init(&psip.dccsct.list);
	return 0;
}


static void dump_section_header(const char *table_name, struct table_header *hdr)
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

static void dump_tdt(tdt_t *p_tdt)
{
	char utc[20];
	rout(0, "TDT (Time and Date Table)", NULL);
	convert_UTC(&p_tdt->utc_time, utc, sizeof(utc));
	rout(1, "UTC time", "%s", utc);
}

static void dump_tot(tot_t *p_tot)
{
	char utc[20];
	rout(0, "TOT (Time Offset Table)", NULL);
	convert_UTC(&p_tot->utc_time, utc, sizeof(utc));
	rout(1, "UTC time", "%s", utc);

	dump_descriptors(2, &(p_tot->list));
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

static void dump_sdt(sdt_t *p_sdt)
{
	if (p_sdt == NULL)
		return;
	struct service_node *pn = NULL;
	char sdt_name[32] = {0};
	if(p_sdt->sdt_header.table_id == SDT_ACTUAL_TID)
		snprintf(sdt_name, 32, "SDT ACTUAL tid 0x%x", SDT_ACTUAL_TID);
	else if (p_sdt->sdt_header.table_id == SDT_OTHER_TID)
		snprintf(sdt_name, 32, "SDT OTHER tid 0x%x", SDT_OTHER_TID);
	rout(0, sdt_name, NULL);
	rout(1, "transport_stream_id", "0x%x", p_sdt->sdt_header.table_id_ext);
	rout(1, "section_length", "%d", p_sdt->sdt_header.section_length);
	rout(1, "version_number", "%d", p_sdt->sdt_header.version_number);
	rout(1, "Current next", "%s", p_sdt->sdt_header.current_next_indicator ? "yes" : "no");
	rout(1, "original_network_id", "0x%x", p_sdt->original_network_id);
	if (!list_empty(&(p_sdt->h)))
	{
		list_for_each(&(p_sdt->h), pn, n)
		{
			rout(2, "service_id", "0x%04x(%d) ", pn->service_id, pn->service_id);
			rout(3, "EIT_schedule_flag", "0x%x ", pn->EIT_schedule_flag);
			rout(3, "EIT_present_following_flag", "0x%x ", pn->EIT_present_following_flag);
			rout(3, "running_status", "0x%x", pn->running_status);
			rout(3, "free_CA_mode", "0x%x", pn->free_CA_mode);
			dump_descriptors(4, &(pn->list));
		}
	}
}

static void dump_bat(bat_t *p_bat)
{
	if (p_bat == NULL)
		return;
	struct transport_stream_node *pn = NULL;

	rout(0, "BAT", NULL);
	rout(1, "section_length", "%d", p_bat->bat_header.section_length);
	rout(1, "bouquet_id", "0x%x", p_bat->bouquet_id);
	rout(1, "version_number", " %d", p_bat->bat_header.version_number);
	rout(1, "Current next", " %s", p_bat->bat_header.current_next_indicator ? "yes" : "no");
	rout(1, "bouquet descriptor length", " %d", p_bat->bouquet_descriptors_length );
	dump_descriptors(2, &(p_bat->list));
	if(p_bat->transport_stream_loop_length)
	{
		rout(1, "transport_streams", NULL);
		list_for_each(&(p_bat->h), pn, n)
		{
			rout(2, NULL, "0x%04x(%d) ", pn->transport_stream_id, pn->transport_stream_id);
			rout(3, "original_network_id", "%x ", pn->original_network_id);
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
	char nit_name[32] = {0};

	if (p_nit->nit_header.table_id == NIT_ACTUAL_TID)
		snprintf(nit_name, 32,  "NIT ACTUAL tid 0x%x", p_nit->nit_header.table_id);
	if (p_nit->nit_header.table_id == NIT_OTHER_TID)
		snprintf(nit_name, 32,  "NIT OTHER tid 0x%x", p_nit->nit_header.table_id);
	rout(0, nit_name, NULL);
	rout(1, "section_length", " %d", p_nit->nit_header.section_length);
	rout(1, "network_id", " 0x%x", p_nit->network_id);
	rout(1, "version_number", " %d", p_nit->nit_header.version_number);
	rout(1, "Current next", " %s", p_nit->nit_header.current_next_indicator ? "yes" : "no");
	dump_descriptors(2, &(p_nit->list));
	rout(2, "transport_stream ", NULL);
	list_for_each(&(p_nit->h), pn, n)
	{
		rout(3, "transport_stream_id", "0x%x ", pn->transport_stream_id);
		rout(4, "original_network_id", "0x%x ", pn->original_network_id);
		dump_descriptors(5, &(pn->list));
	}
}

static void dump_eit(eit_t *p_eit)
{
	struct event_node *pn = NULL;
	dump_section_header("EIT", &p_eit->eit_header);
	rout(1, "transport_stream_id", " 0x%x", p_eit->transport_stream_id);
	rout(1, "original_network_id", " 0x%x", p_eit->original_network_id);
	rout(1, "segment_last_section_number", " 0x%x", p_eit->segment_last_section_number);
	rout(1, "events", NULL);
	list_for_each(&(p_eit->h), pn, n)
	{
		rout(2, "event_id", " 0x%x ", pn->event_id);
		rout(2, "start_time", "0x%x ", pn->start_time);
		rout(2, "duration", " 0x%x ", pn->duration);
		rout(2, "running_status", "%d ", pn->running_status);
		rout(2, "free_CA_mode", "%d ", pn->free_CA_mode);
		dump_descriptors(3, &(pn->list));
	}
}

/* ---- ATSC PSIP table dumping (A/65) ---- */

static void dump_mgt(atsc_mgt_t *mgt)
{
	dump_section_header("MGT (Master Guide Table)", &mgt->mgt_header);
	rout(1, "protocol_version", "%d", mgt->protocol_version);
	rout(1, "tables_defined", "%d", mgt->tables_defined);
	for (int i = 0; i < mgt->tables_defined; i++) {
		rout(2, "table_type", "0x%04x", mgt->tables[i].table_type);
		rout(2, "table_type_PID", "0x%x", mgt->tables[i].table_type_PID);
		rout(2, "version_number", "%d", mgt->tables[i].table_type_version_number);
		rout(2, "number_bytes", "%u", mgt->tables[i].number_bytes);
		if (!list_empty(&mgt->tables[i].list))
			dump_descriptors(3, &mgt->tables[i].list);
	}
	if (!list_empty(&mgt->list))
		dump_descriptors(1, &mgt->list);
}

static void dump_vct(const char *name, atsc_vct_t *vct)
{
	dump_section_header(name, &vct->vct_header);
	rout(1, "protocol_version", "%d", vct->protocol_version);
	rout(1, "num_channels_in_section", "%d", vct->num_channels_in_section);
	for (int i = 0; i < vct->num_channels_in_section; i++) {
		rout(2, "channel", NULL);
		rout(3, "major_channel_number", "%d", vct->channels[i].major_channel_number);
		rout(3, "minor_channel_number", "%d", vct->channels[i].minor_channel_number);
		rout(3, "modulation_mode", "%d", vct->channels[i].modulation_mode);
		rout(3, "program_number", "%d", vct->channels[i].program_number);
		rout(3, "service_type", "%d", vct->channels[i].service_type);
		rout(3, "source_id", "%d", vct->channels[i].source_id);
		if (!list_empty(&vct->channels[i].list))
			dump_descriptors(4, &vct->channels[i].list);
	}
	if (!list_empty(&vct->list))
		dump_descriptors(1, &vct->list);
}

static void dump_rrt(atsc_rrt_t *rrt)
{
	dump_section_header("RRT (Rating Region Table)", &rrt->rrt_header);
	rout(1, "protocol_version", "%d", rrt->protocol_version);
	rout(1, "dimensions_defined", "%d", rrt->dimensions_defined);
	if (!list_empty(&rrt->list))
		dump_descriptors(1, &rrt->list);
}

static void dump_ett(atsc_ett_t *ett)
{
	dump_section_header("ETT (Extended Text Table)", &ett->ett_header);
	rout(1, "protocol_version", "%d", ett->protocol_version);
	rout(1, "ETM_id", "0x%x", ett->ETM_id);
}

static void dump_atsc_eit(atsc_eit_t *eit)
{
	dump_section_header("ATSC EIT (Event Information Table)", &eit->eit_header);
	rout(1, "protocol_version", "%d", eit->protocol_version);
	rout(1, "num_events_in_section", "%d", eit->num_events_in_section);
	for (int i = 0; i < eit->num_events_in_section; i++) {
		rout(2, "event_id", "0x%x", eit->events[i].event_id);
		rout(2, "start_time", "%u", eit->events[i].start_time);
		rout(2, "length_in_seconds", "%u", eit->events[i].length_in_seconds);
		if (!list_empty(&eit->events[i].list))
			dump_descriptors(3, &eit->events[i].list);
	}
}

static void dump_stt(atsc_stt_t *stt)
{
	dump_section_header("STT (System Time Table)", &stt->stt_header);
	rout(1, "protocol_version", "%d", stt->protocol_version);
	rout(1, "system_time", "%u", stt->system_time);
	rout(1, "GPS_UTC_offset", "%d", stt->GPS_UTC_offset);
	rout(1, "daylight_saving", "0x%x", stt->daylight_saving);
	if (!list_empty(&stt->list))
		dump_descriptors(1, &stt->list);
}

static void dump_dcct(atsc_dcct_t *dcct)
{
	dump_section_header("DCCT (Directed Channel Change Table)", &dcct->dcct_header);
	rout(1, "protocol_version", "%d", dcct->protocol_version);
	rout(1, "dcc_test_count", "%d", dcct->dcc_test_count);
	if (!list_empty(&dcct->list))
		dump_descriptors(1, &dcct->list);
}

static void dump_dccsct(atsc_dccsct_t *dccsct)
{
	dump_section_header("DCCSCT (Directed Channel Change Selection Table)", &dccsct->dccsct_header);
	rout(1, "protocol_version", "%d", dccsct->protocol_version);
	rout(1, "updates_defined", "%d", dccsct->updates_defined);
	if (!list_empty(&dccsct->list))
		dump_descriptors(1, &dccsct->list);
}

static int psip_seen(void)
{
	return psip_seen_mask != 0;
}

static void dump_psip(void)
{
	if (psip_seen_mask & PSIP_MGT)
		dump_mgt(&psip.mgt);
	if (psip_seen_mask & PSIP_TVCT)
		dump_vct("TVCT (Terrestrial Virtual Channel Table)", &psip.tvct);
	if (psip_seen_mask & PSIP_CVCT)
		dump_vct("CVCT (Cable Virtual Channel Table)", &psip.cvct);
	if (psip_seen_mask & PSIP_RRT)
		dump_rrt(&psip.rrt);
	if (psip_seen_mask & PSIP_EIT)
		dump_atsc_eit(&psip.eit);
	if (psip_seen_mask & PSIP_ETT)
		dump_ett(&psip.ett);
	if (psip_seen_mask & PSIP_STT)
		dump_stt(&psip.stt);
	if (psip_seen_mask & PSIP_DCCT)
		dump_dcct(&psip.dcct);
	if (psip_seen_mask & PSIP_DCCSCT)
		dump_dccsct(&psip.dccsct);
}

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

	if (psip_seen() && (tsaconf->tables & PSIP_SHOW))
		dump_psip();

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

int parse_nit(uint8_t *pbuf, uint16_t buf_size, nit_t *p_nit)
{
	int16_t section_len = 0;
	uint8_t *pdata = NULL;
	struct transport_stream_node *pn = NULL, *next = NULL;

	int ret = parse_section_header(pbuf, buf_size, &p_nit->nit_header);
	if (ret != 0)
		return ret;

	section_len = p_nit->nit_header.section_length;

	pdata = p_nit->nit_header.private_data_byte;

	section_len -= (5 + 4);

	if (!list_empty(&(p_nit->list)))
		free_descriptors(&(p_nit->list));

	if (!list_empty(&(p_nit->h))) {
		list_for_each_safe(&(p_nit->h), pn, next, n)
		{
			list_del(&(pn->n));
			free(pn);
		}
	}

	p_nit->network_descriptors_length = TS_READ16(pdata) & 0xFFF;	
	pdata += 2;
	parse_descriptors(&(p_nit->list), pdata, (int)p_nit->network_descriptors_length);
	pdata += p_nit->network_descriptors_length;
	p_nit->transport_stream_loop_length = TS_READ16(pdata) & 0xFFF;
	pdata += 2;
	section_len -= 4 + p_nit->network_descriptors_length;;

	while (section_len > 0) {
		pn = calloc(1, sizeof(struct transport_stream_node));
		if (!pn) {
			return ENOMEM;
		}
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
		list_add_tail(&(p_nit->h), &(pn->n));
	}
	return 0;
}

int parse_bat(uint8_t *pbuf, uint16_t buf_size, bat_t *p_bat)
{
	int16_t section_len = 0;
	uint8_t *pdata = NULL;
	struct transport_stream_node *pn = NULL, *next = NULL;

	int ret = parse_section_header(pbuf, buf_size, &p_bat->bat_header);
	if (ret != 0)
		return ret;

	section_len = p_bat->bat_header.section_length;
	p_bat->bouquet_id = p_bat->bat_header.table_id;
	pdata = p_bat->bat_header.private_data_byte;

	section_len -= 5 + 4;

	if (!list_empty(&(p_bat->h))) {
		list_for_each_safe(&(p_bat->h), pn, next, n)
		{
			list_del(&(pn->n));
			free(pn);
		}
	}	
	if (!list_empty(&(p_bat->list)))
		free_descriptors(&(p_bat->list));

	p_bat->bouquet_descriptors_length = TS_READ16(pdata) & 0xFFF;
	pdata += 2;
	parse_descriptors(&(p_bat->list), pdata, p_bat->bouquet_descriptors_length);
	pdata += p_bat->bouquet_descriptors_length;
	section_len -= 2;
	section_len -= p_bat->bouquet_descriptors_length;
	p_bat->transport_stream_loop_length = TS_READ16(pdata) & 0xFFF;
	pdata += 2;
	section_len -= 2;
	while (section_len > 0) {
		pn = calloc(1, sizeof(struct transport_stream_node));
		if (!pn) {
			return ENOMEM;
		}
		list_head_init(&(pn->list));
		list_node_init(&(pn->n));
		pn->transport_stream_id = TS_READ16(pdata);
		pdata += 2;
		pn->original_network_id = TS_READ16(pdata);
		pdata += 2;
		pn->transport_descriptors_length = TS_READ16(pdata) & 0xFFF;
		pdata += 2;
		parse_descriptors(&(pn->list), pdata, pn->transport_descriptors_length);
		pdata += pn->transport_descriptors_length;
		section_len -= (6 + pn->transport_descriptors_length);
		list_add_tail(&(p_bat->h), &(pn->n));
	}

	return 0;
}

int parse_sdt(uint8_t *pbuf, uint16_t buf_size, sdt_t *p_sdt)
{
	int16_t section_len = 0;
	uint8_t *pdata = NULL;
	struct service_node *pn = NULL, *next = NULL;

	int ret = parse_section_header(pbuf, buf_size, &p_sdt->sdt_header);
	if (ret != 0)
		return ret;

	section_len = p_sdt->sdt_header.section_length;
	pdata = p_sdt->sdt_header.private_data_byte;

	section_len -= (5 + 4);

	if (!list_empty(&(p_sdt->h))) {
		list_for_each_safe(&(p_sdt->h), pn, next, n)
		{
			list_del(&(pn->n));
			if (!list_empty(&(pn->list))) {
				free_descriptors(&(pn->list));
			}
			free(pn);
		}
	}

	p_sdt->original_network_id = TS_READ16(pdata);
	pdata += 3;
	section_len -= 3;
	
	while (section_len > 0) {
		pn = calloc(1, sizeof(struct service_node));
		if (!pn) {
			return ENOMEM;
		}
		list_head_init(&(pn->list));
		list_node_init(&(pn->n));
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
		section_len -= (5 + pn->descriptors_loop_length);
		list_add_tail(&(p_sdt->h), &(pn->n));
	}

	return 0;
}

static int parse_eit(uint8_t *pbuf, uint16_t buf_size, eit_t *p_eit)
{
	uint16_t section_len = 0;
	uint8_t *pdata = NULL;
	struct event_node *pn = NULL;

	int ret = parse_section_header(pbuf, buf_size, &p_eit->eit_header);
	if (ret != 0)
		return ret;


	pdata = p_eit->eit_header.private_data_byte;
	section_len = p_eit->eit_header.section_length;

	section_len -= (5 + 4);
	p_eit->transport_stream_id = TS_READ16(pdata);
	pdata += 2;
	p_eit->original_network_id = TS_READ16(pdata);
	pdata += 2;
	p_eit->segment_last_section_number = TS_READ8(pdata);
	pdata += 1;
	p_eit->last_table_id = TS_READ8(pdata);
	pdata += 1;
	section_len -= 6;
	while (section_len > 0) {
		pn = calloc(1, sizeof(struct event_node));
		if (!pn) {
			return ENOMEM;
		}
		list_head_init(&(pn->list));
		list_node_init(&(pn->n));
		pn->event_id = TS_READ16(pdata);
		pdata += 2;
		pn->start_time = TS_READ64_BITS(pdata, 40, 0);
		pn->duration = TS_READ64_BITS(pdata, 24, 40);
		pdata += 8;
		pn->running_status = TS_READ16_BITS(pdata, 3, 0);
		pn->free_CA_mode = TS_READ16_BITS(pdata, 1, 3);
		pn->descriptors_loop_length = TS_READ16_BITS(pdata, 12, 4);
		pdata += 2;
		fprintf(stderr, "DBG eit: event_id=%d desc_len=%d section_len=%d buf_size=%d\n", pn->event_id, pn->descriptors_loop_length, section_len, buf_size);
		for (int di = 0; di < 45 && di < buf_size; di++) fprintf(stderr, "%02x ", pdata[di]);
		fprintf(stderr, "\n");
		parse_descriptors(&(pn->list), pdata, (int)(pn->descriptors_loop_length));
		section_len -= (12 + pn->descriptors_loop_length);;
		list_add_tail(&(p_eit->h), &(pn->n));
	}

	return 0;
}

static int parse_tdt(uint8_t *pbuf, uint16_t buf_size, tdt_t *p_tdt)
{
	uint16_t section_len = 0;
	uint8_t *pdata = pbuf;

	if (unlikely(pbuf == NULL || p_tdt == NULL)) {
		return -1;
	}

	if (unlikely(pdata[0] != TDT_TID)) {
		return -1;
	}

	pdata += 1;
	section_len = TS_READ16(pdata) & 0xFFF;
	p_tdt->section_length = section_len;
	pdata += 2;
	memcpy(&p_tdt->utc_time, pdata, 5);
	return 0;
}

static int parse_tot(uint8_t *pbuf, uint16_t buf_size, tot_t *p_tot)
{
	uint8_t *pdata = pbuf;

	if (unlikely(pbuf == NULL || p_tot == NULL)) {
		return -1;
	}

	if (unlikely(pdata[0] != TOT_TID)) {
		return -1;
	}

	pdata += 1;
	p_tot->section_length = TS_READ16(pdata) & 0xFFF;
	pdata += 2;
	memcpy(&p_tot->utc_time, pdata, 5);
	pdata += 5;
	p_tot->descriptors_loop_length = TS_READ16(pdata) & 0xFFF;
	pdata += 2;
	if (!list_empty(&(p_tot->list)))
		free_descriptors(&(p_tot->list));
	parse_descriptors(&(p_tot->list), pdata, (int)p_tot->descriptors_loop_length);
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


void free_multi_string(struct multiple_string *str)
{
	for (int i = 0; i < str->number_strings; i ++) {
		for (int j = 0; j < str->strings[i].number_segments; j++) {
			free(str->strings[i].segments[j].compressed_string_byte);
		}
		free(str->strings[i].segments);
	}
	free(str->strings);
}

int parse_multi_string(uint8_t *pbuf, struct multiple_string *str)
{
	uint8_t *pdata = pbuf;
	str->number_strings = TS_READ8(pdata);
	pdata += 1;
	str->strings = calloc(str->number_strings, sizeof(struct lang_string));
	if (!str->strings) {
		return ENOMEM;
	}
	for (int i = 0; i < str->number_strings; i ++) {
		str->strings[i].ISO_639_language_code = TS_READ32_BITS(pdata, 24, 0);
		str->strings[i].number_segments = TS_READ32_BITS(pdata, 8, 24);
		pdata += 4;
		str->strings[i].segments = calloc(str->strings[i].number_segments, sizeof(struct string_segment));
		if (!str->strings[i].segments) {
			return ENOMEM;
		}
		for (int j =0; j < str->strings[i].number_segments; j ++) {
			str->strings[i].segments[j].compression_type = TS_READ8(pdata);
			pdata += 1;
			str->strings[i].segments[j].mode = TS_READ8(pdata);
			pdata += 1;
			str->strings[i].segments[j].number_bytes = TS_READ8(pdata);
			pdata += 1;
			str->strings[i].segments[j].compressed_string_byte = calloc(1, str->strings[i].segments[j].number_bytes);
			if (!str->strings[i].segments[j].compressed_string_byte) {
				return ENOMEM;
			}
			memcpy(str->strings[i].segments[j].compressed_string_byte, pdata, str->strings[i].segments[j].number_bytes);
			pdata += str->strings[i].segments[j].number_bytes;
		}
	}
	return pdata - pbuf;
}

static int parse_mgt(uint8_t *pbuf, uint16_t buf_size, atsc_mgt_t *mgt)
{
	uint8_t *pdata = pbuf;

	int ret = parse_section_header(pbuf, buf_size, &mgt->mgt_header);
	if (ret != 0)
		return ret;

	if (mgt->tables) {
		for (int i = 0; i < mgt->tables_defined; i++) {
			if (!list_empty(&mgt->tables[i].list))
				free_descriptors(&mgt->tables[i].list);
		}
		free(mgt->tables);
	}
	if (!list_empty(&mgt->list))
		free_descriptors(&mgt->list);

	pdata = mgt->mgt_header.private_data_byte;
	mgt->protocol_version = TS_READ8(pdata);
	pdata += 1;
	mgt->tables_defined = TS_READ16(pdata);
	pdata += 2;
	mgt->tables = calloc(mgt->tables_defined, sizeof(struct define_table));
	if (!mgt->tables) {
		return ENOMEM;
	}
	for (int i= 0; i < mgt->tables_defined; i ++) {
		mgt->tables[i].table_type = TS_READ16(pdata);
		pdata += 2;
		mgt->tables[i].table_type_PID = TS_READ16_BITS(pdata, 13, 3);
		pdata += 2;
		mgt->tables[i].table_type_version_number = TS_READ8_BITS(pdata, 5, 3);
		pdata += 1;
		mgt->tables[i].number_bytes = TS_READ32(pdata);
		pdata += 4;
		mgt->tables[i].table_type_descriptors_length = TS_READ16_BITS(pdata, 12, 4);
		pdata += 2;
		list_head_init(&mgt->tables[i].list);
		parse_descriptors(&mgt->tables[i].list, pdata, mgt->tables[i].table_type_descriptors_length);
		pdata += mgt->tables[i].table_type_descriptors_length;
	}
	mgt->descriptors_length = TS_READ16_BITS(pdata, 12, 4);
	pdata += 2;
	list_head_init(&mgt->list);
	parse_descriptors(&(mgt->list), pdata, mgt->descriptors_length);
	return 0;
}

static int parse_tvct(uint8_t *pbuf, uint16_t buf_size, atsc_vct_t *tvct)
{
	uint8_t *pdata = pbuf;

	int ret = parse_section_header(pbuf, buf_size, &tvct->vct_header);
	if (ret != 0)
		return ret;

	if (tvct->channels) {
		for (int i = 0; i < tvct->num_channels_in_section; i ++) {
			if (!list_empty(&tvct->channels[i].list)) {
				free_descriptors(&tvct->channels[i].list);
			}
		}
		free(tvct->channels);
	}
	if (!list_empty(&tvct->list))
		free_descriptors(&tvct->list);

	pdata = tvct->vct_header.private_data_byte;
	tvct->protocol_version = TS_READ8(pdata);
	pdata += 1;
	tvct->num_channels_in_section = TS_READ8(pdata);
	pdata += 1;
	tvct->channels = calloc(tvct->num_channels_in_section, sizeof(struct define_channel));
	if (!tvct->channels) {
		return ENOMEM;
	}
	for (int i = 0; i < tvct->num_channels_in_section; i ++) {
		memcpy(tvct->channels[i].short_name, pdata, 7 * sizeof(uint16_t));
		pdata += 7 * sizeof(uint16_t);
		tvct->channels[i].major_channel_number = TS_READ32_BITS(pdata, 10, 4);
		tvct->channels[i].minor_channel_number = TS_READ32_BITS(pdata, 10, 14);
		tvct->channels[i].modulation_mode = TS_READ32_BITS(pdata, 8, 24);
		pdata += 4;
		tvct->channels[i].carrier_frequency = TS_READ32(pdata);
		pdata += 4;
		tvct->channels[i].channel_TSID = TS_READ16(pdata);
		pdata += 2;
		tvct->channels[i].program_number = TS_READ16(pdata);
		pdata += 2;
		tvct->channels[i].ETM_location = TS_READ16_BITS(pdata, 2, 0);
		tvct->channels[i].access_controlled = TS_READ16_BITS(pdata, 1, 2);
		tvct->channels[i].hidden = TS_READ16_BITS(pdata, 1, 3);
		tvct->channels[i].hide_guide = TS_READ16_BITS(pdata, 1, 6);
		tvct->channels[i].service_type = TS_READ16_BITS(pdata, 6, 10);
		pdata += 2;
		tvct->channels[i].source_id = TS_READ16(pdata);
		pdata += 2;
		tvct->channels[i].descriptors_length = TS_READ16_BITS(pdata, 10, 6);
		pdata += 2;
		list_head_init(&tvct->channels[i].list);
		parse_descriptors(&tvct->channels[i].list, pdata, tvct->channels[i].descriptors_length);
		pdata += tvct->channels[i].descriptors_length;
	}
	tvct->additional_descriptors_length = TS_READ16_BITS(pdata, 10, 6);
	pdata += 2;
	list_head_init(&tvct->list);
	parse_descriptors(&tvct->list, pdata, tvct->additional_descriptors_length);
	pdata += tvct->additional_descriptors_length;
	return 0;
}

static int parse_cvct(uint8_t *pbuf, uint16_t buf_size, atsc_vct_t *cvct)
{
	uint8_t *pdata = pbuf;

	int ret = parse_section_header(pbuf, buf_size, &cvct->vct_header);
	if (ret != 0)
		return ret;

	if (cvct->channels) {
		for (int i = 0; i < cvct->num_channels_in_section; i ++) {
			if (!list_empty(&cvct->channels[i].list)) {
				free_descriptors(&cvct->channels[i].list);
			}
		}
		free(cvct->channels);
	}
	if (!list_empty(&cvct->list))
		free_descriptors(&cvct->list);

	pdata = cvct->vct_header.private_data_byte;
	cvct->protocol_version = TS_READ8(pdata);
	pdata += 1;

	cvct->num_channels_in_section = TS_READ8(pdata);
	pdata += 1;
	cvct->channels = calloc(cvct->num_channels_in_section, sizeof(struct define_channel));
	if (!cvct->channels) {
		return ENOMEM;
	}
	for (int i = 0; i < cvct->num_channels_in_section; i ++) {
		memcpy(cvct->channels[i].short_name, pdata, 7*16);
		pdata += 7*16;
		cvct->channels[i].major_channel_number = TS_READ32_BITS(pdata, 10, 4);
		cvct->channels[i].minor_channel_number = TS_READ32_BITS(pdata, 10, 14);
		cvct->channels[i].modulation_mode = TS_READ32_BITS(pdata, 8, 24);
		pdata += 4;
		cvct->channels[i].carrier_frequency = TS_READ32(pdata);
		pdata += 4;
		cvct->channels[i].channel_TSID = TS_READ16(pdata);
		pdata += 2;
		cvct->channels[i].program_number = TS_READ16(pdata);
		pdata += 2;
		cvct->channels[i].ETM_location = TS_READ16_BITS(pdata, 2, 0);
		cvct->channels[i].access_controlled = TS_READ16_BITS(pdata, 1, 2);
		cvct->channels[i].hidden = TS_READ16_BITS(pdata, 1, 3);
		cvct->channels[i].path_select = TS_READ16_BITS(pdata, 1, 4);
		cvct->channels[i].out_of_band = TS_READ16_BITS(pdata, 1, 5);
		cvct->channels[i].hide_guide = TS_READ16_BITS(pdata, 1, 6);
		cvct->channels[i].service_type = TS_READ16_BITS(pdata, 6, 10);
		pdata += 2;
		cvct->channels[i].source_id = TS_READ16(pdata);
		pdata += 2;
		cvct->channels[i].descriptors_length = TS_READ16_BITS(pdata, 10, 6);
		pdata += 2;
		list_head_init(&cvct->channels[i].list);
		parse_descriptors(&cvct->channels[i].list, pdata, cvct->channels[i].descriptors_length);
		pdata += cvct->channels[i].descriptors_length;
	}
	cvct->additional_descriptors_length = TS_READ16_BITS(pdata, 10, 6);
	pdata += 2;
	list_head_init(&cvct->list);
	parse_descriptors(&cvct->list, pdata, cvct->additional_descriptors_length);
	pdata += cvct->additional_descriptors_length;
	return 0;
}

static int parse_rrt(uint8_t *pbuf, uint16_t buf_size, atsc_rrt_t *rrt)
{
	uint8_t *pdata = pbuf;

	int ret = parse_section_header(pbuf, buf_size, &rrt->rrt_header);
	if (ret != 0)
		return ret;
	
	free_multi_string(&rrt->rating_region_name_text);
	if (rrt->dimensions) {
		for (int i = 0; i < rrt->dimensions_defined; i++) {
			free_multi_string(&rrt->dimensions[i].dimension_name_text);
			for (int j = 0; j < rrt->dimensions[i].values_defined; j++) {
				free_multi_string(&rrt->dimensions[i].rating[j].abbrev_rating_value_text);
				free_multi_string(&rrt->dimensions[i].rating[j].rating_value_text);
			}
		}
	}
	if (!list_empty(&rrt->list))
		free_descriptors(&rrt->list);

	pdata = rrt->rrt_header.private_data_byte;
	rrt->protocol_version = TS_READ8(pdata);
	pdata += 1;
	rrt->rating_region_name_length = TS_READ8(pdata);
	pdata += 1;
	parse_multi_string(pdata, &rrt->rating_region_name_text);
	pdata += rrt->rating_region_name_length;
	rrt->dimensions_defined = TS_READ8(pdata);
	pdata += 1;
	for (int i = 0; i < rrt->dimensions_defined; i ++) {
		rrt->dimensions[i].dimension_name_length = TS_READ8(pdata);
		pdata += 1;
		parse_multi_string(pdata, &rrt->dimensions[i].dimension_name_text);
		pdata += rrt->dimensions[i].dimension_name_length;
		rrt->dimensions[i].graduated_scale = TS_READ8_BITS(pdata, 1, 3);
		rrt->dimensions[i].values_defined = TS_READ8_BITS(pdata, 4, 4);
		pdata += 1;
		rrt->dimensions[i].rating = calloc(rrt->dimensions[i].values_defined, sizeof(struct define_rating));
		if (!rrt->dimensions[i].rating) {
			return ENOMEM;
		}
		for (int j = 0; j < rrt->dimensions[i].values_defined; j ++) {
			rrt->dimensions[i].rating[j].abbrev_rating_value_length = TS_READ8(pdata);
			pdata += 1;
			parse_multi_string(pdata, &rrt->dimensions[i].rating[j].abbrev_rating_value_text);
			pdata += rrt->dimensions[i].rating[j].abbrev_rating_value_length;
			rrt->dimensions[i].rating[j].rating_value_length = TS_READ8(pdata);
			pdata += 1;
			parse_multi_string(pdata, &rrt->dimensions[i].rating[j].rating_value_text);
			pdata += rrt->dimensions[i].rating[j].rating_value_length;
		}
	}
	rrt->descriptors_length = TS_READ16_BITS(pdata, 10, 6);
	pdata += 2;
	list_head_init(&rrt->list);
	parse_descriptors(&rrt->list, pdata, rrt->descriptors_length);

	return 0;
}

static int parse_ett(uint8_t *pbuf, uint16_t buf_size, atsc_ett_t *ett)
{
	uint8_t *pdata = pbuf;

	int ret = parse_section_header(pbuf, buf_size, &ett->ett_header);
	if (ret != 0)
		return ret;

	pdata = ett->ett_header.private_data_byte;
	ett->protocol_version = TS_READ8(pdata);
	pdata += 1;
	ett->ETM_id = TS_READ32(pdata);
	pdata += 4;
	parse_multi_string(pdata, &ett->extended_text_message);
	return 0;
}

static int parse_atsc_eit(uint8_t *pbuf, uint16_t buf_size, atsc_eit_t* eit)
{
	uint8_t *pdata = pbuf;

	int ret = parse_section_header(pbuf, buf_size, &eit->eit_header);
	if (ret != 0)
		return ret;
	if (eit->events) {
		for (int i = 0; i < eit->num_events_in_section; i ++) {
			free_multi_string(&eit->events[i].title_text);
			free_descriptors(&eit->events[i].list);
		}
		free(eit->events);
	}

	pdata = eit->eit_header.private_data_byte;
	eit->protocol_version = TS_READ8(pdata);
	pdata += 1;
	eit->num_events_in_section = TS_READ8(pdata);
	pdata += 1;
	eit->events = calloc(eit->num_events_in_section, sizeof(struct define_event));
	if (!eit->events) {
		return ENOMEM;
	}
	for (int i = 0; i < eit->num_events_in_section; i ++) {
		eit->events[i].event_id = TS_READ16_BITS(pdata, 14, 2);
		pdata += 2;
		eit->events[i].start_time = TS_READ32(pdata);
		pdata += 4;
		eit->events[i].ETM_location = TS_READ32_BITS(pdata, 2, 2);
		eit->events[i].length_in_seconds = TS_READ32_BITS(pdata, 20, 4);
		eit->events[i].title_length = TS_READ32_BITS(pdata, 8, 24);
		pdata += 4;
		parse_multi_string(pdata, &eit->events[i].title_text);
		pdata += eit->events[i].title_length;
		eit->events[i].descriptors_length = TS_READ16_BITS(pdata, 12, 4);
		pdata += 2;
		list_head_init(&eit->events[i].list);
		parse_descriptors(&eit->events[i].list, pdata, eit->events[i].descriptors_length);
	}

	return 0;
}

static int parse_stt(uint8_t *pbuf, uint16_t buf_size, atsc_stt_t* stt)
{
	uint16_t section_len = 0;
	uint8_t *pdata = pbuf;

	int ret = parse_section_header(pbuf, buf_size, &stt->stt_header);
	if (ret != 0)
		return ret;

	section_len = stt->stt_header.section_length;
	pdata = stt->stt_header.private_data_byte;
	section_len -= (5 + 4);
	stt->protocol_version = TS_READ8(pdata);
	pdata += 1;
	stt->system_time = TS_READ32(pdata);
	pdata += 4;
	stt->GPS_UTC_offset = TS_READ8(pdata);
	pdata += 1;
	stt->daylight_saving = TS_READ16(pdata);
	pdata += 2;
	section_len -= 8;

	if (!list_empty(&(stt->list))) {
		free_descriptors(&(stt->list));
	}

	while (section_len > 0) {
		list_head_init(&(stt->list));
		parse_descriptors(&(stt->list), pdata, (int)section_len);
	}

	return 0;
}

static int parse_dcct(uint8_t *pbuf, uint16_t buf_size, atsc_dcct_t* dcct)
{
	uint8_t *pdata = pbuf;

	int ret = parse_section_header(pbuf, buf_size, &dcct->dcct_header);
	if (ret != 0)
		return ret;

	pdata = dcct->dcct_header.private_data_byte;
	dcct->protocol_version = TS_READ8(pdata);
	pdata += 1;
	dcct->dcc_test_count = TS_READ8(pdata);
	pdata += 1;
	dcct->dcc_tests = calloc(dcct->dcc_test_count, sizeof(struct define_dcc_test));
	if (!dcct->dcc_tests) {
		return ENOMEM;
	}
	
	for (int i = 0; i < dcct->dcc_test_count; i ++) {
		dcct->dcc_tests[i].dcc_context = TS_READ64_BITS(pdata, 1, 0);
		dcct->dcc_tests[i].dcc_from_major_channel_number = TS_READ64_BITS(pdata, 10, 4);
		dcct->dcc_tests[i].dcc_from_minor_channel_number = TS_READ64_BITS(pdata, 10, 14);
		dcct->dcc_tests[i].dcc_to_major_channel_number = TS_READ64_BITS(pdata, 10, 28);
		dcct->dcc_tests[i].dcc_to_minor_channel_number = TS_READ64_BITS(pdata, 10, 38);
		dcct->dcc_tests[i].dcc_start_time = TS_READ64_BITS(pdata, 16, 48);
		pdata += 8;
		dcct->dcc_tests[i].dcc_start_time1 = TS_READ16(pdata);
		pdata += 2;
		dcct->dcc_tests[i].dcc_end_time = TS_READ32(pdata);
		pdata += 4;
		dcct->dcc_tests[i].dcc_term_count = TS_READ8(pdata);
		pdata += 1;
		dcct->dcc_tests[i].dcc_terms = calloc(dcct->dcc_tests[i].dcc_term_count, sizeof(struct define_dcc_term));
		if (!dcct->dcc_tests[i].dcc_terms) {
			return ENOMEM;
		}
		for (int j = 0; j < dcct->dcc_tests[i].dcc_term_count; j ++) {
			dcct->dcc_tests[i].dcc_terms[j].dcc_selection_type = TS_READ8(pdata);
			pdata += 1;
			dcct->dcc_tests[i].dcc_terms[j].dcc_selection_id = TS_READ64(pdata);
			pdata += 8;
			dcct->dcc_tests[i].dcc_terms[j].dcc_term_descriptors_length = TS_READ16_BITS(pdata, 10, 6);
			pdata += 2;
			list_head_init(&dcct->dcc_tests[i].dcc_terms[j].list);
			parse_descriptors(&dcct->dcc_tests[i].dcc_terms[j].list, pdata, dcct->dcc_tests[i].dcc_terms[j].dcc_term_descriptors_length);
			pdata += dcct->dcc_tests[i].dcc_terms[j].dcc_term_descriptors_length;
		}
		dcct->dcc_tests[i].dcc_test_descriptors_length = TS_READ16_BITS(pdata, 10, 6);
		pdata += 2;
		list_head_init(&dcct->dcc_tests[i].list);
		parse_descriptors(&dcct->dcc_tests[i].list, pdata, dcct->dcc_tests[i].dcc_test_descriptors_length);
		pdata += dcct->dcc_tests[i].dcc_test_descriptors_length;
	}
	dcct->dcc_additional_descriptors_length = TS_READ16_BITS(pdata, 10, 6);
	pdata += 2;
	list_head_init(&dcct->list);
	parse_descriptors(&dcct->list, pdata, dcct->dcc_additional_descriptors_length);
	return 0;
}

static int parse_dccsct(uint8_t *pbuf, uint16_t buf_size, atsc_dccsct_t* dccsct)
{
	uint8_t *pdata = pbuf;

	int ret = parse_section_header(pbuf, buf_size, &dccsct->dccsct_header);
	if (ret != 0)
		return ret;

	pdata = dccsct->dccsct_header.private_data_byte;
	dccsct->protocol_version = TS_READ8(pdata);
	pdata += 1;
	dccsct->updates_defined = TS_READ8(pdata);
	pdata += 1;
	dccsct->updates = calloc(dccsct->updates_defined, sizeof(struct define_update));
	if (!dccsct->updates) {
		return ENOMEM;
	}
	for (int i = 0; i < dccsct->updates_defined; i++) {
		dccsct->updates[i].update_type = TS_READ8(pdata);
		pdata += 1;
		dccsct->updates[i].update_data_length = TS_READ8(pdata);
		pdata += 1;
		if (dccsct->updates[i].update_type == 0x01) {
			dccsct->updates[i].genre_category_code = TS_READ8(pdata);
			pdata += 1;
			pdata += parse_multi_string(pdata, &dccsct->updates[i].genre_category_name_text);
		} else if (dccsct->updates[i].update_type == 0x02) {
			dccsct->updates[i].dcc_state_location_code = TS_READ8(pdata);
			pdata += 1;
			pdata += parse_multi_string(pdata, &dccsct->updates[i].dcc_state_location_code_text);
		} else if (dccsct->updates[i].update_type == 0x03) {
			dccsct->updates[i].state_code = TS_READ8(pdata);
			pdata += 1;
			dccsct->updates[i].dcc_county_location_code = TS_READ16_BITS(pdata, 10, 6);
			pdata += 2;
			pdata += parse_multi_string(pdata, &dccsct->updates[i].dcc_county_location_code_text);
		}
		dccsct->updates[i].dccsct_descriptors_length = TS_READ16_BITS(pdata, 10, 6);
		pdata += 2;
		list_head_init(&dccsct->updates[i].list);
		parse_descriptors(&dccsct->updates[i].list, pdata, dccsct->updates[i].dccsct_descriptors_length);
		pdata += dccsct->updates[i].dccsct_descriptors_length;
	}
	dccsct->dccsct_additional_descriptors_length = TS_READ16_BITS(pdata, 10, 6);
	pdata += 2;
	list_head_init(&dccsct->list);
	parse_descriptors(&dccsct->list, pdata, dccsct->dccsct_additional_descriptors_length);

	return 0;
}

static int psip_proc(uint16_t pid __maybe_unused, uint8_t *pkt, uint16_t len)
{
	switch (pkt[0]) {
		case MGT_TID:
			parse_mgt(pkt, len, &psip.mgt);
			psip_seen_mask |= PSIP_MGT;
			break;
		case TVCT_TID:
			parse_tvct(pkt, len, &psip.tvct);
			psip_seen_mask |= PSIP_TVCT;
			break;
		case CVCT_TID:
			parse_cvct(pkt, len, &psip.cvct);
			psip_seen_mask |= PSIP_CVCT;
			break;
		case RRT_TID:
			parse_rrt(pkt, len, &psip.rrt);
			psip_seen_mask |= PSIP_RRT;
			break;
		case ETT_TID:
			parse_ett(pkt, len, &psip.ett);
			psip_seen_mask |= PSIP_ETT;
			break;
		case EIT_TID:
			parse_atsc_eit(pkt, len, &psip.eit);
			psip_seen_mask |= PSIP_EIT;
			break;
		case STT_TID:
			parse_stt(pkt, len, &psip.stt);
			psip_seen_mask |= PSIP_STT;
			break;
		case DCCT_TID:
			parse_dcct(pkt, len, &psip.dcct);
			psip_seen_mask |= PSIP_DCCT;
			break;
		case DCCSCT_TID:
			parse_dccsct(pkt, len, &psip.dccsct);
			psip_seen_mask |= PSIP_DCCSCT;
			break;
		default:
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

	//filter tdt and tot at same time
	init_table_filter(TDT_PID, TDT_TID, 0xFF, tdt_tot_proc);
	init_table_filter(TOT_PID, TOT_TID, 0xFF, tdt_tot_proc);

	/* All PSIP tables share PID 0x1FFB, but MAX_FILTER_NUM (6) limits the
	 * number of filters per PID. Register a single wildcard filter (mask 0)
	 * and dispatch by table_id inside psip_proc. */
	init_table_filter(MGT_PID, 0, 0, psip_proc);
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
