#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "filter.h"
#include "pes.h"
#include "ts.h"
#include "result.h"
#include "subtitle.h"
#include "teletext.h"

#define PES_MAX_LENGTH (64 * 1024)

typedef struct {
	uint16_t pid_num;
	uint64_t pid_bitmap[128];
	uint16_t pid_index[TS_MAX_PID + 1];
	pes_t *list;
} mpegts_pes_t;

static mpegts_pes_t pes = {.pid_num = 0, .list = NULL};

// static pes_data_callback pes_fns[0xFF] = {NULL};


void *pes_private_alloc(uint8_t tag)
{
	if (tag == 0x59) {
		struct subtitle_pes_data *sub = calloc(1, sizeof(struct subtitle_pes_data));
		if (!sub) {
			return NULL;
		}
		list_head_init(&sub->seg_list);
		return sub;
	} else if (tag == 0x56) {
		struct teletext_pes_data *text = calloc(1, sizeof(struct teletext_pes_data));
		return text;
	}
	return NULL;
}

void register_pes_data_callback(uint16_t pid, uint8_t stream_type, pes_data_callback cb, uint8_t tag)
{
	// pes_fns[stream_type] = cb;
	if (pes.pid_index[pid] >= pes.pid_num) {
		return;
	}
	pes_t *pt = &pes.list[pes.pid_index[pid]];
	//run for pes already in parsing
	if (pt->type == stream_type && !pt->cb) {
		pt->cb = cb;
		if (!pt->priv) {
			pt->tag = tag;
			pt->priv = pes_private_alloc(tag);
		}
	}

}


/*see iso13818-1 Table 2-17*/
int parse_pes_packet(uint16_t pid, uint8_t *pkt, uint16_t len)
{
	pes_t *pt = NULL;

	if (pes.pid_bitmap[ pid / 64] & ((uint64_t) 1 << (pid % 64))) {
		if (pes.pid_index[pid] > pes.pid_num) {
			printf("pes not register for %d\n", pid);
			return -1;
		}
		pt = &pes.list[pes.pid_index[pid]];
	}

	if (pt == NULL) {
		printf("pes not register for %d\n", pid);
		return -1;
	}

	uint8_t *buf = pkt;
	pt->packet_start_code_prefix = ((uint32_t)buf[0] << 16 | (uint32_t)buf[1] << 8 | buf[2]);
	if (PES_PACKET_START != pt->packet_start_code_prefix) {
		return -1;
	}
	int head_len = 0;
	pt->stream_id = buf[3];
	buf += 4;
	pt->PES_packet_length = TS_READ16(buf);
	buf += 2;
	head_len += 6;
	if ((pt->stream_id != stream_id_program_stream_map) && (pt->stream_id != stream_id_padding_stream) &&
		(pt->stream_id != stream_id_private_stream_2) && (pt->stream_id != stream_id_ECM_stream) &&
		(pt->stream_id != stream_id_EMM_stream) && (pt->stream_id != stream_id_program_stream_directory) &&
		(pt->stream_id != stream_id_H222_DSMCC_stream) && (pt->stream_id != stream_id_H222_typeE_stream)) {
		pt->packet_data.PES_scrambling_control = TS_READ8_BITS(buf, 2, 2);
		pt->packet_data.PES_priority = TS_READ8_BITS(buf, 1, 4);
		pt->packet_data.data_alignment_indicator = TS_READ8_BITS(buf, 1, 5);
		pt->packet_data.copyright = TS_READ8_BITS(buf, 1, 6);
		pt->packet_data.original_or_copy = TS_READ8_BITS(buf, 1, 7);
		buf += 1;
		pt->packet_data.PTS_DTS_flags = TS_READ8_BITS(buf, 2, 0);
		pt->packet_data.ESCR_flag = TS_READ8_BITS(buf, 1, 2);
		pt->packet_data.ES_rate_flag = TS_READ8_BITS(buf, 1, 3);
		pt->packet_data.DSM_trick_mode_flag = TS_READ8_BITS(buf, 1, 4);
		pt->packet_data.additional_copy_info_flag = TS_READ8_BITS(buf, 1, 5);
		pt->packet_data.PES_CRC_flag = TS_READ8_BITS(buf, 1, 6);
		pt->packet_data.PES_extension_flag = TS_READ8_BITS(buf, 1, 7);
		buf += 1;
		pt->packet_data.PES_header_data_length = TS_READ8(buf);
		buf += 1;
		head_len += 3;
		if ((pt->packet_data.PTS_DTS_flags & 0x2) == 0x2) {
			// PTS
			pt->packet_data.pts.flag = 0x2;
			pt->packet_data.pts.PTS_DTS_1 = TS_READ8_BITS(buf, 3, 4);
			buf += 1;
			pt->packet_data.pts.PTS_DTS_2 = TS_READ16(buf) >> 1;
			buf += 2;
			pt->packet_data.pts.PTS_DTS_3 = TS_READ16(buf) >> 1;
			buf += 2;
			head_len += 5;
		}
		if ((pt->packet_data.PTS_DTS_flags & 0x1) == 0x1) {
			// DTS
			pt->packet_data.dts.flag = 0x3;
			pt->packet_data.dts.PTS_DTS_1 = TS_READ8_BITS(buf, 3, 4);
			buf += 1;
			pt->packet_data.dts.PTS_DTS_2 = TS_READ16(buf) >> 1;
			buf += 2;
			pt->packet_data.dts.PTS_DTS_3 = TS_READ16(buf) >> 1;
			buf += 2;
			head_len += 5;
		}
		if (pt->packet_data.ESCR_flag) {
			// ESCR
			pt->packet_data.escr.ESCR_base_1 = (buf[0] >> 3) & 0x7;
			pt->packet_data.escr.ESCR_base_2 = (((buf[0] & 0x3) << 13) | (buf[1] << 5) | (buf[2] >> 3));
			pt->packet_data.escr.ESCR_base_3 = ((buf[2] & 0x3) << 13) | (buf[3] << 5) | (buf[4] >> 3);
			pt->packet_data.escr.ESCR_extension = (((buf[4] & 0x3) << 8) | (buf[5] >> 1));
			buf += 6;
			head_len += 6;
		}
		if (pt->packet_data.ES_rate_flag) {
			// ES_rate
			pt->packet_data.rate.ES_rate = (uint32_t)(buf[0] << 15 | buf[1] << 7 | buf[2] >> 1) & 0x3FFFFF;
			buf += 3;
			head_len += 3;
		}
		if (pt->packet_data.DSM_trick_mode_flag) {
			// DSM_trick_mode
			pt->packet_data.trick_mode.DSM_trick_mode_control = buf[0] >> 5;
			pt->packet_data.trick_mode.rep_cntrl = buf[0] & 0x1F;
			buf += 1;
			head_len += 1;
		}
		if (pt->packet_data.additional_copy_info_flag) {
			// additional_copy_info
			pt->packet_data.copy_info.additional_copy_info = TS_READ8_BITS(buf, 7, 1);
			buf += 1;
			head_len += 1;
		}
		if (pt->packet_data.PES_CRC_flag) {
			// PES_CRC
			pt->packet_data.previous_PES_packet_CRC = TS_READ16(buf);
			buf += 2;
			head_len += 2;
		}
		if (pt->packet_data.PES_extension_flag) {
			// PES_extension
			pt->packet_data.extension.PES_private_data_flag = buf[0] >> 7;
			pt->packet_data.extension.pack_header_field_flag = ((buf[0] >> 6) & 0x1);
			pt->packet_data.extension.program_packet_sequence_counter_flag = ((buf[0] >> 5) & 0x1);
			pt->packet_data.extension.P_STD_buffer_flag = ((buf[0] >> 4) & 0x1);
			pt->packet_data.extension.PES_extension_flag_2 = buf[0] & 0x1;
			buf += 1;
			head_len += 1;
			if (pt->packet_data.extension.PES_private_data_flag) {
				memcpy(pt->packet_data.extension.PES_private_data, buf, 16);
				buf += 16;
				head_len += 16;
			}
			if (pt->packet_data.extension.pack_header_field_flag) {
				pt->packet_data.extension.pack_head_length = buf[0];
				buf += 1;
				//parse_pack_header(buf);
				buf += pt->packet_data.extension.pack_head_length;
				head_len += (1 + pt->packet_data.extension.pack_head_length);
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
				head_len += 2;
			}
			if (pt->packet_data.extension.PES_extension_flag_2) {
				pt->packet_data.extension.PES_extension_field_length = buf[0] & 0x7F;
				buf += 1;
				buf += pt->packet_data.extension.PES_extension_field_length;
				head_len += (1 + pt->packet_data.extension.PES_extension_field_length);
			}
		}
		// stuffing_byte 0xFF
		buf += (9 + pt->packet_data.PES_header_data_length - head_len);
		head_len += (9 + pt->packet_data.PES_header_data_length - head_len);
		// no more than 32 stuffing bytes

		// PES_packet_data_byte
		pt->PES_packet_data_byte = buf;
		pt->PES_packet_length -= (head_len - 6);
	} else if ((pt->stream_id == stream_id_program_stream_map) || (pt->stream_id == stream_id_private_stream_2) ||
			   (pt->stream_id == stream_id_ECM_stream) || (pt->stream_id == stream_id_EMM_stream) ||
			   (pt->stream_id == stream_id_program_stream_directory) || (pt->stream_id == stream_id_H222_DSMCC_stream) ||
			   (pt->stream_id == stream_id_H222_typeE_stream)) {
		/* PES_packet_data_byte */
		pt->PES_packet_data_byte = buf;
	} else if (pt->stream_id == stream_id_padding_stream) {
		/* padding_byte, do not need parse */
		pt->PES_packet_data_byte = NULL;
		buf += pt->PES_packet_length;
	}
	
	if (pt->cb && pt->PES_packet_data_byte) {
		pt->cb(pid, pt->PES_packet_data_byte, pt->PES_packet_length, pt->priv);
	}
	/* elementary stream has one type content */
	return pt->PES_packet_length + 6;
}

static int pes_proc(uint16_t pid, uint8_t *pkt, uint16_t len)
{
	int ret = parse_pes_packet(pid, pkt, len);
	if (ret < 0) {
		// printf("error in parsing pes %d, len %d\n", pid, len);
		return -1;
	}
	return 0;
}

void dump_pes_private(pes_t *pt)
{
	if (pt->priv) {
		if (pt->tag == 0x59)
			dump_subtitles(pt->priv);
		else if (pt->tag == 0x56)
			dump_teletext(pt->priv);
	}
}

void free_pes_private(pes_t *pt)
{
	if (pt && pt->cb && pt->priv) {
		if (pt->tag == 0x59) {
			free_subtitles(pt->priv);
		} else if (pt->tag == 0x56) {
			free_teletext(pt->priv);
		}
		free(pt->priv);
	}
}

void dump_pes_infos(void)
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
		if (pes.list[i].priv) {
			dump_pes_private(&pes.list[i]);
		}
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
		pes_t *pes_list = (pes_t *)realloc(pes.list, (pes.pid_num + 1) * sizeof(pes_t));
		if (!pes_list) {return;}
		pes.list = pes_list;
		memset(&pes.list[pes.pid_num], 0, sizeof(pes_t));
		pes.list[pes.pid_num].type = stream_type;
		pes.list[pes.pid_num].pid = pid;
		pes.list[pes.pid_num].priv = NULL;
		pes.list[pes.pid_num].cb = NULL;
		pes.pid_index[pid] = pes.pid_num;
		//can be null at first
		// pes.list[pes.pid_num].cb = pes_fns[stream_type];
		pes.pid_num ++;

		filter_t *f = filter_alloc(pid);
		filter_set(f, &para, pes_proc);
	}
}

void unregister_pes_ops(void)
{
	filter_param_t para = {
		.depth = 1,
		.coff = {0},
		.mask = {0},
		.negate = {0},
	};
	filter_t *f = NULL;
	for (int i = 0; i < pes.pid_num; i ++) {
		f = filter_lookup(pes.list[i].pid, &para);
		if (f) {
			filter_free(f);
		}
		free_pes_private(pes.list + i);
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
		if ((pes.pid_bitmap[ pid / 64] & ((uint64_t) 1 << (pid % 64)))) {
			return true;
		}
	}
	return false;
}
