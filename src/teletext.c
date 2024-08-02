#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "pes.h"
#include "table.h"
#include "teletext.h"
#include "ts.h"
#include "types.h"
#include "utils.h"

// 8 magazine 100 pages
static uint8_t g_text[8][100][32][40];

void composite_teletext_pages(int X, int P, int Y, int z, uint8_t v)
{
    g_text[X][P][Y][z] = v;
}

static void dump_teletext_page_oneline(int X, int P, int Y)
{
	char line[8];
	char buf[41] = "                                        ";
	snprintf(line, 8, "X/%d", Y);

	for (int i = 0; i < 40; i++) {
		char ch = g_text[X][P][Y][i] & 0x7F;
		buf[i] = ch;
	}
	rout(1, line, "%s", buf);
}

void dump_teletext_pages(int X, int P)
{
	char page[32];
	snprintf(page, 32, "Page %d", P);
	rout(0, page, NULL);
	for (int i = 1; i <= 25; i++) {
		dump_teletext_page_oneline(X, P, i);
	}
}

#ifdef _MSC_VER
unsigned int popcount(uint32_t u)
{
	u = (u & 0x55555555) + ((u >> 1) & 0x55555555);
	u = (u & 0x33333333) + ((u >> 2) & 0x33333333);
	u = (u & 0x0F0F0F0F) + ((u >> 4) & 0x0F0F0F0F);
	u = (u & 0x00FF00FF) + ((u >> 8) & 0x00FF00FF);
	u = (u & 0x0000FFFF) + ((u >> 16) & 0x0000FFFF);
	return u;
}
#else
#define popcount(x) __builtin_popcount(x)
#endif // _MSC_VER

static inline bool odd_parity8_test(uint8_t value) { return (popcount(value) % 2) == 1; }

static inline bool odd_parity24_test(uint8_t v1, uint8_t v2, uint8_t v3)
{
	return ((popcount(v1) + popcount(v2) + popcount(v3)) % 2) == 1;
}

#define REVERSE_BIT(x, v) ((~(1 << v)) & x)

static unsigned char bits4_lookup[16] = {
	0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe, 0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf
};

uint8_t reverse_byte(uint8_t n)
{
	// Reverse the top and bottom nibble then swap them.
	return (bits4_lookup[n & 0xf] << 4) | bits4_lookup[n >> 4];
}

#define REVERSE_BYTE(n) reverse_byte(n)

// #define HAMMING8_DATA(x) ((((x & 0x55) >> 3) & 0x8) | (((x & 0x55) >> 2) & 0x4) | (((x & 0x55) >> 1) & 0x2) | (((x &
// 0x55)) & 0x1))

/* rev order */
#define HAMMING8_DATA(x)                                                                                               \
	((((x & 0x55) >> 6) & 0x1) | (((x & 0x55) >> 3) & 0x2) | (((x & 0x55)) & 0x4) | (((x & 0x55) << 3) & 0x8))

// #define HAMMING24_DATA(x) ((x[2]&0xFE) >>1 | (x[1]&0xFE) << 6 | ((x[0] & 0x2E) << 13) & 0x7 |  ((x[0] & 0x2E) << 12)
// & 0x8)

#define HAMMING24_DATA(x)                                                                                              \
	((REVERSE_BYTE(x[2]) & 0x7F) << 11 | ((REVERSE_BYTE(x[1]) & 0x7F) << 4) |                                          \
	 ((REVERSE_BYTE(x[0]) & 0x74) >> 3) & 0xE | ((REVERSE_BYTE(x[0]) & 0x74) >> 2) & 0x1)

int decode_hamming8_code(uint8_t code)
{
	bool A = odd_parity8_test(code & 0xC5);
	bool B = odd_parity8_test(code & 0x71);
	bool C = odd_parity8_test(code & 0x5C);
	bool D = odd_parity8_test(code & 0xFF);
	if (A && B && C) {
		if (D) {
			/* all correct */
			return code;
		} else {
			/* error in P4 */
			return REVERSE_BIT(code, 1);
		}
	} else {
		if (D) {
			/*double error , reject */
			return -1;
		} else {
			if (A && B && !C) {
				/* P3 */
				return REVERSE_BIT(code, 3);
			} else if (A && C && !B) {
				/* P2 */
				return REVERSE_BIT(code, 5);
			} else if (!A && B && C) {
				/* P1 */
				return REVERSE_BIT(code, 7);
			} else if (!A && !B && C) {
				/* D4 */
				return REVERSE_BIT(code, 0);
			} else if (!A && !C && B) {
				/* D3 */
				return REVERSE_BIT(code, 2);
			} else if (A && !B && !C) {
				/* D2 */
				return REVERSE_BIT(code, 4);
			} else {
				/* D1 */
				return REVERSE_BIT(code, 6);
			}
		}
	}
	return 0;
}

int decode_hamming24_code(uint8_t data[3])
{
	bool A = odd_parity24_test(data[0] & 0xAA, data[1] & 0xAA, data[2] & 0xAA);
	bool B = odd_parity24_test(data[0] & 0x66, data[1] & 0x66, data[2] & 0x66);
	bool C = odd_parity24_test(data[0] & 0x1E, data[1] & 0x1E, data[2] & 0x1E);
	bool D = odd_parity24_test(data[0] & 1, data[1] & 0xFE, 0);
	bool E = odd_parity24_test(0, data[1] & 0x1, data[2] & 0xFE);
	bool F = odd_parity24_test(data[0], data[1], data[2]);
	if (A && B && C && D && E) {
		if (F) {
			return 0;
		} else {
			/* P6 */
			*(data + 2) = REVERSE_BIT(data[2], 0);
			return 0;
		}
	} else {
		if (F) {
			return -1;
		} else {
			int pos = 0;
			if (!E) {
				pos += 1 << 4;
			}
			if (!D) {
				pos += 1 << 3;
			}
			if (!C) {
				pos += 1 << 2;
			}
			if (!B) {
				pos += 1 << 1;
			}
			if (!A) {
				pos += 1;
			}
			*(data + (pos - 1) / 8) = REVERSE_BIT(data[(pos - 1) / 8], pos % 8);
			return 0;
		}
	}
}

/* see ETS 300706 7.1.2 */
int decode_hamming_address(uint16_t addr, int *X)
{
	/* Odd numbered data bits carry the Hamming 8/4 protection bits. */
	uint8_t magazine = (addr >> 8) & 0xFF;
	uint8_t packet = addr & 0xFF;
	int ret = decode_hamming8_code(magazine);
	if (ret < 0) {
		return -1;
	}
	*X = (HAMMING8_DATA(ret)) & 0x7;
	int Y = (HAMMING8_DATA(ret) >> 3 & 0x1) | (HAMMING8_DATA(decode_hamming8_code(packet)) << 1);

	// printf("0x%x -> X %d, 0x%x -> Y %d\n", magazine, *X, packet, Y);
	return Y;
}

int decode_odd_parity(uint8_t data)
{
	if (!odd_parity8_test(data)) {
		return -1;
	}
	return (REVERSE_BYTE(data) & 0x7F);
}

int parse_teletext_data(uint16_t addr, uint8_t data[40])
{
	int X;
	int Y = decode_hamming_address(addr, &X);
	static int P = 0;
	int ret;
	if (Y == 0) {
		/* page header */
		/* three main elements: page address, control bits and data
		 * normally intended for display as described in subclause 9.3.1. */
		ret = decode_hamming8_code(data[0]);
		uint8_t units = HAMMING8_DATA(ret);
		uint8_t tens = HAMMING8_DATA(decode_hamming8_code(data[1]));
		P = tens * 10 + units;
		// printf("X %d page %d (%d, %d)\n", X, P, tens, units);
		uint8_t s1 __maybe_unused = HAMMING8_DATA(decode_hamming8_code(data[2]));
		uint8_t s2 __maybe_unused = HAMMING8_DATA(decode_hamming8_code(data[3])) & 0x7;
		uint8_t s3 __maybe_unused = HAMMING8_DATA(decode_hamming8_code(data[4]));
		uint8_t s4 __maybe_unused = HAMMING8_DATA(decode_hamming8_code(data[5])) & 0x3;
		uint8_t erase __maybe_unused = (HAMMING8_DATA(decode_hamming8_code(data[3])) >> 3) & 0x1;
		uint8_t newflash __maybe_unused = (HAMMING8_DATA(decode_hamming8_code(data[5])) >> 2) & 0x1;
		uint8_t subtitle __maybe_unused = (HAMMING8_DATA(decode_hamming8_code(data[5])) >> 3) & 0x1;
		uint8_t supress_header __maybe_unused = HAMMING8_DATA(decode_hamming8_code(data[6])) & 0x1;
		uint8_t update_indicator __maybe_unused = (HAMMING8_DATA(decode_hamming8_code(data[6])) >> 1) & 0x1;
		uint8_t interrupted_sequence __maybe_unused = (HAMMING8_DATA(decode_hamming8_code(data[6])) >> 2) & 0x1;
		uint8_t inhibit_display __maybe_unused = (HAMMING8_DATA(decode_hamming8_code(data[6])) >> 3) & 0x1;
		uint8_t magazine_serial __maybe_unused = (HAMMING8_DATA(decode_hamming8_code(data[7]))) & 0x1;
		uint8_t national_option __maybe_unused = (HAMMING8_DATA(decode_hamming8_code(data[7])) >> 1) & 0x7;
		// from data 8 to 39, carry 32 bytes character or display control codes
		// byte 32 to 39 are usually coded to represent a real-time clock
		if (erase) {
			for (int j = 1; j <= 28; j++) {
				for (int i = 0; i < 40; i++) {
					composite_teletext_pages(X, P, j, i, ' ');
				}
			}
		}

	} else if (Y > 25) {
		/* non-displayable packets */
		/* Packets with Y = 26 to 31 may also use byte 6 to extend the packet address range.
		 * Byte 6 is then Hamming 8/4 coded and is referred to as the Designation Code.
		 */
		uint8_t designation_code __maybe_unused = HAMMING8_DATA(decode_hamming8_code(data[0]));
		// decode_hamming24_code(data+3)
		// uint32_t triplet = HAMMING24_DATA(data);

		if (Y == 26) {

		} else if (Y == 28) {

		} else if (Y == 30) {
		}
	} else {
		/* normal packets intended for direct display */
		for (int i = 0; i < 40; i++) {
			composite_teletext_pages(X, P, Y, i, decode_odd_parity(data[i]));
		}
	}
	return 0;
}

int parse_teletext(uint16_t pid, uint8_t *pbuf, int len, void *teletext)
{
	uint8_t *pdata = pbuf;
	int t_len = 0;
	struct teletext_pes_data *text = (struct teletext_pes_data *)teletext;
	if (text->units) {
		free(text->units);
	}
	text->data_identifier = TS_READ8(pdata);
	pdata += 1;
	t_len += 1;
	text->n_units = (len - 1) / (2 + sizeof(struct data_field));
	text->units = calloc(text->n_units, sizeof(struct data_unit));
	if (!text->units) {
		return ENOMEM;
	}
	int i = 0, j = 0;
	while (t_len < len) {
		text->units[i].data_unit_id = TS_READ8(pdata);
		assert(text->units[i].data_unit_id == 0x2 || text->units[i].data_unit_id == 0x3 ||
			   text->units[i].data_unit_id == 0xff);
		pdata += 1;
		text->units[i].data_unit_length = TS_READ8(pdata);
		assert(text->units[i].data_unit_length == 0x2c);
		pdata += 1;
		text->units[i].data.field_parity = TS_READ8_BITS(pdata, 1, 2);
		text->units[i].data.line_offset = TS_READ8_BITS(pdata, 5, 3);
		pdata += 1;
		text->units[i].data.framing_code = TS_READ8(pdata);
		pdata += 1;
		text->units[i].data.magazine_and_packet_address = TS_READ16(pdata);
		pdata += 2;
		for (j = 0; j < 40; j++) {
			text->units[i].data.data_block[j] = TS_READ8(pdata);
			pdata += 1;
		}
		parse_teletext_data(text->units[i].data.magazine_and_packet_address, text->units[i].data.data_block);
		t_len += 46;
		i++;
	}
	// dump_teletext_pages(6, 6);
	return 0;
}

void dump_teletext(struct teletext_pes_data *text)
{
	rout(2, "data_identifier", "0x%x", text->data_identifier);
	for (int i = 0; i < text->n_units; i++) {
		rout(3, "data_unit_id", "0x%x", text->units[i].data_unit_id);
		rout(3, "data_unit_length", "%d", text->units[i].data_unit_length);
		rout(3, "field_parity", "%d", text->units[i].data.field_parity);
		rout(3, "line_offset", "%d->%d", text->units[i].data.line_offset,
			 (text->units[i].data.line_offset > 0x6 && text->units[i].data.line_offset < 0x17)
				 ? (text->units[i].data.field_parity ? text->units[i].data.line_offset
													 : text->units[i].data.line_offset + 313)
				 : -1);
		rout(3, "field_parity", "%d", text->units[i].data.field_parity);
		rout(3, "framing_code", "%d", text->units[i].data.framing_code);
		rout(3, "magazine_and_packet_address", "0x%x", text->units[i].data.magazine_and_packet_address);
		res_hexdump(3, "data_block", text->units[i].data.data_block, 40);
	}
}

void free_teletext(struct teletext_pes_data *text)
{
    free(text->units);
}
