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

int parse_splice_descriptor(uint8_t *pbuf)
{
	return 0;
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
	switch (splice->splice_command_type) {
        case SPLICE_SCEDULE:
            break;
        case SPLICE_INSERT:
            break;
        case SPLICE_TIME_SIGNAL:
            break;
        case SPLICE_BANDWIDTH_RESERVATION:
            break;
        case SPLICE_PRIVATE:
            break;
        case SPLICE_NULL:
        default:
            break;
	}
	pdata += ret;
	splice->descriptor_loop_length = TS_READ16(pdata);
	pdata += 2;
	parse_splice_descriptor(pdata);
    pdata += 2;
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
	filter_param_t para = {
		.depth = 1,
		.coff = { SCTE_SPLICE_TID },
		.mask = { 0xFF },
	};

	if ((scte.pid_bitmap[pid / 64] & ((uint64_t)1 << (pid % 64))) == 0) {
		scte.pid_bitmap[pid / 64] |= ((uint64_t)1 << (pid % 64));
		scte.list[scte.pid_num].pid = pid;
		scte.pid_num ++;
		filter_t *f = filter_alloc(pid);
		filter_set(f, &para, scte_proc);
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
    }

	return;
}