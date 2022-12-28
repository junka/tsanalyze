#include <stdint.h>
#include <stddef.h>

#include "types.h"
#include "scte/scte.h"
#include "descriptor.h"
#include "result.h"
#include "ts.h"
#include "filter.h"
#include "table.h"

#define SCTE_MAX_PID_NUM (8)

typedef struct {
	uint16_t pid_num;
	uint64_t pid_bitmap[128];
	scte_t list[SCTE_MAX_PID_NUM];
	uint64_t scte_sections;
} mpegts_scte_t;

mpegts_scte_t scte;


int parse_splice_schedule(uint8_t *pbuf, int plen)
{
    uint16_t len = 0;
    uint8_t *pdata = pbuf;
    uint8_t splice_count = TS_READ8(pdata);
    pdata += 1;
    len += 1;
    struct splice_event evt;
    for (int i = 0; i < splice_count; i ++) {
        evt.splice_event_id = TS_READ32(pdata);
        pdata += 4;
        len += 4;
        evt.splice_event_cancel_indicator = TS_READ8_BITS(pdata, 1, 0);
        pdata += 1;
        len += 1;
        if (evt.splice_event_cancel_indicator == 0) {
            evt.out_of_network_indicator = TS_READ8_BITS(pdata, 1, 0);
            evt.program_splice_flag = TS_READ8_BITS(pdata, 1, 1);
            evt.duration_flag = TS_READ8_BITS(pdata, 1, 2);
            pdata += 1;
            len += 1;
            if (evt.program_splice_flag == 1) {
                evt.utc_splice_time = TS_READ32(pdata);
                pdata += 4;
                len += 4;
            }
            if (evt.program_splice_flag == 0) {
                evt.component_count = TS_READ8(pdata);
                pdata += 1;
                len += 1;
                evt.components = calloc(evt.component_count, sizeof(struct event_component));
                for (int j = 0; j < evt.component_count; j++) {
                    evt.components[j].component_tag = TS_READ8(pdata);
                    pdata += 1;
                    len += 1;
                    evt.components[j].utc_splice_time = TS_READ32(pdata);
                    pdata += 4;
                    len += 4;
                }
            }
            if (evt.duration_flag) {
                evt.duration.auto_return = TS_READ8_BITS(pdata, 1, 0);
                evt.duration.duration_h = TS_READ8_BITS(pdata, 1, 7);
                pdata += 1;
                len += 1;
                evt.duration.duration = TS_READ32(pdata);
                pdata += 4;
                len += 4;
            }
            evt.unique_program_id = TS_READ16(pdata);
            pdata += 2;
            len += 2;
            evt.avail_num = TS_READ8(pdata);
            pdata += 1;
            len += 1;
            evt.avails_expected = TS_READ8(pdata);
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

int parse_splice_insert(uint8_t *pbuf, int plen)
{
    uint16_t len = 0;
    uint8_t *pdata = pbuf;
    int ret = 0;

    struct splice_event evt;
    evt.splice_event_id = TS_READ32(pdata);
    pdata += 4;
    len += 4;
    evt.splice_event_cancel_indicator = TS_READ8_BITS(pdata, 1, 0);
    pdata += 1;
    len += 1;
    if (evt.splice_event_cancel_indicator == 0) {
        evt.out_of_network_indicator = TS_READ8_BITS(pdata, 1, 0);
        evt.program_splice_flag = TS_READ8_BITS(pdata, 1, 1);
        evt.duration_flag = TS_READ8_BITS(pdata, 1, 2);
        evt.splice_immediate_flag = TS_READ8_BITS(pdata, 1, 3);
        pdata += 1;
        len += 1;
        if (evt.program_splice_flag == 1 && evt.splice_immediate_flag == 0) {
            ret = parse_splice_time(pdata, &evt.time);
            pdata += ret;
            len += ret;
        }
        if (evt.program_splice_flag == 0) {
            evt.component_count = TS_READ8(pdata);
            pdata += 1;
            len += 1;
            evt.components = calloc(evt.component_count, sizeof(struct event_component));
            for (int i = 0; i < evt.component_count; i ++) {
                evt.components[i].component_tag = TS_READ8(pdata);
                pdata += 1;
                len += 1;
                if (evt.splice_immediate_flag == 0) {
                    ret = parse_splice_time(pdata, &evt.components[i].time);
                    pdata += ret;
                    len += ret;
                }
            }
        }
        if (evt.duration_flag == 1) {
            evt.duration.auto_return = TS_READ8_BITS(pdata, 1, 0);
            evt.duration.duration_h = TS_READ8_BITS(pdata, 1, 7);
            pdata += 1;
            len += 1;
            evt.duration.duration = TS_READ32(pdata);
            pdata += 4;
            len += 4;
        }
        evt.unique_program_id = TS_READ16(pdata);
        pdata += 2;
        len += 2;
        evt.avail_num = TS_READ8(pdata);
        pdata += 1;
        len += 1;
        evt.avails_expected = TS_READ8(pdata);
        pdata += 1;
        len += 1;
    }
    return len;
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
	splice->section_length = section_len;
	splice->protocol_version = TS_READ8(pdata);
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
    struct splice_time t;
	switch (splice->splice_command_type) {
        case SPLICE_SCEDULE:
            ret = parse_splice_schedule(pdata, splice->splice_command_length);
            break;
        case SPLICE_INSERT:
            ret = parse_splice_insert(pdata, splice->splice_command_length);
            break;
        case SPLICE_TIME_SIGNAL:
            ret = parse_splice_time(pdata, &t);
            break;
        case SPLICE_BANDWIDTH_RESERVATION:
            break;
        case SPLICE_PRIVATE:
            break;
        case SPLICE_NULL:
        default:
            break;
	}
	pdata += splice->splice_command_length;
	splice->descriptor_loop_length = TS_READ16(pdata);
	pdata += 2;
	parse_descriptors(&(splice->list), pdata, splice->descriptor_loop_length);
    pdata += splice->descriptor_loop_length;
	/* skip alignment bytes and crc */

	return 0;
}

static int scte_proc(__attribute__((unused)) uint16_t pid, uint8_t *pkt, uint16_t len)
{
	scte.scte_sections++;
    scte_t *pscte = NULL;
    for (int i = 0; i < SCTE_MAX_PID_NUM; i ++) {
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
		scte.pid_num ++;
        register_section_ops(pid, SCTE_SPLICE_TID, scte_proc);
	}
}

void unregister_scte_ops(void)
{
	filter_param_t para = {
		.depth = 1,
		.coff = {SCTE_SPLICE_TID},
		.mask = {0xFF},
		.negete = {0},
	};
	filter_t *f = NULL;
	for (int i = 0; i < scte.pid_num; i ++) {
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
    for (int i = 0; i < scte.pid_num; i ++) {
        rout(1, "PID", "%d", scte.list[i].pid);
        rout(2, "sap_type", "%d", scte.list[i].sap_type);
        rout(2, "splice_command_type", "%d", scte.list[i].splice_command_type);
        rout(2, "splice_command_length", "%d", scte.list[i].splice_command_length);
        if (!list_empty(&(scte.list[i].list)))
		    dump_descriptors(2, &(scte.list[i].list));
    }

	return;
}