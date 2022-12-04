#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "filter.h"
#include "io.h"
#include "table.h"
#include "ts.h"
#include "utils.h"

/*
 * port from ffmpeg for judging TS packet length
 */

static int analyze(const uint8_t *buf, int size, int packet_size, int *index)
{
	int stat[TS_MAX_PACKET_SIZE];
	int i;
	int x = 0;
	int best_score = 0;

	memset(stat, 0, packet_size * sizeof(int));

	for (x = i = 0; i < size - 3; i++) {
		if ((buf[i] == 0x47) && !(buf[i + 1] & 0x80) && (buf[i + 3] & 0x30)) {
			stat[x]++;

			if (stat[x] > best_score) {
				best_score = stat[x];
				if (index)
					*index = x;
			}
		}

		x++;
		if (x == packet_size)
			x = 0;
	}

	return best_score;
}

static int mpegts_probe(unsigned char *buf, int buf_size)
{
#define CHECK_COUNT 10

	const int size = buf_size;
	int score, fec_score, dvhs_score;
	int check_count = size / TS_FEC_PACKET_SIZE;

	if (check_count < CHECK_COUNT)
		return -1;

	score = analyze(buf, TS_PACKET_SIZE * check_count, TS_PACKET_SIZE, NULL) * CHECK_COUNT / check_count;
	dvhs_score = analyze(buf, TS_DVHS_PACKET_SIZE * check_count, TS_DVHS_PACKET_SIZE, NULL) * CHECK_COUNT / check_count;
	fec_score = analyze(buf, TS_FEC_PACKET_SIZE * check_count, TS_FEC_PACKET_SIZE, NULL) * CHECK_COUNT / check_count;

	/*
	 * we need a clear definition for the returned score ,
	 * otherwise things will become messy sooner or later
	 */
	if (score > fec_score && score > dvhs_score && score > 6)
		return 0; // 100 + score - CHECK_COUNT;
	else if (dvhs_score > score && dvhs_score > fec_score && dvhs_score > 6)
		return 1; // 100 + dvhs_score - CHECK_COUNT;
	else if (fec_score > 6)
		return 2; // 100 + fec_score - CHECK_COUNT;
	else
		return -1;
}

struct section_parser {
	int total_len;
	int32_t limit_len;
	uint8_t cc;
	uint8_t buffer[65536];
};

// do memcpy if section length greater than one packet
int16_t section_preproc(uint16_t pid, uint8_t *pkt, uint16_t len, uint8_t **buffering,
						uint8_t payload_unit_start_indicator, uint8_t continuity_counter, uint8_t psi_or_pes)
{
	static struct section_parser sec[8192];
	struct section_parser *p = &sec[pid];
	*buffering = NULL;

	/* indicate start of PES or PSI */
	if (payload_unit_start_indicator == 1) {
		p->cc = continuity_counter;
		if (psi_or_pes == 0) {
			/* PSI */
			uint8_t pointer_field = pkt[0];
			/*skip pointer_field, valid for PSI and stream_type 0x05 private_sections*/
			p->total_len = len - 1 - pointer_field;
			/* a PSI section following, skip table id now */
			p->limit_len = (int32_t)(((int16_t)pkt[2 + pointer_field] << 8) | pkt[3 + pointer_field]) & 0x0FFF;
			p->limit_len += 3;
			/*section in one pkt , go without buffering*/
			if (p->limit_len <= (p->total_len)) {
				*buffering = (pkt + 1 + pointer_field);
				return p->limit_len;
			} else {
				memset(p->buffer, 0, 4096); // PSI length less than this
				memcpy(p->buffer, pkt + 1 + pointer_field, p->total_len);
			}
		} else { /* PES doesn't have pointer field */
			p->total_len = len;
			p->limit_len = (int32_t)(((int16_t)pkt[4] << 8) | pkt[5]) & 0xFFFF;
			p->limit_len += 6;
			if (p->limit_len == 6) {
				// uint8_t stream_id = pkt[3];
				/* allowed only for a video elementary stream */
				// if (stream_id <= 0xEF && stream_id >= 0xE0) {
					memset(p->buffer, 0, 65536);
					memcpy(p->buffer, pkt, p->total_len);
				// }
				return -1;
			} else if (p->limit_len <= p->total_len) {
				*buffering = pkt;
				return p->total_len;
			} else {
				memset(p->buffer, 0, 65536);
				memcpy(p->buffer, pkt, p->total_len);
			}
		}

	} else {
		if (psi_or_pes == 0) {
			if (p->total_len == 0)
				return -1;
			memcpy(p->buffer + p->total_len, pkt, len);
			p->total_len += len;
			if (p->total_len >= p->limit_len) {
				*buffering = p->buffer;
				p->total_len = 0;
				return p->limit_len;
			}
		} else {
			if (p->total_len == 0)
				return -1;
			memcpy(p->buffer + p->total_len, pkt, len);
			p->total_len += len;
			if (p->total_len >= p->limit_len) {
				*buffering = p->buffer;
				p->total_len = 0;
				return p->total_len;
			}
		}
	}
	/* tell us buffering */
	return -1;
}

struct pid_ops {
	uint16_t pid;
	uint64_t pkts_in;
	uint64_t error_in;
	uint64_t bits_in;
	uint64_t pcr;
	uint64_t bitrate;
};

struct pid_ops pid_dev[MAX_TS_PID_NUM];

uint64_t calc_pcr_clock(pcr_clock pcr)
{
	return pcr.program_clock_reference_base * 300 + pcr.program_clock_reference_extension;
}

int ts_adaptation_field_proc(uint8_t *data, uint8_t len)
{
	ts_adaptation_field adapt;
	pcr_clock pcr, opcr;
	uint8_t *ptr = data;
	uint8_t l = len;

	adapt.discontinuity_indicator = TS_READ_BIT(ptr, 7);
	adapt.random_access_indicator = TS_READ_BIT(ptr, 6);
	adapt.elementary_stream_priority_indicator = TS_READ_BIT(ptr, 5);
	adapt.PCR_flag = TS_READ_BIT(ptr, 4);
	adapt.OPCR_flag = TS_READ_BIT(ptr, 3);
	adapt.splicing_point_flag = TS_READ_BIT(ptr, 2);
	adapt.transport_private_data_flag = TS_READ_BIT(ptr, 1);
	adapt.adaptation_field_extension_flag = TS_READ_BIT(ptr, 0);
	PL_STEP(ptr, l, 1);

	if (adapt.PCR_flag) {
		pcr.program_clock_reference_base = (((uint64_t)TS_READ32(ptr) << 1) | ptr[4] >> 7);
		PL_STEP(ptr, l, 4);
		pcr.program_clock_reference_extension = (TS_READ16(ptr) & 0x1FF);
		PL_STEP(ptr, l, 2);
		// printf("clock %lu\n",calc_pcr_clock(pcr));
	}
	if (adapt.OPCR_flag) {
		opcr.program_clock_reference_base = (((uint64_t)TS_READ32(ptr) << 1) | ptr[4] >> 7);
		PL_STEP(ptr, l, 4);
		opcr.program_clock_reference_extension = (TS_READ16(ptr) & 0x1FF);
		PL_STEP(ptr, l, 2);
		// printf("original clock %lu\n",calc_pcr_clock(pcr));
	}
	if (adapt.splicing_point_flag) {
		PL_STEP(ptr, l, 1);
	}
	if (adapt.transport_private_data_flag) {
		uint8_t transport_private_data_length = TS_READ8(ptr);
		PL_STEP(ptr, l, 1);
		PL_STEP(ptr, l, transport_private_data_length);
	}
	if (adapt.adaptation_field_extension_flag) {
		uint8_t adaptation_field_extension_length = TS_READ8(ptr);
		PL_STEP(ptr, l, 1);
		PL_STEP(ptr, l, adaptation_field_extension_length);
	}
	if (l != 0)
		return -1;

	return 0;
}

int ts_proc(uint8_t *data, uint8_t len)
{
	ts_header head;
	uint8_t psi_or_pes = 1;
	uint8_t *ptr = data;
	int16_t sec_len = -1;
	uint8_t *pbuf = NULL;
	if (unlikely(ptr == NULL))
		return -1;
	if (unlikely(ptr[0] != TS_SYNC_BYTE))
		return -1;

	PL_STEP(ptr, len, 1);
	head.PID = TS_READ16(ptr) & 0x1FFF;
	head.transport_error_indicator = TS_READ8(ptr) >> 7;
	head.payload_unit_start_indicator = TS_READ8(ptr) >> 6;
	PL_STEP(ptr, len, 2);
	head.adaptation_field_control = (TS_READ8(ptr) >> 4) & 0x3;
	head.continuity_counter = TS_READ8(ptr) & 0x3;
	PL_STEP(ptr, len, 1);

	pid_dev[head.PID].pkts_in++;

	if (head.adaptation_field_control == ADAPT_ONLY ||
		head.adaptation_field_control == ADAPT_BOTH) {
		ts_adaptation_field adapt;
		adapt.adaptation_field_length = TS_READ8(ptr);
		PL_STEP(ptr, len, 1);
		ts_adaptation_field_proc(ptr, adapt.adaptation_field_length);
		PL_STEP(ptr, len, adapt.adaptation_field_length);
		// TODO
	}

	/* no data_byte */
	if (head.adaptation_field_control == ADAPT_ONLY ||
		head.adaptation_field_control == ADAPT_RESERVED) {
		return 0;
	}

	if (head.transport_error_indicator == 1) {
		pid_dev[head.PID].error_in++;
	}

	if (head.PID == NULL_PID) {
		return 0;
	}

	if (head.PID < 0x20 || check_pmt_pid(head.PID)) // take as psi
		psi_or_pes = 0;

	if (psi_or_pes && !check_es_pid(head.PID)) {
		/* unrefered pes or what */
		return -1;
	}

	sec_len = section_preproc(head.PID, ptr, len, &pbuf, 
							  head.payload_unit_start_indicator,
							  head.continuity_counter, psi_or_pes);
	if (sec_len == -1)
		return 0;
	
	/*use filter to process a section*/
	filter_proc(head.PID, pbuf, sec_len);

	return 0;
}

void dump_ts_info(void)
{
	struct tsa_config *tsaconf = get_config();
	if (!tsaconf->stats)
		return;

	uint16_t pid = 0;
	rout(0, "TS bits statistics:");
	rout(1, "%7s%21s%11s", "PID", "In", "Err");
	for (pid = 0; pid <= NULL_PID; pid++) {
		if (pid_dev[pid].pkts_in)
			rout(1, "%04d(0x%04x)  %2c  %10" PRIu64 "%10" PRIu64, pid, pid,
				 ':', pid_dev[pid].pkts_in, pid_dev[pid].error_in);
	}
}

int init_pid_processor(void)
{
	filter_init();
	init_table_ops();
	init_descriptor_parsers();
	return 0;
}

void uninit_pid_processor(void) 
{
	uninit_table_ops();
}

int ts_process()
{
	struct tsa_config *tsaconf = get_config();
	struct io_ops *ops = lookup_io_ops(tsaconf->type);
	void *ptr = NULL;
	size_t len, ts_pktlen = 0, pkt_con_len = 0;
	int start_index = 0;
	int typ = 0;
	uint8_t pkt_con[TS_FEC_PACKET_SIZE];

	if (ops->open(tsaconf->name) < 0)
		return -1;

	ops->read(&ptr, &len);

	typ = mpegts_probe((uint8_t *)ptr, len);
	if (typ == 0) {
		ts_pktlen = TS_PACKET_SIZE;
	} else if (typ == 1) {
		ts_pktlen = TS_DVHS_PACKET_SIZE;
	} else if (typ == 2) {
		ts_pktlen = TS_FEC_PACKET_SIZE;
	} else {
		printf("TS file invalid format\n");
		return -1;
	}

	// hexdump(ptr, 188);
	analyze(ptr, ts_pktlen * 2, ts_pktlen, &start_index);
	//printf("Ts starting at offset %d\n", start_index);
	// hexdump(ptr+start_index, ts_pktlen);

	ptr += start_index;
	len -= start_index;
	PL_STEP(ptr, len, start_index);

	while (ops->end()) {
		if (pkt_con_len == ts_pktlen) {
			ts_proc(pkt_con, ts_pktlen);
			pkt_con_len = 0;
		}
		while (len >= ts_pktlen) {
			ts_proc(ptr, ts_pktlen);
			PL_STEP(ptr, len, ts_pktlen);
		}
		if (len) {
			memcpy(pkt_con, ptr, len);
			pkt_con_len = len;
		}
		if (ops->read(&ptr, &len) < 0)
			break;
		if (pkt_con_len) {
			memcpy(pkt_con + pkt_con_len, ptr, ts_pktlen - pkt_con_len);
			PL_STEP(ptr, len, ts_pktlen - pkt_con_len);
			pkt_con_len = ts_pktlen;
		}
	}
	ops->close();

	return 0;
}
