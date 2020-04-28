#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "filter.h"
#include "ts.h"
#include "pes.h"

#define PES_MAX_LENGTH 64 * 1024 * 1024

int parse_pes(uint8_t *pkt, uint16_t len)
{
	pes_t pes;
	uint8_t *buf = pkt;
	pes.packet_start_code_prefix = ((uint32_t)buf[0] << 16 | (uint32_t)buf[1] << 8 | buf[2]);
	pes.stream_id = buf[3];
	buf += 4;
	pes.PES_packet_length = TS_READ16(buf);
	buf += 2;
	if ((pes.stream_id != program_stream_map) && (pes.stream_id != padding_stream) &&
		(pes.stream_id != private_stream_2) && (pes.stream_id != ECM_stream) && (pes.stream_id != EMM_stream) &&
		(pes.stream_id != program_stream_directory) && (pes.stream_id != DSMCC_stream) && (pes.stream_id != H222_1_E)) {
		pes.packet_data.PES_scrambling_control = (buf[0] >> 4) & 0x3;
		pes.packet_data.PES_priority = (buf[0] >> 3) & 0x1;
		pes.packet_data.data_alignment_indicator = (buf[0] >> 2) & 0x1;
		pes.packet_data.copyright = (buf[0] >> 1) & 0x1;
		pes.packet_data.original_or_copy = (buf[0]) & 0x1;
		buf += 1;
		pes.packet_data.PTS_DTS_flags = (buf[0] >> 6) & 0x3;
		pes.packet_data.ESCR_flag = (buf[0] >> 5) & 0x1;
		pes.packet_data.ES_rate_flag = (buf[0] >> 4) & 0x1;
		pes.packet_data.DSM_trick_mode_flag = (buf[0] >> 3) & 0x1;
		pes.packet_data.additional_copy_info_flag = (buf[0] >> 2) & 0x1;
		pes.packet_data.PES_CRC_flag = (buf[0] >> 1) & 0x1;
		pes.packet_data.PES_extension_flag = (buf[0]) & 0x1;
		buf += 1;
		pes.packet_data.PES_header_data_length = buf[0];
		buf += 1;
		if ((pes.packet_data.PTS_DTS_flags & 0x2) == 0x2) {
			// PTS
			pes.packet_data.pts.flag = 0x2;
			pes.packet_data.pts.PTS_DTS_1 = (buf[0] >> 1) & 0x7;
			buf += 1;
			pes.packet_data.pts.PTS_DTS_2 = TS_READ16(buf) >> 1;
			buf += 2;
			pes.packet_data.pts.PTS_DTS_3 = TS_READ16(buf) >> 1;
			buf += 2;
		}
		if ((pes.packet_data.PTS_DTS_flags & 0x1) == 0x1) {
			// DTS
			pes.packet_data.dts.flag = 0x3;
			pes.packet_data.dts.PTS_DTS_1 = (buf[0] >> 1) & 0x7;
			buf += 1;
			pes.packet_data.dts.PTS_DTS_2 = TS_READ16(buf) >> 1;
			buf += 2;
			pes.packet_data.dts.PTS_DTS_3 = TS_READ16(buf) >> 1;
			buf += 2;
		}
		if (pes.packet_data.ESCR_flag) {
			// ESCR
			pes.packet_data.escr.ESCR_base_1 = (buf[0] >> 3) & 0x7;
			pes.packet_data.escr.ESCR_base_2 = (((buf[0] & 0x3) << 13) | (buf[1] << 5) | (buf[2] >> 3));
			pes.packet_data.escr.ESCR_base_3 = ((buf[2] & 0x3) << 13) | (buf[3] << 5) | (buf[4] >> 3);
			pes.packet_data.escr.ESCR_extension = (((buf[4] & 0x3) << 8) | (buf[5] >> 1));
			buf += 6;
		}
		if (pes.packet_data.ES_rate_flag) {
			// ES_rate
			pes.packet_data.rate.ES_rate = (uint32_t)(buf[0] << 15 | buf[1] << 7 | buf[2] >> 1) & 0x3FFFFF;
			buf += 3;
		}
		if (pes.packet_data.DSM_trick_mode_flag) {
			// DSM_trick_mode
			pes.packet_data.trick_mode.DSM_trick_mode_control = buf[0] >> 5;
			pes.packet_data.trick_mode.rep_cntrl = buf[0] & 0x1F;
			buf += 1;
		}
		if (pes.packet_data.additional_copy_info_flag) {
			// additional_copy_info
			pes.packet_data.copy_info.additional_copy_info = buf[0] & 0x7F;
			buf += 1;
		}
		if (pes.packet_data.PES_CRC_flag) {
			// PES_CRC
			pes.packet_data.previous_PES_packet_CRC = (buf[0] << 8 | buf[1]);
			buf += 2;
		}
		if (pes.packet_data.PES_extension_flag) {
			// PES_extension
			pes.packet_data.extension.PES_private_data_flag = buf[0] >> 7;
			pes.packet_data.extension.pack_header_field_flag = ((buf[0] >> 6) & 0x1);
			pes.packet_data.extension.program_packet_sequence_counter_flag = ((buf[0] >> 5) & 0x1);
			pes.packet_data.extension.P_STD_buffer_flag = ((buf[0] >> 4) & 0x1);
			pes.packet_data.extension.PES_extension_flag_2 = buf[0] & 0x1;
			buf += 1;
			if (pes.packet_data.extension.PES_private_data_flag) {
				memcpy(pes.packet_data.extension.PES_private_data, buf, 16);
				buf += 16;
			}
			if (pes.packet_data.extension.pack_header_field_flag) {
				pes.packet_data.extension.pack_head_length = buf[0];
				buf += 1;
				buf += pes.packet_data.extension.pack_head_length;
			}
			if (pes.packet_data.extension.program_packet_sequence_counter_flag) {
				pes.packet_data.extension.ppsc.program_packet_sequence_counter = buf[0] & 0x7F;
				pes.packet_data.extension.ppsc.MPEG1_MPEG2_identifier = (buf[1] >> 6) & 0x1;
				pes.packet_data.extension.ppsc.original_stuff_length = buf[1] & 0x3F;
				buf += 2;
			}
			if (pes.packet_data.extension.P_STD_buffer_flag) {
				pes.packet_data.extension.pstd_buffer.PSTD_buffer_scale = (buf[0] >> 5) & 0x1;
				pes.packet_data.extension.pstd_buffer.PSTD_buffer_size = (((buf[0] & 0x1F) << 8) | buf[1]);
				buf += 2;
			}
			if (pes.packet_data.extension.PES_extension_flag_2) {
				pes.packet_data.extension.PES_extension_field_length = buf[0] & 0x7F;
				buf += 1;
				buf += pes.packet_data.extension.PES_extension_field_length;
			}
		}
		// stuffing_byte 0xFF
		// no more than 32 stuffing bytes

		// PES_packet_data_byte
	} else if ((pes.stream_id == program_stream_map) || (pes.stream_id == private_stream_2) ||
			   (pes.stream_id == ECM_stream) || (pes.stream_id == EMM_stream) ||
			   (pes.stream_id == program_stream_directory) || (pes.stream_id == DSMCC_stream) ||
			   (pes.stream_id == H222_1_E)) {
		// PES_packet_data_byte
		pes.PES_packet_data_byte = buf;
	} else if (pes.stream_id == padding_stream) {
		// padding_byte
		pes.padding_byte = buf;
	}
	// elementary stream has one type content
	return 0;
}

static int pes_proc(uint16_t pid, uint8_t *pkt, uint16_t len)
{
	parse_pes(pkt, len);
	return 0;
}

void register_pes_ops(uint16_t pid)
{
	if (pid == NIT_PID)
		return;
	filter_t *f = filter_alloc(pid);
	filter_param_t para;
	para.depth = 1;
	para.coff[0] = 0xFF;
	para.mask[0] = 0;
	filter_set(f, &para, pes_proc);
}
