#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "descriptor.h"
#include "result.h"
#include "table.h"
#include "table_priv.h"
#include "utils.h"

/* ---- dumping ---- */

void dump_tdt(tdt_t *p_tdt)
{
	char utc[20];
	rout(0, "TDT (Time and Date Table)", NULL);
	convert_UTC(&p_tdt->utc_time, utc, sizeof(utc));
	rout(1, "UTC time", "%s", utc);
}

void dump_tot(tot_t *p_tot)
{
	char utc[20];
	rout(0, "TOT (Time Offset Table)", NULL);
	convert_UTC(&p_tot->utc_time, utc, sizeof(utc));
	rout(1, "UTC time", "%s", utc);

	dump_descriptors(2, &(p_tot->list));
}

void dump_sdt(sdt_t *p_sdt)
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

void dump_bat(bat_t *p_bat)
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

void dump_nit(nit_t *p_nit)
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

void dump_eit(eit_t *p_eit)
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

/* ---- parsing ---- */

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

int parse_eit(uint8_t *pbuf, uint16_t buf_size, eit_t *p_eit)
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

int parse_tdt(uint8_t *pbuf, uint16_t buf_size, tdt_t *p_tdt)
{
	uint16_t section_len = 0;
	uint8_t *pdata = pbuf;

	if (pbuf == NULL || p_tdt == NULL) {
		return -1;
	}

	if (pdata[0] != TDT_TID) {
		return -1;
	}

	pdata += 1;
	section_len = TS_READ16(pdata) & 0xFFF;
	p_tdt->section_length = section_len;
	pdata += 2;
	memcpy(&p_tdt->utc_time, pdata, 5);
	return 0;
}

int parse_tot(uint8_t *pbuf, uint16_t buf_size, tot_t *p_tot)
{
	uint8_t *pdata = pbuf;

	if (pbuf == NULL || p_tot == NULL) {
		return -1;
	}

	if (pdata[0] != TOT_TID) {
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