#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "types.h"

char *convert_UTC(UTC_time_t *t)
{
	static char str[20] = { 0 };
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

#if __GNUC__ && !defined( __has_warning )
#define SUPPRESSING
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
	snprintf(str, 20, "%04d/%02u/%02u %02d:%02d:%02d", Y, M, D, hour, min, sec);
#ifdef  SUPPRESSING
#pragma GCC diagnostic pop
#endif

	return str;
}

int bitmap64_full(uint64_t *bitmap, uint64_t last)
{
	uint64_t mask = ((uint64_t)1 << ((last + 1) % 64)) - 1;
	if (last >= 64)
	{
		for (uint64_t i = 0; i < last / 64; i ++)
		{
			if (bitmap[i] != UINT64_MAX)
				return -1;
		}
	}
	if ((bitmap[last % 64] & mask) != mask)
		return -1;
	return 0;
}
