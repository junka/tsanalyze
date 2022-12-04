#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pes.h"
#include "ps.h"
#include "ts.h"
#include "descriptor.h"


int parse_system_header(uint8_t *pkt, uint16_t len, system_header *sh)
{
	uint8_t *buf = pkt;
	uint16_t l = len;

	sh->system_header_start_code = TS_READ32(buf);
	PL_STEP(buf, l, 4);
	sh->header_length = TS_READ16(buf);
	PL_STEP(buf, l, 2);
	sh->marker_bit = TS_READ32_BITS(buf, 1, 0);
	sh->rate_bound = TS_READ32_BITS(buf, 22, 1);
	sh->marker_bit1 = TS_READ32_BITS(buf, 1, 23);
	sh->audio_bound = TS_READ32_BITS(buf, 6, 24);
	sh->fixed_flag = TS_READ32_BITS(buf, 1, 30);
	sh->CSPS_flag = TS_READ32_BITS(buf, 1, 31);
	PL_STEP(buf, l, 4);
	sh->system_audio_lock_flag = TS_READ8_BITS(buf, 1, 0);
	sh->system_video_lock_flag = TS_READ8_BITS(buf, 1, 1);
	sh->marker_bit2 = TS_READ8_BITS(buf, 1, 2);
	sh->video_bound = TS_READ8_BITS(buf, 5, 3);
	PL_STEP(buf, l, 1);
	sh->packet_rate_restriction_flag = TS_READ8_BITS(buf, 1, 0);
	PL_STEP(buf, l, 1);
	//if next bit start with 1, then there is estd es;
	if (TS_READ_BIT(buf, 7)) {
		sh->es.stream_id = TS_READ8(buf);
		PL_STEP(buf, l, 1);
		sh->es.PSTD_buffer_bound_scale = TS_READ16_BITS(buf, 1, 2);
		sh->es.PSTD_buffer_size_bound = TS_READ16_BITS(buf, 13, 3);
		PL_STEP(buf, l, 2);
	}
	return sh->header_length + 6;
}

int parse_pack(uint16_t pid, uint8_t *pkt, uint16_t len, pack_header *ph)
{
	uint8_t *buf = pkt;
	uint16_t l = len;

	ph->pack_start_code = TS_READ32(buf);
	PL_STEP(buf, l, 4);
	ph->md = TS_READ64(buf);
	PL_STEP(buf, l, 8);

	ph->program_mux_rate_l = TS_READ8_BITS(buf, 6, 0);
	ph->marker_bit5 = TS_READ8_BITS(buf, 1, 6);
	ph->marker_bit6 = TS_READ8_BITS(buf, 1, 7);

	PL_STEP(buf, l, 1);

	ph->pack_stuffing_length = TS_READ8_BITS(buf, 5, 0);
	
	PL_STEP(buf, l, 1);
	ph->stuffing_byte = (uint8_t *)malloc(ph->pack_stuffing_length);
	memcpy(ph->stuffing_byte, buf, ph->pack_stuffing_length);

	PL_STEP(buf, l, ph->pack_stuffing_length);

	system_header sh;
	uint32_t next_head = TS_READ32(buf);
	int ret = 0;
	if (next_head == SYSTEM_START) {
		ret = parse_system_header(buf, l, &sh);
		PL_STEP(buf, l, ret);
		next_head = TS_READ32(buf);
	}

	while(((next_head >> 8) == PES_PACKET_START) && 
		((next_head & 0xFF) != stream_id_program_stream_end))
	{
		ret = parse_pes_packet(pid, buf, l);

		PL_STEP(buf, l, ret);
		next_head = TS_READ32(buf);
	}

	return len - l;
}

int parse_program_stream_map(uint8_t *pkt, uint16_t len, ps_map *map)
{
	uint8_t * buf = pkt;
	int l = len;
	map->packet_start_code_prefix = TS_READ32_BITS(buf, 24, 0);
	map->map_stream_id = TS_READ32_BITS(buf, 8, 24);
	
	PL_STEP(buf, l, 4);
	map->program_stream_map_length = TS_READ16(buf);
	
	PL_STEP(buf, l, 2);
	map->current_next_indicator = TS_READ8_BITS(buf, 1, 0);
	map->program_stream_map_version = TS_READ8_BITS(buf, 5, 3);
	
	PL_STEP(buf, l, 1);
	map->marker_bit = TS_READ8_BITS(buf, 1, 7);
	
	PL_STEP(buf, l, 1);
	map->program_stream_info_length = TS_READ16(buf);
	
	PL_STEP(buf, l, 2);
	parse_descriptors(&map->list, buf, map->program_stream_info_length);
	buf += map->program_stream_info_length;
	l -= map->program_stream_info_length;
	map->elementary_stream_map_length = TS_READ16(buf);
	
	PL_STEP(buf, l, 2);
	int k = 0;
	list_head_init(&map->h);
	while(k < map->elementary_stream_map_length) {
		es_map * node = malloc(sizeof(es_map));
		list_head_init(&node->list);
		list_node_init(&node->n);
		node->stream_type = TS_READ8(buf);
		k += 1;
		buf += 1;
		node->elementary_stream_id = TS_READ8(buf);
		k += 1;
		buf += 1;
		node->elementary_stream_info_length = TS_READ16(buf);
		k += 2;
		buf += 2;
		parse_descriptors(&node->list, buf, node->elementary_stream_info_length);
		list_add_tail(&map->h, &node->n);
		k += node->elementary_stream_info_length;
		buf += node->elementary_stream_info_length;
	}
	l -= map->elementary_stream_map_length;
	PL_STEP(buf, l, 4);

	return len - l;
}

int parse_directory_PES_packet(uint8_t *pkt, uint16_t len, directory_PES_packet *dpp)
{
	uint8_t * buf = pkt;
	int l = len;
	dpp->packet_start_code_prefix = TS_READ32_BITS(buf, 24, 0);
	dpp->directory_stream_id = TS_READ32_BITS(buf, 8, 24);
	PL_STEP(buf, l, 4);
	dpp->PES_packet_length = TS_READ16(buf);
	PL_STEP(buf, l, 2);
	dpp->number_of_access_units = TS_READ16_BITS(buf, 15, 0);
	dpp->marker_bit = TS_READ16_BITS(buf, 1, 15);
	PL_STEP(buf, l, 2);
	dpp->prev_directory_offset1 = TS_READ16_BITS(buf, 15, 0);
	dpp->marker_bit1 = TS_READ16_BITS(buf, 1, 15);
	PL_STEP(buf, l, 2);
	dpp->prev_directory_offset2 = TS_READ16_BITS(buf, 15, 0);
	dpp->marker_bit2 = TS_READ16_BITS(buf, 1, 15);
	PL_STEP(buf, l, 2);
	dpp->prev_directory_offset3 = TS_READ16_BITS(buf, 15, 0);
	dpp->marker_bit3 = TS_READ16_BITS(buf, 1, 15);
	PL_STEP(buf, l, 2);
	dpp->prev_directory_offset4 = TS_READ16_BITS(buf, 15, 0);
	dpp->marker_bit4 = TS_READ16_BITS(buf, 1, 15);
	PL_STEP(buf, l, 2);
	dpp->prev_directory_offset5 = TS_READ16_BITS(buf, 15, 0);
	dpp->marker_bit5 = TS_READ16_BITS(buf, 1, 15);
	PL_STEP(buf, l, 2);
	dpp->prev_directory_offset6 = TS_READ16_BITS(buf, 15, 0);
	dpp->marker_bit6 = TS_READ16_BITS(buf, 1, 15);
	PL_STEP(buf, l, 2);
	dpp->units = malloc(sizeof(access_unit) * dpp->number_of_access_units);
	int i = 0;
	while(i < dpp->number_of_access_units) {
		dpp->units[i].packet_stream_id = TS_READ8(buf);
		PL_STEP(buf, l, 1);
		dpp->units[i].PES_header_position_offset_sign = TS_READ16_BITS(buf, 1, 0);
		dpp->units[i].PES_header_position_offset1 = TS_READ16_BITS(buf, 14, 1);
		dpp->units[i].marker_bit1 = TS_READ16_BITS(buf, 1, 15);
		PL_STEP(buf, l, 2);

		dpp->units[i].PES_header_position_offset2 = TS_READ16_BITS(buf, 15, 1);
		dpp->units[i].marker_bit2 = TS_READ16_BITS(buf, 1, 15);
		PL_STEP(buf, l, 2);

		dpp->units[i].PES_header_position_offset3 = TS_READ16_BITS(buf, 15, 1);
		dpp->units[i].marker_bit3 = TS_READ16_BITS(buf, 1, 15);
		PL_STEP(buf, l, 2);

		dpp->units[i].reference_offset = TS_READ16(buf);
		PL_STEP(buf, l, 2);

		dpp->units[i].marker_bit4 = TS_READ8_BITS(buf, 1, 0);
		dpp->units[i].PTS1 = TS_READ8_BITS(buf, 4, 1);
		dpp->units[i].marker_bit5 = TS_READ8_BITS(buf, 1, 7);
		PL_STEP(buf, l, 1);

		dpp->units[i].PTS2 = TS_READ16_BITS(buf, 15, 1);
		dpp->units[i].marker_bit6 = TS_READ16_BITS(buf, 1, 15);
		PL_STEP(buf, l, 2);

		dpp->units[i].PTS3 = TS_READ16_BITS(buf, 15, 1);
		dpp->units[i].marker_bit7 = TS_READ16_BITS(buf, 1, 15);
		PL_STEP(buf, l, 2);

		dpp->units[i].bytes_to_read1 = TS_READ16_BITS(buf, 15, 1);
		dpp->units[i].marker_bit8 = TS_READ16_BITS(buf, 1, 15);
		PL_STEP(buf, l, 2);

		dpp->units[i].bytes_to_read2 = TS_READ8(buf);
		PL_STEP(buf, l, 1);

		dpp->units[i].marker_bit9 = TS_READ8_BITS(buf, 1, 0);
		dpp->units[i].intra_coded_indicator = TS_READ8_BITS(buf, 1, 1);
		dpp->units[i].coding_parameters_indicator = TS_READ8_BITS(buf, 2, 2);
		PL_STEP(buf, l, 1);
		
	}
	return len - l;
}

/* see 2.4.3.8 */
/* see table 2-31 in iso 13818-1 */
int parse_ps(uint16_t pid, uint8_t *pkt, uint16_t len)
{
	uint8_t *buf = pkt;
	pack_header ph;
	uint16_t l = len;

	while (TS_READ32(buf) == PACK_START) {
		int ret = parse_pack(pid, buf, l, &ph);
		PL_STEP(buf, l, ret);
	}
	return (TS_READ32(buf) == PROGRAM_END);
}
