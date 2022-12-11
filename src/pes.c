#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "filter.h"
#include "pes.h"
#include "ts.h"
#include "result.h"

#define PES_MAX_LENGTH (64 * 1024)

typedef struct {
	uint16_t pid_num;
	uint64_t pid_bitmap[128];
	pes_t *list;
} mpegts_pes_t;

static mpegts_pes_t pes = {.pid_num = 0,};

/*see iso13818-1 Table 2-17*/
int parse_pes_packet(uint16_t pid, uint8_t *pkt, uint16_t len)
{
	int i = 0;
	pes_t *pt = NULL;
	for (i = 0; i < pes.pid_num; i ++) {
		if (pid == pes.list[i].pid) {
			pt = &pes.list[i];
			break;
		}
	}
	if (pt == NULL)
		pt = (pes_t *)calloc(1, sizeof(pes_t));

	uint8_t *buf = pkt;
	pt->packet_start_code_prefix = ((uint32_t)buf[0] << 16 | (uint32_t)buf[1] << 8 | buf[2]);
	if (PES_PACKET_START != pt->packet_start_code_prefix) {
		return -1;
	}

	pt->stream_id = buf[3];
	buf += 4;
	pt->PES_packet_length = TS_READ16(buf);
	buf += 2;
	if ((pt->stream_id != stream_id_program_stream_map) && (pt->stream_id != stream_id_padding_stream) &&
		(pt->stream_id != stream_id_private_stream_2) && (pt->stream_id != stream_id_ECM_stream) && (pt->stream_id != stream_id_EMM_stream) &&
		(pt->stream_id != stream_id_program_stream_directory) && (pt->stream_id != stream_id_H222_DSMCC_stream) && (pt->stream_id != stream_id_H222_typeE_stream)) {
		pt->packet_data.PES_scrambling_control = (buf[0] >> 4) & 0x3;
		pt->packet_data.PES_priority = (buf[0] >> 3) & 0x1;
		pt->packet_data.data_alignment_indicator = (buf[0] >> 2) & 0x1;
		pt->packet_data.copyright = (buf[0] >> 1) & 0x1;
		pt->packet_data.original_or_copy = (buf[0]) & 0x1;
		buf += 1;
		pt->packet_data.PTS_DTS_flags = (buf[0] >> 6) & 0x3;
		pt->packet_data.ESCR_flag = (buf[0] >> 5) & 0x1;
		pt->packet_data.ES_rate_flag = (buf[0] >> 4) & 0x1;
		pt->packet_data.DSM_trick_mode_flag = (buf[0] >> 3) & 0x1;
		pt->packet_data.additional_copy_info_flag = (buf[0] >> 2) & 0x1;
		pt->packet_data.PES_CRC_flag = (buf[0] >> 1) & 0x1;
		pt->packet_data.PES_extension_flag = (buf[0]) & 0x1;
		buf += 1;
		pt->packet_data.PES_header_data_length = buf[0];
		buf += 1;
		if ((pt->packet_data.PTS_DTS_flags & 0x2) == 0x2) {
			// PTS
			pt->packet_data.pts.flag = 0x2;
			pt->packet_data.pts.PTS_DTS_1 = (buf[0] >> 1) & 0x7;
			buf += 1;
			pt->packet_data.pts.PTS_DTS_2 = TS_READ16(buf) >> 1;
			buf += 2;
			pt->packet_data.pts.PTS_DTS_3 = TS_READ16(buf) >> 1;
			buf += 2;
		}
		if ((pt->packet_data.PTS_DTS_flags & 0x1) == 0x1) {
			// DTS
			pt->packet_data.dts.flag = 0x3;
			pt->packet_data.dts.PTS_DTS_1 = (buf[0] >> 1) & 0x7;
			buf += 1;
			pt->packet_data.dts.PTS_DTS_2 = TS_READ16(buf) >> 1;
			buf += 2;
			pt->packet_data.dts.PTS_DTS_3 = TS_READ16(buf) >> 1;
			buf += 2;
		}
		if (pt->packet_data.ESCR_flag) {
			// ESCR
			pt->packet_data.escr.ESCR_base_1 = (buf[0] >> 3) & 0x7;
			pt->packet_data.escr.ESCR_base_2 = (((buf[0] & 0x3) << 13) | (buf[1] << 5) | (buf[2] >> 3));
			pt->packet_data.escr.ESCR_base_3 = ((buf[2] & 0x3) << 13) | (buf[3] << 5) | (buf[4] >> 3);
			pt->packet_data.escr.ESCR_extension = (((buf[4] & 0x3) << 8) | (buf[5] >> 1));
			buf += 6;
		}
		if (pt->packet_data.ES_rate_flag) {
			// ES_rate
			pt->packet_data.rate.ES_rate = (uint32_t)(buf[0] << 15 | buf[1] << 7 | buf[2] >> 1) & 0x3FFFFF;
			buf += 3;
		}
		if (pt->packet_data.DSM_trick_mode_flag) {
			// DSM_trick_mode
			pt->packet_data.trick_mode.DSM_trick_mode_control = buf[0] >> 5;
			pt->packet_data.trick_mode.rep_cntrl = buf[0] & 0x1F;
			buf += 1;
		}
		if (pt->packet_data.additional_copy_info_flag) {
			// additional_copy_info
			pt->packet_data.copy_info.additional_copy_info = buf[0] & 0x7F;
			buf += 1;
		}
		if (pt->packet_data.PES_CRC_flag) {
			// PES_CRC
			pt->packet_data.previous_PES_packet_CRC = (buf[0] << 8 | buf[1]);
			buf += 2;
		}
		if (pt->packet_data.PES_extension_flag) {
			// PES_extension
			pt->packet_data.extension.PES_private_data_flag = buf[0] >> 7;
			pt->packet_data.extension.pack_header_field_flag = ((buf[0] >> 6) & 0x1);
			pt->packet_data.extension.program_packet_sequence_counter_flag = ((buf[0] >> 5) & 0x1);
			pt->packet_data.extension.P_STD_buffer_flag = ((buf[0] >> 4) & 0x1);
			pt->packet_data.extension.PES_extension_flag_2 = buf[0] & 0x1;
			buf += 1;
			if (pt->packet_data.extension.PES_private_data_flag) {
				memcpy(pt->packet_data.extension.PES_private_data, buf, 16);
				buf += 16;
			}
			if (pt->packet_data.extension.pack_header_field_flag) {
				pt->packet_data.extension.pack_head_length = buf[0];
				buf += 1;
				buf += pt->packet_data.extension.pack_head_length;
			}
			if (pt->packet_data.extension.program_packet_sequence_counter_flag) {
				pt->packet_data.extension.ppsc.program_packet_sequence_counter = buf[0] & 0x7F;
				pt->packet_data.extension.ppsc.MPEG1_MPEG2_identifier = (buf[1] >> 6) & 0x1;
				pt->packet_data.extension.ppsc.original_stuff_length = buf[1] & 0x3F;
				buf += 2;
			}
			if (pt->packet_data.extension.P_STD_buffer_flag) {
				pt->packet_data.extension.pstd_buffer.PSTD_buffer_scale = (buf[0] >> 5) & 0x1;
				pt->packet_data.extension.pstd_buffer.PSTD_buffer_size = (((buf[0] & 0x1F) << 8) | buf[1]);
				buf += 2;
			}
			if (pt->packet_data.extension.PES_extension_flag_2) {
				pt->packet_data.extension.PES_extension_field_length = buf[0] & 0x7F;
				buf += 1;
				buf += pt->packet_data.extension.PES_extension_field_length;
			}
		}
		// stuffing_byte 0xFF
		// no more than 32 stuffing bytes

		// PES_packet_data_byte
	} else if ((pt->stream_id == stream_id_program_stream_map) || (pt->stream_id == stream_id_private_stream_2) ||
			   (pt->stream_id == stream_id_ECM_stream) || (pt->stream_id == stream_id_EMM_stream) ||
			   (pt->stream_id == stream_id_program_stream_directory) || (pt->stream_id == stream_id_H222_DSMCC_stream) ||
			   (pt->stream_id == stream_id_H222_typeE_stream)) {
		/* PES_packet_data_byte */
		pt->PES_packet_data_byte = buf;
	} else if (pt->stream_id == stream_id_padding_stream) {
		/* padding_byte, do not need parse */
	}
	/* elementary stream has one type content */
	return pt->PES_packet_length + 6;
}

static int pes_proc(uint16_t pid, uint8_t *pkt, uint16_t len)
{
	int ret = parse_pes_packet(pid, pkt, len);
	if (ret < 0) {
		// printf("error in parsing pes\n");
		return -1;
	}
	return 0;
}

void pes_init()
{
	memset(&pes, 0, sizeof(pes));
}

void dump_pes_infos()
{
	struct tsa_config *tsaconf = get_config();
	if (!tsaconf->detail)
		return;

	if (pes.pid_num > 0)
		rout(0, "PES", NULL);

	for (int i = 0; i < pes.pid_num; i++) {
		rout(1, "PID", "0x%x(%d)", pes.list[i].pid, pes.list[i].pid);
		rout(2, "stream_type", "%s", get_stream_type(pes.list[i].type));
		rout(2, "stream_id", "0x%x", pes.list[i].stream_id);
	}
}

void register_pes_ops(uint16_t pid, uint8_t stream_type)
{
	filter_param_t para = {
		.depth = 1,
		.coff = {0},
		.mask = {0},
	};
	if ((pes.pid_bitmap[ pid / 64] & ((uint64_t) 1 << (pid % 64))) == 0) {
		pes.pid_bitmap[ pid / 64] |= ((uint64_t) 1 << (pid % 64));
		pes.list = (pes_t *)realloc(pes.list, (pes.pid_num + 1) * sizeof(pes_t));
		pes.list[pes.pid_num].type = stream_type;
		pes.list[pes.pid_num].pid = pid;
		pes.pid_num ++;

		filter_t *f = filter_alloc(pid);
		filter_set(f, &para, pes_proc);
	}
}

void unregister_pes_ops()
{
	filter_param_t para = {
		.depth = 1,
		.coff = {0},
		.mask = {0},
		.negete = {0},
	};
	filter_t *f = NULL;
	for (int i = 0; i < pes.pid_num; i ++) {
		f = filter_lookup(pes.list[i].pid, &para);
		if (f) {
			filter_free(f);
		}
	}
	pes.pid_num = 0;
	free(pes.list);
}


bool check_es_pid(uint16_t pid)
{
	struct tsa_config *tsaconf = get_config();
	/* skip pes */
	if (!tsaconf->detail)
		return false;

	for (int i = 0x20; i < 0x1FFF; i ++) {
		if ((pes.pid_bitmap[ pid / 64] & ((uint64_t) 1 << (pid % 64))) == 1) {
			return true;
		}
	}
	return false;
}