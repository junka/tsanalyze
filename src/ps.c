#include <stdint.h>
#include <stdio.h>

#include "ts.h"
#include "ps.h"
#include "pes.h"

int parse_pack(uint8_t *pkt, uint16_t len)
{
	return 0;
}

int parse_ps(uint8_t *pkt, uint16_t len)
{
	uint8_t *buf = pkt;
	while (TS_READ32(buf) == PACK_START) {
		parse_pack(buf, len);
	}
	// TS_READ32(buf)==PROGRAM_END;
	return 0;
}
