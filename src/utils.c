#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "utils.h"

#define LINE_LEN 128

void hexdump(uint8_t *buf, uint32_t len)
{
	unsigned int i, out, ofs;
	const unsigned char *data = buf;
	char line[LINE_LEN]; /* space needed 8+16*3+3+16 == 75 */

	ofs = 0;
	while (ofs < len) {
		/* format the line in the buffer, then use printf to output to screen */
		out = snprintf(line, LINE_LEN, "%08X:", ofs);
		for (i = 0; ((ofs + i) < len) && (i < 16); i++)
			out += snprintf(line + out, LINE_LEN - out, " %02X", (data[ofs + i] & 0xff));
		for (; i <= 16; i++)
			out += snprintf(line + out, LINE_LEN - out, " | ");
		for (i = 0; (ofs < len) && (i < 16); i++, ofs++) {
			unsigned char c = data[ofs];
			if ((c < ' ') || (c > '~'))
				c = '.';
			out += snprintf(line + out, LINE_LEN - out, "%c", c);
		}
		fprintf(stdout, "%s\n", line);
	}
	fflush(stdout);
}

char *convert_UTC(UTC_time_t *t)
{
	static char str[19] = { 0 };
	/*16bits lsb MJB + 24 bits BCD*/
	uint16_t mjd = (((uint16_t)t->time[0] << 8) | (t->time[1]));
	uint8_t hour = t->time[2];
	uint8_t min = t->time[3];
	uint8_t sec = t->time[4];

	/*see en300_468 Annex C*/
	int Y, M, D;
	int K;
	int Y1 = (int)((mjd - 15078.2) / 365.25);
	int M1 = (int)((mjd - 14956.1 - (int)(Y1 * 365.25)) / 30.6001);
	D = mjd - 14956 - (int)(Y1 * 365.25) - (int)(M1 * 30.6001);
	if (M1 == 14 || M1 == 15)
		K = 1;
	else
		K = 0;
	Y = Y1 + K + 1900;
	M = M1 - 1 - K * 12;

	snprintf(str, 18, "%d/%d/%d %02x:%02x:%02x", Y, M, D, hour, min, sec);
	return str;
}
