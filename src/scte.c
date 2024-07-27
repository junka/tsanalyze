#include <stddef.h>
#include <stdint.h>
#include <errno.h>

#include "descriptor.h"
#include "filter.h"
#include "result.h"
#include "scte/scte.h"
#include "table.h"
#include "ts.h"
#include "types.h"

#define SCTE_MAX_PID_NUM (8)

typedef struct {
	uint16_t pid_num;
	uint64_t pid_bitmap[128];
	scte_t list[SCTE_MAX_PID_NUM];
	uint64_t scte_sections;
} mpegts_scte_t;

mpegts_scte_t scte;

int parse_splice_schedule(uint8_t *pbuf, int plen, struct splice_schedule *sche)
{
	uint16_t len = 0;
	uint8_t *pdata = pbuf;
	sche->splice_count = TS_READ8(pdata);
	pdata += 1;
	len += 1;
	sche->splices = calloc(sche->splice_count, sizeof(struct splice_event));
	if (!sche->splices) {
		return ENOMEM;
	}
	for (int i = 0; i < sche->splice_count; i++) {
		sche->splices[i].splice_event_id = TS_READ32(pdata);
		pdata += 4;
		len += 4;
		sche->splices[i].splice_event_cancel_indicator = TS_READ8_BITS(pdata, 1, 0);
		pdata += 1;
		len += 1;
		if (sche->splices[i].splice_event_cancel_indicator == 0) {
			sche->splices[i].out_of_network_indicator = TS_READ8_BITS(pdata, 1, 0);
			sche->splices[i].program_splice_flag = TS_READ8_BITS(pdata, 1, 1);
			sche->splices[i].duration_flag = TS_READ8_BITS(pdata, 1, 2);
			pdata += 1;
			len += 1;
			if (sche->splices[i].program_splice_flag == 1) {
				sche->splices[i].utc_splice_time = TS_READ32(pdata);
				pdata += 4;
				len += 4;
			}
			if (sche->splices[i].program_splice_flag == 0) {
				sche->splices[i].component_count = TS_READ8(pdata);
				pdata += 1;
				len += 1;
				sche->splices[i].components = calloc(sche->splices[i].component_count, sizeof(struct event_component));
				if (!sche->splices[i].components) {
					return ENOMEM;
				}
				for (int j = 0; j < sche->splices[i].component_count; j++) {
					sche->splices[i].components[j].component_tag = TS_READ8(pdata);
					pdata += 1;
					len += 1;
					sche->splices[i].components[j].utc_splice_time = TS_READ32(pdata);
					pdata += 4;
					len += 4;
				}
			}
			if (sche->splices[i].duration_flag) {
				sche->splices[i].duration.auto_return = TS_READ8_BITS(pdata, 1, 0);
				sche->splices[i].duration.duration_h = TS_READ8_BITS(pdata, 1, 7);
				pdata += 1;
				len += 1;
				sche->splices[i].duration.duration = TS_READ32(pdata);
				pdata += 4;
				len += 4;
			}
			sche->splices[i].unique_program_id = TS_READ16(pdata);
			pdata += 2;
			len += 2;
			sche->splices[i].avail_num = TS_READ8(pdata);
			pdata += 1;
			len += 1;
			sche->splices[i].avails_expected = TS_READ8(pdata);
			pdata += 1;
			len += 1;
		}
	}
	return len;
}

int parse_splice_time(uint8_t *pbuf, struct splice_time *time)
{
	uint16_t len = 0;
	uint8_t *pdata = pbuf;
	time->time_specified_flag = TS_READ8_BITS(pdata, 1, 0);
	time->pts_time_h = TS_READ8_BITS(pdata, 1, 7);
	pdata += 1;
	len += 1;
	if (time->time_specified_flag == 1) {
		time->pts_time = TS_READ32(pdata);
		pdata += 4;
		len += 4;
	}
	return len;
}

int parse_splice_insert(uint8_t *pbuf, int plen, struct splice_event *evt)
{
	uint16_t len = 0;
	uint8_t *pdata = pbuf;
	int ret = 0;

	evt->splice_event_id = TS_READ32(pdata);
	pdata += 4;
	len += 4;
	evt->splice_event_cancel_indicator = TS_READ8_BITS(pdata, 1, 0);
	pdata += 1;
	len += 1;
	if (evt->splice_event_cancel_indicator == 0) {
		evt->out_of_network_indicator = TS_READ8_BITS(pdata, 1, 0);
		evt->program_splice_flag = TS_READ8_BITS(pdata, 1, 1);
		evt->duration_flag = TS_READ8_BITS(pdata, 1, 2);
		evt->splice_immediate_flag = TS_READ8_BITS(pdata, 1, 3);
		pdata += 1;
		len += 1;
		if (evt->program_splice_flag == 1 && evt->splice_immediate_flag == 0) {
			ret = parse_splice_time(pdata, &evt->time);
			pdata += ret;
			len += ret;
		}
		if (evt->program_splice_flag == 0) {
			evt->component_count = TS_READ8(pdata);
			pdata += 1;
			len += 1;
			evt->components = calloc(evt->component_count, sizeof(struct event_component));
			if (!evt->components) {
				return ENOMEM;
			}
			for (int i = 0; i < evt->component_count; i++) {
				evt->components[i].component_tag = TS_READ8(pdata);
				pdata += 1;
				len += 1;
				if (evt->splice_immediate_flag == 0) {
					ret = parse_splice_time(pdata, &evt->components[i].time);
					pdata += ret;
					len += ret;
				}
			}
		}
		if (evt->duration_flag == 1) {
			evt->duration.auto_return = TS_READ8_BITS(pdata, 1, 0);
			evt->duration.duration_h = TS_READ8_BITS(pdata, 1, 7);
			pdata += 1;
			len += 1;
			evt->duration.duration = TS_READ32(pdata);
			pdata += 4;
			len += 4;
		}
		evt->unique_program_id = TS_READ16(pdata);
		pdata += 2;
		len += 2;
		evt->avail_num = TS_READ8(pdata);
		pdata += 1;
		len += 1;
		evt->avails_expected = TS_READ8(pdata);
		pdata += 1;
		len += 1;
	}
	return len;
}

static int parse_splice_private(uint8_t *pbuf)
{
	int len = 0;
	uint8_t *pdata = pbuf;
	uint32_t identifier __maybe_unused = TS_READ32(pdata);
	pdata += 4;
	len += 4;

	return len;
}

/* see table 17 */
int parse_splice_descriptors(struct list_head *h, uint8_t *buf, int len)
{
	int l = len;
	uint8_t *ptr = buf;
	descriptor_t *more = NULL;
	uint32_t identifier;
	void *des = NULL;
	while (l > 0) {
		// printf("%s(0x%x) : %d, %d\n", des_ops[ptr[0]].tag_name, ptr[0], l,
		// ptr[1]);
		uint8_t tag = ptr[0];
		// des = des_ops[tag].descriptor_alloc();
		des = calloc(1, sizeof(descriptor_t) + ptr[1]);
		if (!des) {
			return ENOMEM;
		}
		more = (descriptor_t *)des;
		more->tag = tag;
		more->length = ptr[1];
		ptr += 2;
		identifier = TS_READ32(ptr);
		if (identifier != 0x43554549) /* CUEI*/
		{
			// printf("invalid splice descriptor 0x%x\n", identifier);
		}
		ptr += 4;
		memcpy(more->data, ptr, more->length - 4);
		l -= more->length - 6;
		ptr += more->length;
		list_add_tail(h, &(more->n));
	}
	return 0;
}

void dump_splice_descriptors(int lv, struct list_head *list)
{
	const char *tag_name[] = {
#define _(a, v) #a,
		foreach_enum_scte_splice_descriptor
#undef _
	};
	descriptor_t *p = NULL, *next = NULL;
	list_for_each_safe(list, p, next, n)
	{
		if (p->tag < 5) {
			rout(lv, tag_name[p->tag], "0x%x len %d", p->tag, p->length);
			if (p->tag == 0x0) {
				struct avail_splice_descriptor avail;
				avail.provider_avail_id = TS_READ32(p->data);
				rout(lv + 1, "provider_avail_id", "0x%x", avail.provider_avail_id);
			} else if (p->tag == 0x1) {
				struct DTMF_splice_descriptor dtmf;
				dtmf.preroll = TS_READ8(p->data);
				dtmf.dtmf_count = (p->data[1] >> 5) & 0x11;
				dtmf.DTMF_char = &p->data[2];
				rout(lv + 1, "preroll", "%d", dtmf.preroll);
				rout(lv + 1, "dtmf_count", "%d", dtmf.dtmf_count);
				rout(lv + 1, "DTMF_char", "%s", dtmf.DTMF_char);
			}
		}
	}
}

static int parse_splice_info(uint8_t *pbuf, uint16_t buf_size, scte_t *splice)
{
	uint16_t section_len = 0;
	uint8_t *pdata = pbuf;
	int ret = 0;
	/* table id should be 0xFC */
	splice->table_id = TS_READ8(pdata);
	pdata += 1;
	splice->section_syntax_indicator = TS_READ8_BITS(pdata, 1, 0);
	splice->private_indicator = TS_READ8_BITS(pdata, 1, 1);
	splice->sap_type = TS_READ8_BITS(pdata, 2, 2);
	section_len = TS_READ16(pdata) & 0x0FFF;
	pdata += 2;
	if (splice->section_syntax_indicator == 1 && (section_len > 0x3FD)) {
		return -1;
	} else if (splice->section_syntax_indicator == 0 && (section_len > 0xFFD)) {
		return -1;
	}

	if (!list_empty(&(splice->list)))
		free_descriptors(&(splice->list));

	// res_hexdump(0, "", pbuf, buf_size);
	// printf("section len %d, buf_size %d\n", section_len, buf_size);

	splice->section_length = section_len;
	splice->protocol_version = TS_READ8(pdata);
	pdata += 1;
	splice->encrypted_packet = TS_READ8_BITS(pdata, 1, 0);
	splice->encryption_algorithm = TS_READ8_BITS(pdata, 6, 1);
	splice->pts_adjustment_h = TS_READ8_BITS(pdata, 1, 7);
	pdata += 1;
	splice->pts_adjustment = TS_READ32(pdata);
	pdata += 4;
	splice->cw_index = TS_READ8(pdata);
	pdata += 1;
	splice->tier = TS_READ32_BITS(pdata, 12, 0);
	splice->splice_command_length = TS_READ32_BITS(pdata, 12, 12);
	splice->splice_command_type = TS_READ32_BITS(pdata, 8, 24);
	pdata += 4;
	// printf("tier %d, splice_command_type %d, splice_command_length %d\n",
	// splice->tier, splice->splice_command_type,
	// splice->splice_command_length);
	switch (splice->splice_command_type) {
	case SPLICE_SCHEDULE:
		ret = parse_splice_schedule(pdata, splice->splice_command_length, &splice->schedule);
		break;
	case SPLICE_INSERT:
		ret = parse_splice_insert(pdata, splice->splice_command_length, &splice->evt);
		break;
	case SPLICE_TIME_SIGNAL:
		ret = parse_splice_time(pdata, &splice->t);
		break;
	case SPLICE_BANDWIDTH_RESERVATION:
		break;
	case SPLICE_PRIVATE:
		ret = parse_splice_private(pdata);
		break;
	case SPLICE_NULL:
	default:
		break;
	}
	if (splice->splice_command_length != 0xFFF)
		pdata += splice->splice_command_length;
	else
		pdata += ret;
	splice->descriptor_loop_length = TS_READ16(pdata);
	pdata += 2;
	parse_splice_descriptors(&(splice->list), pdata, splice->descriptor_loop_length);
	pdata += splice->descriptor_loop_length;
	/* skip alignment bytes and crc */

	return 0;
}

static int scte_proc(__maybe_unused uint16_t pid, uint8_t *pkt, uint16_t len)
{
	scte.scte_sections++;
	scte_t *pscte = NULL;
	for (int i = 0; i < SCTE_MAX_PID_NUM; i++) {
		if (scte.list[i].pid == pid) {
			pscte = &scte.list[i];
			break;
		}
	}
	if (unlikely(!pscte)) {
		printf("no scte filter for pid %d\n", pid);
		return -1;
	}
	parse_splice_info(pkt, len, pscte);
	return 0;
}

void register_scte_ops(uint16_t pid)
{
	if (scte.pid_num >= SCTE_MAX_PID_NUM) {
		printf("scte more than %d", SCTE_MAX_PID_NUM);
		return;
	}

	if ((scte.pid_bitmap[pid / 64] & ((uint64_t)1 << (pid % 64))) == 0) {
		scte.pid_bitmap[pid / 64] |= ((uint64_t)1 << (pid % 64));
		scte.list[scte.pid_num].pid = pid;
		list_head_init(&scte.list[scte.pid_num].list);
		scte.pid_num++;
		register_section_ops(pid, SCTE_SPLICE_TID, scte_proc);
	}
}

void unregister_scte_ops(void)
{
	filter_param_t para = {
		.depth = 1, .coff = { SCTE_SPLICE_TID }, .mask = { 0xFF }, .negate = { 0 },
	};
	filter_t *f = NULL;
	for (int i = 0; i < scte.pid_num; i++) {
		f = filter_lookup(scte.list[i].pid, &para);
		if (f) {
			filter_free(f);
		}
	}
	scte.pid_num = 0;
}

void dump_scte_info(void)
{
	struct tsa_config *tsaconf = get_config();
	if (!tsaconf->detail)
		return;

	if (scte.pid_num == 0) {
		return;
	}

	rout(0, "SCTE", NULL);
	for (int i = 0; i < scte.pid_num; i++) {
		rout(1, "PID", "%d", scte.list[i].pid);
		rout(2, "sap_type", "%d", scte.list[i].sap_type);
		rout(2, "protocol_version", "%d", scte.list[i].protocol_version);
		rout(2, "encrypted_packet", "%d", scte.list[i].encrypted_packet);
		rout(2, "encryption_algorithm", "%d", scte.list[i].encryption_algorithm);
		rout(2, "cw_index", "%d", scte.list[i].cw_index);
		rout(2, "tier", "%d", scte.list[i].tier);
		rout(2, "splice_command_type", "%d", scte.list[i].splice_command_type);
		rout(2, "splice_command_length", "%d", scte.list[i].splice_command_length);
		switch (scte.list[i].splice_command_type) {
		case SPLICE_SCHEDULE:
			rout(3, "splice_count", "%d", scte.list[i].schedule.splice_count);
			for (int k = 0; k < scte.list[i].schedule.splice_count; k++) {
				struct splice_event *evt = &scte.list[i].schedule.splices[k];
				rout(3, "splice_event_id", "%d", evt->splice_event_id);
				rout(3, "splice_event_cancel_indicator", "%d", evt->splice_event_cancel_indicator);
				if (evt->splice_event_cancel_indicator == 0) {
					rout(3, "out_of_network_indicator", "%d", evt->out_of_network_indicator);
					rout(3, "program_splice_flag", "%d", evt->program_splice_flag);
					rout(3, "duration_flag", "%d", evt->duration_flag);
					if (evt->program_splice_flag == 1) {
						rout(3, "utc_splice_time", "%u", evt->utc_splice_time);
					}
					if (evt->program_splice_flag == 0) {
						rout(3, "component_count", "%d", evt->component_count);
						for (int j = 0; j < evt->component_count; j++) {
							rout(3, "component_tag", "0x%x", evt->components[j].component_tag);
							if (evt->splice_immediate_flag == 0)
								rout(3, "utc_splice_time", "%u", evt->components[j].utc_splice_time);
						}
					}
					if (evt->duration_flag == 1) {
						rout(3, "duration", "%lu", (uint64_t)evt->duration.duration_h << 32 | evt->duration.duration);
					}
					rout(3, "unique_program_id", "%d", evt->unique_program_id);
					rout(3, "avail_num", "%d", evt->avail_num);
					rout(3, "avails_expected", "%d", evt->avails_expected);
				}
			}
			break;
		case SPLICE_INSERT:
			rout(3, "splice_event_id", "0x%x", scte.list[i].evt.splice_event_id);
			rout(3, "splice_event_cancel_indicator", "%d", scte.list[i].evt.splice_event_cancel_indicator);
			if (scte.list[i].evt.splice_event_cancel_indicator == 0) {
				rout(3, "out_of_network_indicator", "%d", scte.list[i].evt.out_of_network_indicator);
				rout(3, "program_splice_flag", "%d", scte.list[i].evt.program_splice_flag);
				rout(3, "duration_flag", "%d", scte.list[i].evt.duration_flag);
				rout(3, "splice_immediate_flag", "%d", scte.list[i].evt.splice_immediate_flag);
				if (scte.list[i].evt.program_splice_flag && scte.list[i].evt.splice_immediate_flag == 0) {
					rout(3, "pts_time", "%lu",
						 (uint64_t)scte.list[i].evt.time.pts_time_h << 32 | scte.list[i].evt.time.pts_time);
				}
				if (scte.list[i].evt.program_splice_flag == 0) {
					rout(3, "component_count", "%d", scte.list[i].evt.component_count);
					for (int j = 0; j < scte.list[i].evt.component_count; j++) {
						rout(3, "component_tag", "0x%x", scte.list[i].evt.components[j].component_tag);
						if (scte.list[i].evt.splice_immediate_flag == 0)
							rout(3, "pts_time", "%lu", (uint64_t)scte.list[i].evt.components[j].time.pts_time_h << 32 |
														   scte.list[i].evt.components[j].time.pts_time);
					}
				}
				if (scte.list[i].evt.duration_flag == 1) {
					rout(3, "duration", "%lu",
						 (uint64_t)scte.list[i].evt.duration.duration_h << 32 | scte.list[i].evt.duration.duration);
				}
				rout(3, "unique_program_id", "%d", scte.list[i].evt.unique_program_id);
				rout(3, "avail_num", "%d", scte.list[i].evt.avail_num);
				rout(3, "avails_expected", "%d", scte.list[i].evt.avails_expected);
			}
			break;
		case SPLICE_TIME_SIGNAL:
			rout(3, "pts_time", "%lu", (uint64_t)scte.list[i].t.pts_time_h << 32 | scte.list[i].t.pts_time);
			break;
		case SPLICE_BANDWIDTH_RESERVATION:
			break;
		case SPLICE_PRIVATE:
			break;
		case SPLICE_NULL:
			break;
		default:
			break;
		}
		if (!list_empty(&(scte.list[i].list)))
			dump_splice_descriptors(2, &(scte.list[i].list));
	}

	return;
}
