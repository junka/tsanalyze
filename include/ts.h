#ifndef _BITS_H_
#define _BITS_H_

#include <stdint.h>

#include "types.h"
/* define ts structure ,see ISO/IEC13818-1 */

#ifdef __cplusplus
extern "C" {
#endif

#define TS_PACKET_SIZE 188
#define TS_DVHS_PACKET_SIZE 192
#define TS_FEC_PACKET_SIZE 204
#define TS_MAX_PACKET_SIZE 204
#define TS_MAX_PID 8191

#define TS_SYNC_BYTE (0x47)

typedef struct
{
	uint8_t sync_byte; /*0x47*/
	uint16_t transport_error_indicator : 1;
	uint16_t payload_unit_start_indicator : 1;
	uint16_t transport_priority : 1;
	uint16_t PID : 13;
	uint8_t transport_scrambling_control : 2;
	uint8_t adaptation_field_control : 2;
	uint8_t continuity_counter : 4;
} __attribute__((packed)) ts_header;

enum adaptation_field_e {
	ADAPT_RESERVED = 0,
	ADAPT_NO_FIELD = 1,
	ADAPT_ONLY = 2,
	ADAPT_BOTH = 3,
};

typedef struct
{
	uint64_t program_clock_reference_base : 33;
	uint64_t reserved : 6;
	uint64_t program_clock_reference_extension : 9;
} __attribute__((packed)) pcr_clock;

typedef struct
{
	uint8_t adaptation_field_length;
	uint8_t discontinuity_indicator : 1;
	uint8_t random_access_indicator : 1;
	uint8_t elementary_stream_priority_indicator : 1;
	uint8_t PCR_flag : 1;
	uint8_t OPCR_flag : 1;
	uint8_t splicing_point_flag : 1;
	uint8_t transport_private_data_flag : 1;
	uint8_t adaptation_field_extension_flag : 1;
} __attribute__((packed)) ts_adaptation_field;

enum PID_e {
	PAT_PID = 0x0000,
	CAT_PID = 0x0001,
	TSDT_PID = 0x0002,
	IPMP_PID = 0x0003,

	/* 4 - 15 reserved for future use */
	NIT_PID = 0x0010,
	SDT_PID = 0x0011,
	BAT_PID = 0x0011,
	EIT_PID = 0x0012,
	RST_PID = 0x0013,
	TDT_PID = 0x0014,
	TOT_PID = 0x0014,
	RNT_PID = 0x0016,
	DIT_PID = 0x001E,
	SIT_PID = 0x001F,


	/* PSIP tables */
	MGT_PID = 0x1FFB,
	TVCT_PID = 0x1FFB,
	CVCT_PID = 0x1FFB,
	RRT_PID = 0x1FFB,
	STT_PID = 0x1FFB,
	DCCT_PID = 0x1FFB,
	DCCSCT_PID = 0x1FFB,

	/* 32-8186 for PMT */
	NULL_PID = 0x1FFF,
};

#define SYS_CLK (27000000)

#define TS_READ8(buff) (*(buff))
#define TS_READ16(buff) (uint16_t)((((uint16_t) * (buff)) << 8) | ((uint16_t) * (buff + 1)))
#define TS_READ32(buff)                                                                                                \
	(uint32_t)(((uint32_t) * (buff) << 24) | ((uint32_t) * (buff + 1) << 16) | ((uint32_t) * (buff + 2) << 8) |        \
			   ((uint32_t) * (buff + 3)))
#define TS_READ64(buff)                                                                                                \
	(uint64_t)(((uint64_t) * (buff) << 56) | ((uint64_t) * (buff + 1) << 48) | ((uint64_t) * (buff + 2) << 40) |       \
			   ((uint64_t) * (buff + 3) << 32) | ((uint64_t) * (buff + 4) << 24) | ((uint64_t) * (buff + 5) << 16) |   \
			   ((uint64_t) * (buff + 6) << 8) | ((uint64_t) * (buff + 7)))

static inline uint24_t ts_read_uint24(uint8_t *buff)
{
	assert(sizeof(uint24_t) == 3);
	uint24_t ret_v;
	ret_v.bits[0] = *buff;
	ret_v.bits[1] = *(buff+1);
	ret_v.bits[2] = *(buff + 2);
	return ret_v;
}

#define TS_READ24(buff) ts_read_uint24(buff)

#define TS_READ_uint8_t(buf) TS_READ8(buf)
#define TS_READ_uint16_t(buf) TS_READ16(buf)
#define TS_READ_uint32_t(buf) TS_READ32(buf)
#define TS_READ_uint64_t(buf) TS_READ64(buf)
#define TS_READ_uint24_t(buf) TS_READ24(buf)

/*define helper for reading bits set*/
#define TS_READ_BIT(buf, mark) ((buf[0] >> mark) & 0x01)
#define TS_READ8_BITS(buf, bitlen, off) ((TS_READ8(buf) >> (8 - off - bitlen)) & ((1 << bitlen) - 1))
#define TS_READ16_BITS(buf, bitlen, off) ((TS_READ16(buf) >> (16 - off - bitlen)) & ((1 << bitlen) - 1))
#define TS_READ32_BITS(buf, bitlen, off) ((TS_READ32(buf) >> (32 - off - bitlen)) & ((1 << bitlen) - 1))
#define TS_READ64_BITS(buf, bitlen, off) ((TS_READ64(buf) >> (64 - off - bitlen)) & (((uint64_t)1 << bitlen) - 1))
#define TS_READ24_BITS(buf, bitlen, off) ((TS_READ24(buf) >> (24 - off - bitlen)) & ((1 << bitlen) - 1))

#define TS_READ_BITS_uint8_t(buf, bitlen, off) TS_READ8_BITS(buf, bitlen, off)
#define TS_READ_BITS_uint16_t(buf, bitlen, off) TS_READ16_BITS(buf, bitlen, off)
#define TS_READ_BITS_uint32_t(buf, bitlen, off) TS_READ32_BITS(buf, bitlen, off)
#define TS_READ_BITS_uint64_t(buf, bitlen, off) TS_READ64_BITS(buf, bitlen, off)
#define TS_READ_BITS_uint24_t(buf, bitlen, off) TS_READ64_BITS(buf, bitlen, off)

#define PAT_SHOW 	(1 << 0)
#define CAT_SHOW 	(1 << 1)
#define PMT_SHOW 	(1 << 2)
#define TSDT_SHOW 	(1 << 3)
#define NIT_SHOW 	(1 << 4)
#define SDT_SHOW 	(1 << 5)
#define BAT_SHOW 	(1 << 6)
#define TDT_SHOW 	(1 << 7)
#define EIT_SHOW	(1 << 8)

/* payload length step on */
#define PL_STEP(p, l, v) \
	l -= v; \
	p = (uint8_t *)p + v

struct tsa_config
{
	char name[256]; // filename
	uint8_t pids[TS_MAX_PID + 1];
	uint8_t type;
	uint8_t brief : 1;
	uint8_t detail : 1;
	uint8_t stats : 1;
	uint8_t mem;
	uint16_t tables;
	uint8_t output;
};

struct tsa_config *get_config(void);

#define MAX_TS_PID_NUM 8192

int init_pid_processor(void);

void uninit_pid_processor(void);

int ts_process(void);

void dump_ts_info(void);

#ifdef __cplusplus
}
#endif

#endif
