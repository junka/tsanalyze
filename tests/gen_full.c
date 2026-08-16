/*
 * Comprehensive TS stream generator for the tsanalyze ctest suite.
 *
 * Produces the full set of MPEG-TS sample streams (merged from the former
 * gen_stream.c and gen_full.c) into a directory passed as argv[1]:
 *
 *   - robustness:  deliberately ultra-short / malformed streams (empty,
 *                  truncated, garbage, dangling sync) that exercise parser
 *                  robustness against out-of-spec input
 *   - dvb_all.ts      all DVB SI tables (PAT/CAT/TSDT/PMT/NIT/SDT/BAT/EIT/TDT/TOT)
 *                     plus special DVB descriptors (subtitling, teletext, CA)
 *   - dvb_multi_pmt.ts  multiple programs / multiple PMTs, one PMT spanning
 *                     several TS packets (multi-packet section reassembly)
 *   - isdb_stream.ts  DVB table structure populated with ISDB-specific
 *                     descriptors (ARIB STD-B10)
 *   - atsc_psip.ts    ATSC PSIP tables (MGT/TVCT/CVCT/RRT/STT/EIT/ETT/DCCT/DCCSCT)
 *                     carried on PID 0x1FFB
 *
 * Every stream contains at least 10 TS packets (>= 2040 bytes) so that the
 * tsanalyze probe recognises it as a valid 188-byte transport stream.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define TS_PKT 188
#define PAYLOAD_MAX (TS_PKT - 4)
#define NULL_PID 0x1FFF

/* ---- CRC-32 (MPEG-2 / IEEE, poly 0x04C11DB7) ---- */
static uint32_t crc_table[256];

static void crc_init(void)
{
	for (uint32_t i = 0; i < 256; i++) {
		uint32_t c = i << 24;
		for (int k = 0; k < 8; k++)
			c = (c & 0x80000000) ? (c << 1) ^ 0x04C11DB7 : (c << 1);
		crc_table[i] = c;
	}
}

static uint32_t crc32(const uint8_t *data, size_t len)
{
	uint32_t crc = 0;
	for (size_t i = 0; i < len; i++)
		crc = (crc << 8) ^ crc_table[((crc >> 24) ^ data[i]) & 0xFF];
	return crc;
}

static void write_file(const char *dir, const char *name, const uint8_t *buf, size_t len)
{
	char path[1024];
	snprintf(path, sizeof(path), "%s/%s", dir, name);
	FILE *f = fopen(path, "wb");
	if (f == NULL) {
		perror(path);
		exit(1);
	}
	if (len && fwrite(buf, 1, len, f) != len) {
		perror(path);
		exit(1);
	}
	fclose(f);
}

/* ---- helpers for the ultra-short / malformed streams ---- */

/* build a TS packet with a 4-byte header and a payload filled to 188 bytes */
static void ts_packet(uint8_t *out, uint16_t pid, int pusi,
		      const uint8_t *payload, size_t payload_len)
{
	memset(out, 0xFF, TS_PKT); /* stuffing */
	out[0] = 0x47;		       /* sync byte */
	out[1] = (uint8_t)((pusi ? (1 << 6) : 0) | ((pid >> 8) & 0x1F));
	out[2] = (uint8_t)(pid & 0xFF);
	out[3] = 0x10; /* payload only, continuity counter 0 */
	if (payload && payload_len)
		memcpy(out + 4, payload, payload_len);
}

/* one valid PAT section (16 bytes), table_id 0x00, one program -> PMT pid 0x10 */
static size_t build_pat_section(uint8_t *sec)
{
	const uint16_t sec_len = 13; /* bytes after the length field, incl. CRC */
	sec[0] = 0x00;		     /* table_id = PAT */
	sec[1] = (uint8_t)(0x80 | ((sec_len >> 8) & 0x0F));
	sec[2] = (uint8_t)(sec_len & 0xFF);
	sec[3] = 0x00; /* transport_stream_id hi */
	sec[4] = 0x01; /* transport_stream_id lo */
	sec[5] = 0xC1; /* version 0, current_next 1 */
	sec[6] = 0x00; /* section_number */
	sec[7] = 0x00; /* last_section_number */
	sec[8] = 0x00; /* program_number hi */
	sec[9] = 0x01; /* program_number lo */
	sec[10] = 0xE0 | ((0x10 >> 8) & 0x1F);
	sec[11] = 0x10; /* program_map_PID lo */
	uint32_t crc = crc32(sec, 12);
	sec[12] = (uint8_t)(crc >> 24);
	sec[13] = (uint8_t)(crc >> 16);
	sec[14] = (uint8_t)(crc >> 8);
	sec[15] = (uint8_t)(crc);
	return 16;
}

/* a valid stream: 1 PAT packet followed by NULL packets, 12 packets total */
static void build_valid_pat(uint8_t *stream)
{
	uint8_t sec[16];
	size_t sec_len = build_pat_section(sec);
	uint8_t payload[1 + 16];
	payload[0] = 0x00; /* PSI pointer_field */
	memcpy(payload + 1, sec, sec_len);
	ts_packet(stream, 0x0000, 1, payload, 1 + sec_len);
	for (int i = 1; i < 12; i++)
		ts_packet(stream + i * TS_PKT, NULL_PID, 0, NULL, 0);
}

/* ultra-short / malformed streams that exercise parser robustness */
static void build_robustness(const char *dir)
{
	/* empty file */
	write_file(dir, "empty.ts", NULL, 0);

	/* ultra-short files well below the 10-packet probe threshold (2040 B) */
	{
		uint8_t b[1] = { 0x47 };
		write_file(dir, "short_1.ts", b, 1);
	}
	{
		uint8_t b[10];
		memset(b, 0xFF, sizeof(b));
		write_file(dir, "short_10.ts", b, sizeof(b));
	}
	{
		uint8_t b[100];
		memset(b, 0x47, sizeof(b));
		write_file(dir, "short_100.ts", b, sizeof(b));
	}
	{
		uint8_t b[1000];
		memset(b, 0xAA, sizeof(b));
		write_file(dir, "short_1000.ts", b, sizeof(b));
	}

	/* garbage just at the probe threshold, no valid sync pattern */
	{
		uint8_t b[2040];
		memset(b, 0xAA, sizeof(b));
		write_file(dir, "garbage_2040.ts", b, sizeof(b));
	}

	/* a plain valid stream: 12 packets (1 PAT + NULL), used by stream_valid_pat */
	{
		uint8_t b[12 * TS_PKT];
		build_valid_pat(b);
		write_file(dir, "valid_pat.ts", b, sizeof(b));
	}

	/* a truncated stream: 11 full packets + 100 bytes of a 12th (cut mid-payload) */
	{
		uint8_t b[12 * TS_PKT];
		build_valid_pat(b);
		write_file(dir, "truncated.ts", b, 11 * TS_PKT + 100);
	}

	/* a stream with a valid sync pattern but oversized header claims (malformed) */
	{
		uint8_t b[12 * TS_PKT + 1];
		build_valid_pat(b);
		b[12 * TS_PKT] = 0x47; /* dangling trailing sync */
		write_file(dir, "valid_pat_trailing.ts", b, sizeof(b));
	}
}

/* ---- section building ---- */

/* syntax-section builder. Returns total section length (bytes to transmit). */
static int sec_build(uint8_t *out, uint8_t table_id, uint16_t ext, uint8_t version,
		     uint8_t current, uint8_t secnum, uint8_t last_secnum,
		     const uint8_t *body, int body_len)
{
	int len = body_len + 9; /* section_length: 5 + body + crc */
	out[0] = table_id;
	out[1] = (uint8_t)(0xB0 | ((len >> 8) & 0x0F));
	out[2] = (uint8_t)(len & 0xFF);
	out[3] = (uint8_t)(ext >> 8);
	out[4] = (uint8_t)(ext & 0xFF);
	out[5] = (uint8_t)(0xC0 | ((version << 1) & 0x3E) | (current & 1));
	out[6] = secnum;
	out[7] = last_secnum;
	if (body_len)
		memcpy(out + 8, body, body_len);
	uint32_t crc = crc32(out, 8 + body_len);
	out[8 + body_len] = (uint8_t)(crc >> 24);
	out[9 + body_len] = (uint8_t)(crc >> 16);
	out[10 + body_len] = (uint8_t)(crc >> 8);
	out[11 + body_len] = (uint8_t)(crc);
	return 12 + body_len;
}

static int desc(uint8_t *buf, int off, uint8_t tag, const uint8_t *payload, int len)
{
	buf[off] = tag;
	buf[off + 1] = (uint8_t)len;
	if (len)
		memcpy(buf + off + 2, payload, len);
	return off + 2 + len;
}

/* ---- TS packet emission with section fragmentation across packets ---- */
static void emit_section(uint8_t *stream, size_t *pos, uint16_t pid, int *cc,
			 const uint8_t *sec, int sec_len)
{
	int remaining = sec_len;
	int first = 1;
	while (remaining > 0) {
		uint8_t pkt[TS_PKT];
		memset(pkt, 0xFF, TS_PKT);
		pkt[0] = 0x47;
		pkt[1] = (uint8_t)(((pid >> 8) & 0x1F) | (first ? 0x40 : 0)); /* PUSI */
		pkt[2] = (uint8_t)(pid & 0xFF);
		pkt[3] = (uint8_t)(0x10 | ((*cc) & 0x0F)); /* payload only */
		(*cc)++;
		int n;
		if (first) {
			n = PAYLOAD_MAX - 1; /* 1-byte pointer_field */
			if (n > remaining)
				n = remaining;
			pkt[4] = 0; /* pointer_field: section starts here */
			memcpy(pkt + 5, sec, n);
			first = 0;
		} else {
			n = PAYLOAD_MAX;
			if (n > remaining)
				n = remaining;
			memcpy(pkt + 4, sec, n);
		}
		sec += n;
		remaining -= n;
		memcpy(stream + *pos, pkt, TS_PKT);
		*pos += TS_PKT;
	}
}

/* pad a stream with proper NULL packets until it is at least 'min_bytes' long */
static void pad_null(uint8_t *stream, size_t *pos, size_t min_bytes)
{
	while (*pos < min_bytes) {
		uint8_t pkt[TS_PKT];
		memset(pkt, 0xFF, TS_PKT);
		pkt[0] = 0x47;      /* sync byte */
		pkt[1] = 0x1F;      /* NULL PID hi */
		pkt[2] = 0xFF;      /* NULL PID lo */
		pkt[3] = 0x10;      /* payload only */
		memcpy(stream + *pos, pkt, TS_PKT);
		*pos += TS_PKT;
	}
}

/* ====================================================================
 * DVB descriptors
 * ==================================================================== */

/* CA descriptor 0x09 */
static int d_ca(uint8_t *b, int o, uint16_t ca_sys, uint16_t ca_pid)
{
	uint8_t p[6];
	p[0] = (uint8_t)(ca_sys >> 8);
	p[1] = (uint8_t)(ca_sys & 0xFF);
	p[2] = (uint8_t)(0xE0 | ((ca_pid >> 8) & 0x1F));
	p[3] = (uint8_t)(ca_pid & 0xFF);
	p[4] = 0xAA; /* private data */
	p[5] = 0xBB;
	return desc(b, o, 0x09, p, 6);
}

/* network_name 0x40 */
static int d_network_name(uint8_t *b, int o, const char *name)
{
	return desc(b, o, 0x40, (const uint8_t *)name, (int)strlen(name));
}

/* satellite_delivery_system 0x43 (11 bytes) */
static int d_satellite(uint8_t *b, int o)
{
	uint8_t p[11];
	p[0] = 0x00; p[1] = 0x0F; p[2] = 0xA0; p[3] = 0x00; /* frequency */
	p[4] = 0x01; p[5] = 0xF0; /* orbital position */
	p[6] = 0x02; /* west=0 pol=0 rolloff=0 modsys=0 modtype=2 */
	uint32_t sym = (0x00007530u << 4) | 2u; /* symbol_rate 28b + FEC_inner 4b */
	p[7] = (uint8_t)(sym >> 24);
	p[8] = (uint8_t)(sym >> 16);
	p[9] = (uint8_t)(sym >> 8);
	p[10] = (uint8_t)(sym);
	return desc(b, o, 0x43, p, 11);
}

/* service_list 0x41: one or more [service_id 2][service_type 1] */
static int d_service_list(uint8_t *b, int o, uint16_t sid, uint8_t type)
{
	uint8_t p[3];
	p[0] = (uint8_t)(sid >> 8);
	p[1] = (uint8_t)(sid & 0xFF);
	p[2] = type;
	return desc(b, o, 0x41, p, 3);
}

/* service 0x48 */
static int d_service(uint8_t *b, int o, uint8_t type, const char *prov, const char *name)
{
	uint8_t p[64];
	int pl = (int)strlen(prov), nl = (int)strlen(name);
	p[0] = type;
	p[1] = (uint8_t)pl;
	memcpy(p + 2, prov, pl);
	p[2 + pl] = (uint8_t)nl;
	memcpy(p + 3 + pl, name, nl);
	return desc(b, o, 0x48, p, 3 + pl + nl);
}

/* bouquet_name 0x47 */
static int d_bouquet_name(uint8_t *b, int o, const char *name)
{
	return desc(b, o, 0x47, (const uint8_t *)name, (int)strlen(name));
}

/* short_event 0x4D */
static int d_short_event(uint8_t *b, int o, const char *lang, const char *ename, const char *text)
{
	uint8_t p[128];
	int oo = 0;
	p[oo++] = lang[0]; p[oo++] = lang[1]; p[oo++] = lang[2];
	int el = (int)strlen(ename);
	p[oo++] = (uint8_t)el;
	memcpy(p + oo, ename, el); oo += el;
	int tl = (int)strlen(text);
	p[oo++] = (uint8_t)tl;
	memcpy(p + oo, text, tl); oo += tl;
	return desc(b, o, 0x4D, p, oo);
}

/* subtitling 0x59: one 8-byte node */
static int d_subtitling(uint8_t *b, int o, const char *lang, uint8_t type, uint16_t comp, uint16_t anc)
{
	uint8_t p[8];
	p[0] = lang[0]; p[1] = lang[1]; p[2] = lang[2];
	p[3] = type;
	p[4] = (uint8_t)(comp >> 8); p[5] = (uint8_t)(comp);
	p[6] = (uint8_t)(anc >> 8); p[7] = (uint8_t)(anc);
	return desc(b, o, 0x59, p, 8);
}

/* teletext 0x56: one 5-byte node */
static int d_teletext(uint8_t *b, int o, const char *lang, uint8_t type, uint8_t page)
{
	uint8_t p[5];
	p[0] = lang[0]; p[1] = lang[1]; p[2] = lang[2];
	p[3] = (uint8_t)((type << 3) | 0x01); /* type 5b + magazine 3b */
	p[4] = page;
	return desc(b, o, 0x56, p, 5);
}

/* registration 0x05 */
static int d_registration(uint8_t *b, int o, uint32_t fmt)
{
	uint8_t p[4];
	p[0] = (uint8_t)(fmt >> 24); p[1] = (uint8_t)(fmt >> 16);
	p[2] = (uint8_t)(fmt >> 8); p[3] = (uint8_t)(fmt);
	return desc(b, o, 0x05, p, 4);
}

/* local_time_offset 0x58: one 12-byte node */
static int d_local_time_offset(uint8_t *b, int o)
{
	uint8_t p[12];
	p[0] = 'C'; p[1] = 'H'; p[2] = 'N'; /* country code */
	p[3] = 0x00; /* region_id 6b + polarity 1b(+reserved) */
	p[4] = 0x00; p[5] = 0x00; /* local_time_offset */
	p[6] = 0x30; p[7] = 0x00; /* MJD */
	p[8] = 0x12; p[9] = 0x30; p[10] = 0x00; /* 12:30:00 BCD */
	p[11] = 0x00; /* next_time_offset */
	return desc(b, o, 0x58, p, 12);
}

/* ====================================================================
 * ISDB descriptors (ARIB STD-B10)
 * ==================================================================== */

/* network_identification 0xC2 */
static int d_isdb_net_id(uint8_t *b, int o)
{
	uint8_t p[7];
	p[0] = 'J'; p[1] = 'P'; p[2] = 'N'; /* country code */
	p[3] = 0x00; p[4] = 0x01; /* media type */
	p[5] = 0x00; p[6] = 0x02; /* network id */
	return desc(b, o, 0xC2, p, 7);
}

/* isdb_terrestrial_delivery_system 0xFA */
static int d_isdb_terrestrial(uint8_t *b, int o)
{
	uint8_t p[4];
	p[0] = 0x00; p[1] = 0x00; /* area_code 12b + guard_interval 2b + transmission_mode 2b */
	p[2] = 0x00; p[3] = 0x00; /* one frequency */
	return desc(b, o, 0xFA, p, 4);
}

/* hierarchical_transmission 0xC0 */
static int d_isdb_hierarchy(uint8_t *b, int o)
{
	uint8_t p[3];
	p[0] = 0x01; /* reserved 7b + hierarchical_level 1b */
	p[1] = 0xE1; /* reserved 3b + reference_PID 13b */
	p[2] = 0x00;
	return desc(b, o, 0xC0, p, 3);
}

/* digital_copy_control 0xC1 : full form with both optional fields */
static int d_isdb_dcc(uint8_t *b, int o)
{
	/* dcc_info=01(2b) max_bitrate_flag=1 comp_ctrl_flag=1 user_defined=0001(4b) */
	uint8_t p[6];
	p[0] = 0x71;
	p[1] = 0x00; /* maximum_bitrate */
	p[2] = 0x03; /* component_control_length = 3 (one entry) */
	p[3] = 0x01; /* component_tag */
	p[4] = 0x40; /* copy_control=01 max_bitrate_flag=0 reserved */
	p[5] = 0x00; /* maximum_bitrate */
	return desc(b, o, 0xC1, p, 6);
}

/* AVC_video 0x28 */
static int d_isdb_avc(uint8_t *b, int o)
{
	uint8_t p[4];
	p[0] = 0x64; /* profile High */
	p[1] = 0x00; /* constraint/compatible flags */
	p[2] = 0x28; /* level 4.0 */
	p[3] = 0x00; /* picture/24h flags + reserved */
	return desc(b, o, 0x28, p, 4);
}

/* ====================================================================
 * ATSC descriptors (A/65)
 * ==================================================================== */

static int d_atsc_caption(uint8_t *b, int o)
{
	/* 1 byte header (reserved 3b + number_of_services 5b) + 5 bytes/service */
	uint8_t p[6];
	p[0] = 0x01; /* reserved 3b + number_of_services 5b */
	p[1] = 'e'; p[2] = 'n'; p[3] = 'g'; /* language 24b */
	p[4] = 0x00; /* digital_cc 1b + reserved 1b + caption_service_number 6b */
	p[5] = 0x00; /* easy_reader + wide_aspect_ratio + reserved */
	return desc(b, o, 0x86, p, 6);
}

static int d_atsc_channel_name(uint8_t *b, int o, const char *name)
{
	return desc(b, o, 0xA0, (const uint8_t *)name, (int)strlen(name));
}

/* ====================================================================
 * dvb_all.ts : every DVB SI table + special descriptors + 2 programs
 * ==================================================================== */
static void build_dvb_all(const char *dir)
{
	static uint8_t stream[200 * TS_PKT];
	size_t pos = 0;

	/* ---- PAT (pid 0) : tsid 0x0001, prog1->PMT0x20, prog2->PMT0x22 ---- */
	{
		uint8_t body[64], o = 0;
		body[o++] = 0x00; body[o++] = 0x00; body[o++] = 0x00; body[o++] = 0x10; /* network PID */
		body[o++] = 0x00; body[o++] = 0x01; body[o++] = 0x00; body[o++] = 0x20; /* prog1 */
		body[o++] = 0x00; body[o++] = 0x02; body[o++] = 0x00; body[o++] = 0x22; /* prog2 */
		uint8_t sec[256];
		int len = sec_build(sec, 0x00, 0x0001, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0000, &cc, sec, len);
	}

	/* ---- CAT (pid 1) : CA descriptor ---- */
	{
		uint8_t body[16], o = 0;
		o = d_ca(body, o, 0x1702, 0x0E);
		uint8_t sec[256];
		int len = sec_build(sec, 0x01, 0x0001, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0001, &cc, sec, len);
	}

	/* ---- TSDT (pid 2) : registration descriptor ---- */
	{
		uint8_t body[16], o = 0;
		o = d_registration(body, o, 0x41434454);
		uint8_t sec[256];
		int len = sec_build(sec, 0x03, 0x0001, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0002, &cc, sec, len);
	}

	/* ---- NIT actual (pid 0x10) : network_name + satellite + service_list ---- */
	{
		uint8_t nbody[64], no = 0;
		no = d_network_name(nbody, no, "TSA-NET");
		no = d_satellite(nbody, no);
		uint8_t tsbody[64], to = 0;
		to = d_service_list(tsbody, to, 0x0001, 0x01);
		to = d_service_list(tsbody, to, 0x0002, 0x02);
		uint8_t body[128], o = 0;
		body[o++] = (uint8_t)(no >> 8); body[o++] = (uint8_t)(no); /* net_desc_len */
		memcpy(body + o, nbody, no); o += no;
		int tsloop = 6 + to;
		body[o++] = (uint8_t)(tsloop >> 8); body[o++] = (uint8_t)(tsloop);
		body[o++] = 0x00; body[o++] = 0x01; /* transport_stream_id */
		body[o++] = 0x00; body[o++] = 0x01; /* original_network_id */
		body[o++] = (uint8_t)(to >> 8); body[o++] = (uint8_t)(to); /* ts_desc_len */
		memcpy(body + o, tsbody, to); o += to;
		uint8_t sec[256];
		int len = sec_build(sec, 0x40, 0x0001, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0010, &cc, sec, len);
	}

	/* ---- SDT actual (pid 0x11) : two services with service descriptor ---- */
	{
		uint8_t svc1[64], s1 = 0;
		s1 = d_service(svc1, s1, 0x01, "TSA", "TSAnalyze One");
		uint8_t svc2[64], s2 = 0;
		s2 = d_service(svc2, s2, 0x02, "TSA", "TSAnalyze Two");
		uint8_t body[128], o = 0;
		body[o++] = 0x00; body[o++] = 0x01; /* original_network_id */
		body[o++] = 0xFF; /* reserved */
		/* service 1 */
		body[o++] = 0x00; body[o++] = 0x01; /* service_id */
		body[o++] = 0xFC; /* EIT flags + running_status + free_CA */
		body[o++] = (uint8_t)(0xF0 | ((s1 >> 8) & 0x0F));
		body[o++] = (uint8_t)(s1 & 0xFF);
		memcpy(body + o, svc1, s1); o += s1;
		/* service 2 */
		body[o++] = 0x00; body[o++] = 0x02;
		body[o++] = 0xFC;
		body[o++] = (uint8_t)(0xF0 | ((s2 >> 8) & 0x0F));
		body[o++] = (uint8_t)(s2 & 0xFF);
		memcpy(body + o, svc2, s2); o += s2;
		uint8_t sec[256];
		int len = sec_build(sec, 0x42, 0x0001, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0011, &cc, sec, len);
	}

	/* ---- BAT (pid 0x11) : bouquet name + one TS ---- */
	{
		uint8_t bdesc[64], bo = 0;
		bo = d_bouquet_name(bdesc, bo, "Free Bouquet");
		uint8_t tsbody[16], to = 0;
		to = d_service_list(tsbody, to, 0x0001, 0x01);
		uint8_t body[128], o = 0;
		body[o++] = (uint8_t)(bo >> 8); body[o++] = (uint8_t)(bo);
		memcpy(body + o, bdesc, bo); o += bo;
		int tsloop = 6 + to;
		body[o++] = (uint8_t)(tsloop >> 8); body[o++] = (uint8_t)(tsloop);
		body[o++] = 0x00; body[o++] = 0x01;
		body[o++] = 0x00; body[o++] = 0x01;
		body[o++] = (uint8_t)(to >> 8); body[o++] = (uint8_t)(to);
		memcpy(body + o, tsbody, to); o += to;
		uint8_t sec[256];
		int len = sec_build(sec, 0x4A, 0x0001, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0011, &cc, sec, len);
	}

	/* ---- EIT actual (pid 0x12) : one event with short_event ---- */
	{
		uint8_t edesc[64], eo = 0;
		eo = d_short_event(edesc, eo, "eng", "Monday Night", "Comprehensive DVB test");
		uint8_t body[128], o = 0;
		body[o++] = 0x00; body[o++] = 0x01; /* transport_stream_id */
		body[o++] = 0x00; body[o++] = 0x01; /* original_network_id */
		body[o++] = 0x00; /* segment_last_section_number */
		body[o++] = 0x4E; /* last_table_id */
		/* event */
		body[o++] = 0x00; body[o++] = 0x01; /* event_id */
		body[o++] = 0x30; body[o++] = 0x00; body[o++] = 0x12; body[o++] = 0x30; body[o++] = 0x00; /* start_time 5B */
		body[o++] = 0x01; body[o++] = 0x00; body[o++] = 0x00; /* duration 3B */
		body[o++] = (uint8_t)(0x40 | ((eo >> 8) & 0x0F)); /* running_status 3b + free_ca + desc_len hi */
		body[o++] = (uint8_t)(eo & 0xFF);
		memcpy(body + o, edesc, eo); o += eo;
		uint8_t sec[256];
		int len = sec_build(sec, 0x4E, 0x0001, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0012, &cc, sec, len);
	}

	/* ---- TDT (pid 0x14, no CRC) ---- */
	{
		uint8_t pkt[TS_PKT];
		memset(pkt, 0xFF, TS_PKT);
		pkt[0] = 0x47;
		pkt[1] = 0x40;           /* PUSI set */
		pkt[2] = 0x14;
		pkt[3] = 0x10;
		pkt[4] = 0x70; /* table_id */
		pkt[5] = 0x00; pkt[6] = 0x05; /* section_length */
		pkt[7] = 0x30; pkt[8] = 0x00; pkt[9] = 0x12; pkt[10] = 0x30; pkt[11] = 0x00; /* UTC */
		memcpy(stream + pos, pkt, TS_PKT); pos += TS_PKT;
	}

	/* ---- TOT (pid 0x14) : UTC + local_time_offset descriptor ---- */
	{
		uint8_t dbody[16], do_ = 0;
		do_ = d_local_time_offset(dbody, do_);
		uint8_t body[32], o = 0;
		memcpy(body + o, (uint8_t[]){0x30, 0x00, 0x12, 0x30, 0x00}, 5); o += 5; /* UTC */
		body[o++] = (uint8_t)(do_ >> 8); body[o++] = (uint8_t)(do_);
		memcpy(body + o, dbody, do_); o += do_;
		uint8_t sec[64];
		int len = sec_build(sec, 0x73, 0x0001, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0014, &cc, sec, len);
	}

	/* ---- PMT 0x20 (program 1) : subtitling+teletext+CA at program level,
	 *      video + audio + subtitle ES ---- */
	{
		uint8_t pdesc[64], po = 0;
		po = d_subtitling(pdesc, po, "eng", 0x10, 0x100, 0x200);
		po = d_teletext(pdesc, po, "eng", 0x02, 0x01);
		po = d_ca(pdesc, po, 0x1702, 0x0E);
		/* video ES */
		uint8_t vdesc[16], vo = 0;
		/* audio ES with AC3 */
		uint8_t adesc[64], ao = 0;
		uint8_t ac3[1] = { 0x00 };
		ao = desc(adesc, ao, 0x6A, ac3, 1);
		/* subtitle ES with subtitling */
		uint8_t sdesc[16], so = 0;
		so = d_subtitling(sdesc, so, "eng", 0x10, 0x300, 0x400);

		uint8_t body[256], o = 0;
		body[o++] = 0xE0; body[o++] = 0x00; /* PCR_PID = 0xE0 */
		body[o++] = (uint8_t)(po >> 8); body[o++] = (uint8_t)(po);
		memcpy(body + o, pdesc, po); o += po;
		/* video ES 0x02 pid 0xE0 */
		body[o++] = 0x02; body[o++] = 0xE0; body[o++] = 0xE0;
		body[o++] = (uint8_t)(vo >> 8); body[o++] = (uint8_t)(vo);
		memcpy(body + o, vdesc, vo); o += vo;
		/* audio ES 0x03 pid 0xE1 */
		body[o++] = 0x03; body[o++] = 0xE0; body[o++] = 0xE1;
		body[o++] = (uint8_t)(ao >> 8); body[o++] = (uint8_t)(ao);
		memcpy(body + o, adesc, ao); o += ao;
		/* subtitle ES 0x06 pid 0xE2 */
		body[o++] = 0x06; body[o++] = 0xE0; body[o++] = 0xE2;
		body[o++] = (uint8_t)(so >> 8); body[o++] = (uint8_t)(so);
		memcpy(body + o, sdesc, so); o += so;

		uint8_t sec[512];
		int len = sec_build(sec, 0x02, 0x0001, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0020, &cc, sec, len);
	}

	/* ---- PMT 0x22 (program 2) : simpler ---- */
	{
		uint8_t body[64], o = 0;
		body[o++] = 0xE0; body[o++] = 0x10; /* PCR_PID = 0xE0 */
		body[o++] = 0x00; body[o++] = 0x00; /* program_info_length 0 */
		/* video 0x02 pid 0xE0 with AVC registration */
		uint8_t vdesc[16], vo = 0;
		vo = d_registration(vdesc, vo, 0x4856);
		body[o++] = 0x02; body[o++] = 0xE0; body[o++] = 0xE0;
		body[o++] = (uint8_t)(vo >> 8); body[o++] = (uint8_t)(vo);
		memcpy(body + o, vdesc, vo); o += vo;
		uint8_t sec[128];
		int len = sec_build(sec, 0x02, 0x0002, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0022, &cc, sec, len);
	}

	pad_null(stream, &pos, 2040);
	write_file(dir, "dvb_all.ts", stream, pos);
}

/* ====================================================================
 * dvb_multi_pmt.ts : 3 programs, PMT of program 2 spans multiple packets
 * ==================================================================== */
static void build_dvb_multi_pmt(const char *dir)
{
	static uint8_t stream[300 * TS_PKT];
	size_t pos = 0;

	/* PAT : prog1->0x20, prog2->0x30, prog3->0x40 */
	{
		uint8_t body[64], o = 0;
		body[o++] = 0x00; body[o++] = 0x00; body[o++] = 0x00; body[o++] = 0x10;
		body[o++] = 0x00; body[o++] = 0x01; body[o++] = 0x00; body[o++] = 0x20;
		body[o++] = 0x00; body[o++] = 0x02; body[o++] = 0x00; body[o++] = 0x30;
		body[o++] = 0x00; body[o++] = 0x03; body[o++] = 0x00; body[o++] = 0x40;
		uint8_t sec[256];
		int len = sec_build(sec, 0x00, 0x0001, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0000, &cc, sec, len);
	}

	/* PMT 0x20 (prog1) : single ES */
	{
		uint8_t body[32], o = 0;
		body[o++] = 0xE0; body[o++] = 0x00;
		body[o++] = 0x00; body[o++] = 0x00;
		body[o++] = 0x02; body[o++] = 0xE0; body[o++] = 0xE0;
		body[o++] = 0x00; body[o++] = 0x00;
		uint8_t sec[128];
		int len = sec_build(sec, 0x02, 0x0001, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0020, &cc, sec, len);
	}

	/* PMT 0x30 (prog2) : many ES => section > 183 B, spans multiple packets */
	{
		uint8_t body[2048];
		int o = 0;
		body[o++] = 0xE0; body[o++] = 0x00;
		body[o++] = 0x00; body[o++] = 0x00;
		/* 80 ES entries of 5 bytes each = 400 bytes => section ~416 B */
		for (int i = 0; i < 80; i++) {
			body[o++] = 0x02;                    /* video stream */
			body[o++] = 0xE0; body[o++] = (uint8_t)(0x10 + (i & 0x0F)); /* pid 0xE010..0xE01F */
			body[o++] = 0x00; body[o++] = 0x00;  /* no es descriptors */
		}
		/* verify it exceeds one packet payload */
		uint8_t sec[2400];
		int len = sec_build(sec, 0x02, 0x0002, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0030, &cc, sec, len);
	}

	/* PMT 0x40 (prog3) : minimal */
	{
		uint8_t body[16], o = 0;
		body[o++] = 0xE0; body[o++] = 0x00;
		body[o++] = 0x00; body[o++] = 0x00;
		body[o++] = 0x03; body[o++] = 0xE0; body[o++] = 0xF0;
		body[o++] = 0x00; body[o++] = 0x00;
		uint8_t sec[64];
		int len = sec_build(sec, 0x02, 0x0003, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0040, &cc, sec, len);
	}

	pad_null(stream, &pos, 2040);
	write_file(dir, "dvb_multi_pmt.ts", stream, pos);
}

/* ====================================================================
 * isdb_stream.ts : DVB tables carrying ISDB descriptors
 * ==================================================================== */
static void build_isdb(const char *dir)
{
	static uint8_t stream[200 * TS_PKT];
	size_t pos = 0;

	/* PAT */
	{
		uint8_t body[64], o = 0;
		body[o++] = 0x00; body[o++] = 0x00; body[o++] = 0x00; body[o++] = 0x10;
		body[o++] = 0x00; body[o++] = 0x01; body[o++] = 0x00; body[o++] = 0x20;
		uint8_t sec[256];
		int len = sec_build(sec, 0x00, 0x0001, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0000, &cc, sec, len);
	}

	/* NIT with ISDB descriptors */
	{
		uint8_t nbody[64], no = 0;
		no = d_isdb_net_id(nbody, no);
		no = d_isdb_terrestrial(nbody, no);
		uint8_t tsbody[64], to = 0;
		to = d_isdb_hierarchy(tsbody, to);
		to = d_isdb_dcc(tsbody, to);
		uint8_t body[128], o = 0;
		body[o++] = (uint8_t)(no >> 8); body[o++] = (uint8_t)(no);
		memcpy(body + o, nbody, no); o += no;
		int tsloop = 6 + to;
		body[o++] = (uint8_t)(tsloop >> 8); body[o++] = (uint8_t)(tsloop);
		body[o++] = 0x00; body[o++] = 0x01;
		body[o++] = 0x00; body[o++] = 0x01;
		body[o++] = (uint8_t)(to >> 8); body[o++] = (uint8_t)(to);
		memcpy(body + o, tsbody, to); o += to;
		uint8_t sec[256];
		int len = sec_build(sec, 0x40, 0x0001, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0010, &cc, sec, len);
	}

	/* SDT with ISDB service descriptors */
	{
		/* one (physical) service, two descriptors in a single descriptor loop */
		uint8_t svcdesc[128], so = 0;
		so = d_service(svcdesc, so, 0xA0, "ARIB", "ISDB Test Service");
		so = d_isdb_dcc(svcdesc, so); /* copy control on service */
		uint8_t body[128], o = 0;
		body[o++] = 0x00; body[o++] = 0x01;
		body[o++] = 0xFF;
		body[o++] = 0x00; body[o++] = 0x01; /* service_id */
		body[o++] = 0xFC;
		body[o++] = (uint8_t)(0xF0 | ((so >> 8) & 0x0F));
		body[o++] = (uint8_t)(so & 0xFF);
		memcpy(body + o, svcdesc, so); o += so;
		uint8_t sec[256];
		int len = sec_build(sec, 0x42, 0x0001, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0011, &cc, sec, len);
	}

	/* PMT with ISDB descriptors (AVC video, audio component) */
	{
		uint8_t vdesc[16], vo = 0;
		vo = d_isdb_avc(vdesc, vo);
		uint8_t body[64], o = 0;
		body[o++] = 0xE0; body[o++] = 0x00;
		body[o++] = 0x00; body[o++] = 0x00;
		body[o++] = 0x1B; body[o++] = 0xE0; body[o++] = 0xE0; /* H.264 video */
		body[o++] = (uint8_t)(vo >> 8); body[o++] = (uint8_t)(vo);
		memcpy(body + o, vdesc, vo); o += vo;
		body[o++] = 0x04; body[o++] = 0xE0; body[o++] = 0xE1; /* audio, no desc */
		body[o++] = 0x00; body[o++] = 0x00;
		uint8_t sec[128];
		int len = sec_build(sec, 0x02, 0x0001, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0020, &cc, sec, len);
	}

	pad_null(stream, &pos, 2040);
	write_file(dir, "isdb_stream.ts", stream, pos);
}

/* ====================================================================
 * atsc_psip.ts : ATSC PSIP tables on PID 0x1FFB
 * ==================================================================== */
static void build_atsc(const char *dir)
{
	static uint8_t stream[300 * TS_PKT];
	size_t pos = 0;

	/* PAT */
	{
		uint8_t body[64], o = 0;
		body[o++] = 0x00; body[o++] = 0x00; body[o++] = 0x00; body[o++] = 0x10;
		body[o++] = 0x00; body[o++] = 0x01; body[o++] = 0x00; body[o++] = 0x20;
		uint8_t sec[128];
		int len = sec_build(sec, 0x00, 0x0001, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0000, &cc, sec, len);
	}
	/* PMT 0x20 */
	{
		uint8_t body[32], o = 0;
		body[o++] = 0xE0; body[o++] = 0x00;
		body[o++] = 0x00; body[o++] = 0x00;
		body[o++] = 0x02; body[o++] = 0xE0; body[o++] = 0xE0;
		body[o++] = 0x00; body[o++] = 0x00;
		uint8_t sec[64];
		int len = sec_build(sec, 0x02, 0x0001, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0020, &cc, sec, len);
	}

	/* ---- all PSIP tables share PID 0x1FFB ---- */
	int cc = 0;

	/* MGT (0xC7) */
	{
		uint8_t tbody[64], to = 0;
		to = d_atsc_caption(tbody, to);
		uint8_t body[128], o = 0;
		body[o++] = 0x00; /* protocol_version */
		body[o++] = 0x00; body[o++] = 0x01; /* tables_defined = 1 */
		/* table entry : TVCT 0x0100 on PID 0x1FFB */
		body[o++] = 0x01; body[o++] = 0x00; /* table_type */
		body[o++] = (uint8_t)(0xE0 | ((0x1FFB >> 8) & 0x1F)); body[o++] = (uint8_t)(0x1FFB & 0xFF);
		body[o++] = (uint8_t)(0x03 << 3); /* version 0 */
		body[o++] = 0x00; body[o++] = 0x00; body[o++] = 0x00; body[o++] = 0x00; /* number_bytes */
		body[o++] = 0x00; body[o++] = 0x00; /* table_type_descriptors_length */
		body[o++] = (uint8_t)(to >> 8); body[o++] = (uint8_t)(to); /* descriptors_length */
		memcpy(body + o, tbody, to); o += to;
		uint8_t sec[256];
		int len = sec_build(sec, 0xC7, 0x0001, 0, 1, 0, 0, body, o);
		emit_section(stream, &pos, 0x1FFB, &cc, sec, len);
	}

	/* TVCT (0xC8) */
	{
		uint8_t chdesc[32], co = 0;
		co = d_atsc_channel_name(chdesc, co, "EDGE");
		uint8_t body[512], o = 0;
		body[o++] = 0x00; /* protocol_version */
		body[o++] = 0x01; /* num_channels = 1 */
		/* channel */
		uint8_t shortname[14];
		memset(shortname, 0, 14); /* "1234" as UTF-16 exact */
		shortname[0] = 0x00; shortname[1] = '1';
		shortname[2] = 0x00; shortname[3] = '2';
		shortname[4] = 0x00; shortname[5] = '3';
		shortname[6] = 0x00; shortname[7] = '4';
		memcpy(body + o, shortname, 14); o += 14;
		body[o++] = (uint8_t)(0x00); /* major 10b + minor 10b + mod 8b (4 bytes) */
		body[o++] = (uint8_t)(0x01 << 6); /* major=1 -> bytes */
		body[o++] = 0x00; body[o++] = 0x00;
		body[o++] = 0x00; body[o++] = 0x00; body[o++] = 0x00; body[o++] = 0x00; /* carrier_frequency */
		body[o++] = 0x00; body[o++] = 0x01; /* channel_TSID */
		body[o++] = 0x00; body[o++] = 0x01; /* program_number */
		body[o++] = 0x00; body[o++] = 0x00; /* ETM(2)+access(1)+hidden(1)+hide_guide(1)+service_type(6) */
		body[o++] = 0x00; body[o++] = 0x01; /* source_id */
		body[o++] = (uint8_t)(co >> 8); body[o++] = (uint8_t)(co); /* descriptors_length */
		memcpy(body + o, chdesc, co); o += co;
		body[o++] = 0x00; body[o++] = 0x00; /* additional_descriptors_length */
		uint8_t sec[512];
		int len = sec_build(sec, 0xC8, 0x0001, 0, 1, 0, 0, body, o);
		emit_section(stream, &pos, 0x1FFB, &cc, sec, len);
	}

	/* CVCT (0xC9) */
	{
		uint8_t body[128], o = 0;
		body[o++] = 0x00; body[o++] = 0x00; /* protocol_version, num_channels 0 */
		body[o++] = 0x00; body[o++] = 0x00; /* additional_descriptors_length */
		uint8_t sec[128];
		int len = sec_build(sec, 0xC9, 0x0001, 0, 1, 0, 0, body, o);
		emit_section(stream, &pos, 0x1FFB, &cc, sec, len);
	}

	/* STT (0xCD) */
	{
		uint8_t body[32], o = 0;
		body[o++] = 0x00; /* protocol_version */
		body[o++] = 0x00; body[o++] = 0x00; body[o++] = 0x00; body[o++] = 0x00; /* system_time */
		body[o++] = 0x00; /* GPS_UTC_offset */
		body[o++] = 0x00; body[o++] = 0x00; /* daylight_saving */
		uint8_t sec[64];
		int len = sec_build(sec, 0xCD, 0x0001, 0, 1, 0, 0, body, o);
		emit_section(stream, &pos, 0x1FFB, &cc, sec, len);
	}

	/* RRT (0xCA) */
	{
		uint8_t body[32], o = 0;
		body[o++] = 0x00; /* protocol_version */
		body[o++] = 0x01; /* rating_region_name_length (1) */
		body[o++] = 0x00; /* number_strings = 0 */
		body[o++] = 0x00; /* dimensions_defined = 0 */
		body[o++] = 0x00; body[o++] = 0x00; /* descriptors_length = 0 */
		uint8_t sec[64];
		int len = sec_build(sec, 0xCA, 0x0001, 0, 1, 0, 0, body, o);
		emit_section(stream, &pos, 0x1FFB, &cc, sec, len);
	}

	/* ATSC EIT (0xCB) */
	{
		uint8_t body[32], o = 0;
		body[o++] = 0x00; /* protocol_version */
		body[o++] = 0x00; /* num_events */
		uint8_t sec[64];
		int len = sec_build(sec, 0xCB, 0x0001, 0, 1, 0, 0, body, o);
		emit_section(stream, &pos, 0x1FFB, &cc, sec, len);
	}

	/* ETT (0xCC) */
	{
		uint8_t body[32], o = 0;
		body[o++] = 0x00; /* protocol_version */
		body[o++] = 0x00; body[o++] = 0x00; body[o++] = 0x00; body[o++] = 0x00; /* ETM_id */
		body[o++] = 0x00; /* empty multiple string */
		uint8_t sec[64];
		int len = sec_build(sec, 0xCC, 0x0001, 0, 1, 0, 0, body, o);
		emit_section(stream, &pos, 0x1FFB, &cc, sec, len);
	}

	/* DCCT (0xD3) */
	{
		uint8_t body[32], o = 0;
		body[o++] = 0x00; body[o++] = 0x00; /* protocol_version, dcc_test_count 0 */
		body[o++] = 0x00; body[o++] = 0x00; /* additional descriptors length */
		uint8_t sec[64];
		int len = sec_build(sec, 0xD3, 0x0001, 0, 1, 0, 0, body, o);
		emit_section(stream, &pos, 0x1FFB, &cc, sec, len);
	}

	/* DCCSCT (0xD4) */
	{
		uint8_t body[32], o = 0;
		body[o++] = 0x00; body[o++] = 0x00; /* protocol_version, updates_defined 0 */
		body[o++] = 0x00; body[o++] = 0x00; /* additional descriptors length */
		uint8_t sec[64];
		int len = sec_build(sec, 0xD4, 0x0001, 0, 1, 0, 0, body, o);
		emit_section(stream, &pos, 0x1FFB, &cc, sec, len);
	}

	pad_null(stream, &pos, 2040);
	write_file(dir, "atsc_psip.ts", stream, pos);
}

/* ====================================================================
 * subtitle.ts : DVB subtitling (EN 300 743) carried as a PES stream.
 *
 * PAT -> PMT(0x20) -> a subtitle ES on PID 0xE2 whose PMT ES descriptor
 * loop carries a subtitling descriptor (0x59).  The ES itself is a PES
 * stream of subtitle segments.  The parser registers parse_subtitle on
 * that PID and, with --detail (-d), dumps the parsed segments.
 * ==================================================================== */
static void build_subtitle(const char *dir)
{
	static uint8_t stream[100 * TS_PKT];
	size_t pos = 0;

	/* PAT : program 1 -> PMT 0x20 */
	{
		uint8_t body[32], o = 0;
		body[o++] = 0x00; body[o++] = 0x00; body[o++] = 0x00; body[o++] = 0x10;
		body[o++] = 0x00; body[o++] = 0x01; body[o++] = 0x00; body[o++] = 0x20;
		uint8_t sec[128];
		int len = sec_build(sec, 0x00, 0x0001, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0000, &cc, sec, len);
	}

	/* PMT 0x20 : PCR_PID 0xE0, one subtitle ES 0x06 -> PID 0xE2 with a
	 * subtitling descriptor (0x59) so parse_subtitle gets registered. */
	{
		uint8_t esd[16], eo = 0;
		eo = d_subtitling(esd, eo, "eng", 0x10, 0x100, 0x200); /* 8 bytes */
		uint8_t body[64], o = 0;
		body[o++] = 0xE0; body[o++] = 0x00; /* PCR_PID */
		body[o++] = 0x00; body[o++] = 0x00; /* program_info_length 0 */
		body[o++] = 0x06; body[o++] = 0xE0; body[o++] = 0xE2; /* private: PID 0xE2 */
		body[o++] = (uint8_t)(eo >> 8); body[o++] = (uint8_t)(eo);
		memcpy(body + o, esd, eo); o += eo;
		uint8_t sec[128];
		int len = sec_build(sec, 0x02, 0x0001, 0, 1, 0, 0, body, o);
		int cc = 0;
		emit_section(stream, &pos, 0x0020, &cc, sec, len);
	}

	/* ---- subtitle PES on PID 0xE2 ----
	 * A minimal but valid DVB subtitle page:
	 *   PES payload:
	 *     data_identifier      0x20
	 *     subtitle_stream_id   0x00
	 *     page_composition_segment (sync 0x0F, type 0x10, page_id 0x0001):
	 *       page_time_out 0xFF, page_version=0/page_state=0x2 (0x20),
	 *       segment_length 0x0002, no regions.
	 *
	 * PES header (stream_id 0xBD private_stream_1):
	 *   00 00 01 BD  PES_packet_length(hi lo)  80 00 00
	 * PES_packet_length = 3 (header bytes) + payload bytes.  Payload is 10
	 * bytes (2 subtitle header + 8-byte segment), so length = 13 (0x000D).
	 */
	{
		/* PES_packet_data_byte view handed to parse_subtitle: */
		uint8_t subpayload[10] = {
			0x20,                         /* data_identifier */
			0x00,                         /* subtitle_stream_id */
			0x0F, 0x10, 0x00, 0x01, 0x00, 0x02, /* page_composition_segment */
			0xFF, 0x20,                  /* timeout, version+page_state */
		};
		uint8_t pes[9 + 10]; /* 9-byte PES header + 10-byte payload */
		pes[0] = 0x00; pes[1] = 0x00; pes[2] = 0x01; pes[3] = 0xBD; /* private_stream_1 */
		pes[4] = 0x00; pes[5] = 0x0D; /* PES_packet_length = 13 */
		pes[6] = 0x80;                 /* marker, scrambling 0, ... */
		pes[7] = 0x00;                 /* no PTS/DTS, no flags */
		pes[8] = 0x00;                 /* PES_header_data_length 0 */
		memcpy(pes + 9, subpayload, 10);

		uint8_t pkt[TS_PKT];
		memset(pkt, 0xFF, TS_PKT);
		pkt[0] = 0x47;
		pkt[1] = (uint8_t)(0x40 | ((0xE2 >> 8) & 0x1F)); /* PUSI + PID hi */
		pkt[2] = 0xE2;                 /* PID lo */
		pkt[3] = 0x10;                 /* payload only, cc 0 */
		memcpy(pkt + 4, pes, sizeof(pes));
		memcpy(stream + pos, pkt, TS_PKT);
		pos += TS_PKT;
	}

	pad_null(stream, &pos, 2040);
	write_file(dir, "subtitle.ts", stream, pos);
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s <outdir>\n", argv[0]);
		return 1;
	}
	const char *dir = argv[1];
	crc_init();
	mkdir(dir, 0755);

	/* ultra-short / malformed streams */
	build_robustness(dir);

	/* comprehensive DVB / ISDB / ATSC PSIP streams */
	build_dvb_all(dir);
	build_dvb_multi_pmt(dir);
	build_isdb(dir);
	build_atsc(dir);
	build_subtitle(dir);

	return 0;
}