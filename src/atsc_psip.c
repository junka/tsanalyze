#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "descriptor.h"
#include "result.h"
#include "table.h"
#include "table_priv.h"
#include "ts.h"
#include "utils.h"

/* marks which PSIP tables were successfully parsed (see atsc_psip_proc) */
#define PSIP_MGT   (1 << 0)
#define PSIP_TVCT  (1 << 1)
#define PSIP_CVCT  (1 << 2)
#define PSIP_RRT   (1 << 3)
#define PSIP_EIT   (1 << 4)
#define PSIP_ETT   (1 << 5)
#define PSIP_STT   (1 << 6)
#define PSIP_DCCT  (1 << 7)
#define PSIP_DCCSCT (1 << 8)

static atsc_psip_t psip;
static uint16_t psip_seen_mask;

static void free_multi_string(struct multiple_string *str)
{
	for (int i = 0; i < str->number_strings; i ++) {
		for (int j = 0; j < str->strings[i].number_segments; j++) {
			free(str->strings[i].segments[j].compressed_string_byte);
		}
		free(str->strings[i].segments);
	}
	free(str->strings);
}

static int parse_multi_string(uint8_t *pbuf, struct multiple_string *str)
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

void dump_atsc_tables(void)
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

bool atsc_tables_seen(void)
{
	return psip_seen();
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

static int parse_vct(uint8_t *pbuf, uint16_t buf_size, atsc_vct_t *vct, int cable)
{
	uint8_t *pdata = pbuf;

	int ret = parse_section_header(pbuf, buf_size, &vct->vct_header);
	if (ret != 0)
		return ret;

	if (vct->channels) {
		for (int i = 0; i < vct->num_channels_in_section; i ++) {
			if (!list_empty(&vct->channels[i].list)) {
				free_descriptors(&vct->channels[i].list);
			}
		}
		free(vct->channels);
	}
	if (!list_empty(&vct->list))
		free_descriptors(&vct->list);

	pdata = vct->vct_header.private_data_byte;
	vct->protocol_version = TS_READ8(pdata);
	pdata += 1;
	vct->num_channels_in_section = TS_READ8(pdata);
	pdata += 1;
	vct->channels = calloc(vct->num_channels_in_section, sizeof(struct define_channel));
	if (!vct->channels) {
		return ENOMEM;
	}
	for (int i = 0; i < vct->num_channels_in_section; i ++) {
		/* TVCT copies 7*sizeof(uint16_t); CVCT historically copies 7*16,
		 * keep that behaviour identical to the original single file. */
		memcpy(vct->channels[i].short_name, pdata, cable ? 7 * 16 : 7 * sizeof(uint16_t));
		pdata += cable ? 7 * 16 : 7 * sizeof(uint16_t);
		vct->channels[i].major_channel_number = TS_READ32_BITS(pdata, 10, 4);
		vct->channels[i].minor_channel_number = TS_READ32_BITS(pdata, 10, 14);
		vct->channels[i].modulation_mode = TS_READ32_BITS(pdata, 8, 24);
		pdata += 4;
		vct->channels[i].carrier_frequency = TS_READ32(pdata);
		pdata += 4;
		vct->channels[i].channel_TSID = TS_READ16(pdata);
		pdata += 2;
		vct->channels[i].program_number = TS_READ16(pdata);
		pdata += 2;
		vct->channels[i].ETM_location = TS_READ16_BITS(pdata, 2, 0);
		vct->channels[i].access_controlled = TS_READ16_BITS(pdata, 1, 2);
		vct->channels[i].hidden = TS_READ16_BITS(pdata, 1, 3);
		if (cable)
			vct->channels[i].path_select = TS_READ16_BITS(pdata, 1, 4);
		if (cable)
			vct->channels[i].out_of_band = TS_READ16_BITS(pdata, 1, 5);
		vct->channels[i].hide_guide = TS_READ16_BITS(pdata, 1, 6);
		vct->channels[i].service_type = TS_READ16_BITS(pdata, 6, 10);
		pdata += 2;
		vct->channels[i].source_id = TS_READ16(pdata);
		pdata += 2;
		vct->channels[i].descriptors_length = TS_READ16_BITS(pdata, 10, 6);
		pdata += 2;
		list_head_init(&vct->channels[i].list);
		parse_descriptors(&vct->channels[i].list, pdata, vct->channels[i].descriptors_length);
		pdata += vct->channels[i].descriptors_length;
	}
	vct->additional_descriptors_length = TS_READ16_BITS(pdata, 10, 6);
	pdata += 2;
	list_head_init(&vct->list);
	parse_descriptors(&vct->list, pdata, vct->additional_descriptors_length);
	pdata += vct->additional_descriptors_length;
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

int atsc_psip_proc(uint16_t pid __maybe_unused, uint8_t *pkt, uint16_t len)
{
	switch (pkt[0]) {
		case MGT_TID:
			parse_mgt(pkt, len, &psip.mgt);
			psip_seen_mask |= PSIP_MGT;
			break;
		case TVCT_TID:
			parse_vct(pkt, len, &psip.tvct, 0);
			psip_seen_mask |= PSIP_TVCT;
			break;
		case CVCT_TID:
			parse_vct(pkt, len, &psip.cvct, 1);
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

void atsc_init_tables(void)
{
	memset(&psip, 0, sizeof(psip));
	psip_seen_mask = 0;
	list_head_init(&psip.mgt.list);
	list_head_init(&psip.tvct.list);
	list_head_init(&psip.cvct.list);
	list_head_init(&psip.rrt.list);
	list_head_init(&psip.stt.list);
	list_head_init(&psip.dcct.list);
	list_head_init(&psip.dccsct.list);
}