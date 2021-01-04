#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "result.h"

#define LINE_LEN (1024)
static uint8_t outtype = RES_STD;

struct res_ops rops[RES_NUM];

int res_settype(int t)
{
	outtype = t;
	return 0;
}

int res_open(const char *filename)
{
	char outfile[LINE_LEN] = {0};
	if(outtype == RES_STD)
		rops[outtype].f = stdout;
	else if (outtype == RES_TXT)
	{
		snprintf(outfile, LINE_LEN, "%s.txt", filename);
		rops[outtype].f = fopen(outfile, "w");
	}
	else if (outtype == RES_JSON)
	{
		snprintf(outfile, LINE_LEN, "%s.json", filename);
		rops[outtype].f = fopen(outfile, "w");
	}
	return 0;
}

int res_put(int lv, const char *fmt, ...)
{
	va_list args;
	char buf[LINE_LEN] = {0};
	int ret = 0;
	va_start(args, fmt);
	if (lv == 0) {
		ret += snprintf(buf + ret, LINE_LEN - ret, "\n");
	}
	while(lv-- > 0) {
		ret += snprintf(buf + ret, LINE_LEN - ret, "  ");
	}
	ret += vsnprintf(buf + ret, LINE_LEN - ret, fmt, args);
	ret += snprintf(buf + ret, LINE_LEN - ret, "\n");
	ret = fprintf(rops[outtype].f, buf);
	va_end(args);
	return ret;    
}

int res_close(void)
{
	if(outtype != RES_STD)
		fclose(rops[outtype].f);
	return 0;
}
