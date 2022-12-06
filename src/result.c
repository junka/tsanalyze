#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "result.h"

#define LINE_LEN (512)

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

void res_hexdump(int lv, char * title, uint8_t *buf, uint32_t len)
{
	unsigned int i, ofs, j = lv;
	const unsigned char *data = buf;
	char line[LINE_LEN] = {0}; /* space needed 8+16*3+3+16 == 75 */
	int n = 0, out = 0;

	ofs = 0;
	if (j == 0) {
		out = snprintf(line, LINE_LEN, "\n");
	}
	while(j-- > 0) {
		n = snprintf(line + out, LINE_LEN - out, "  ");
		if (n < 0 || n >= LINE_LEN - out)
			break;
		out += n;
	}
	n = snprintf(line + out, LINE_LEN - out, "%s: len %u\n", title, len);
	if (n < 0 || n >= LINE_LEN - out) {
		fprintf(rops[outtype].f, "title too long: %s", title);
		return;
	}
	out += n;

	while (ofs < len) {
		j = lv;
		while(j-- > 0) {
			n = snprintf(line + out, LINE_LEN - out, "  ");
			if (n < 0 || n >= LINE_LEN - out)
				goto printline;
			out += n;
		}
		/* format the line in the buffer, then use printf to output to screen */
		out += snprintf(line + out, LINE_LEN - out, "%08X:", ofs);
		for (i = 0; ((ofs + i) < len) && (i < 16); i++) {
			n = snprintf(line + out, LINE_LEN - out, " %02X", (data[ofs + i] & 0xff));
			if (n < 0 || n >= LINE_LEN - out)
				goto printline;
			out += n;
		}
		for (; i <= 16; i++) {
			n = snprintf(line + out, LINE_LEN - out, " | ");
			if (n < 0 || n >= LINE_LEN - out)
				goto printline;
			out += n;
		}
		for (i = 0; (ofs < len) && (i < 16); i++, ofs++) {
			unsigned char c = data[ofs];
			if ((c < ' ') || (c > '~'))
				c = '.';
			n = snprintf(line + out, LINE_LEN - out, "%c", c);
			if (n < 0 || n >= LINE_LEN - out)
				goto printline;
			out += n;
		}
		n = snprintf(line + out, LINE_LEN - out, "\n");
		if (n < 0 || n >= LINE_LEN - out)
			goto printline;
		out += n;
printline:
		fprintf(rops[outtype].f, "%s", line);
		out = 0;
	}
}

int res_put(int lv, const char *fmt, ...)
{
	va_list args;
	char buf[LINE_LEN] = {0};
	int ret = 0, n = 0;
	va_start(args, fmt);
	if (lv == 0) {
		ret += snprintf(buf, LINE_LEN, "\n");
	}
	while(lv-- > 0) {
		n = snprintf(buf + ret, LINE_LEN - ret, "  ");
		if (n < 0 || n >= LINE_LEN - ret)
			goto end;
		ret += n;
	}
	n = vsnprintf(buf + ret, LINE_LEN - ret, fmt, args);
	if (n < 0 || n >= LINE_LEN - ret)
		goto end;
	ret += n;
	n = snprintf(buf + ret, LINE_LEN - ret, "\n");
	if (n < 0 || n >= LINE_LEN - ret)
		goto end;
	ret += n;

end:
	ret = fprintf(rops[outtype].f, "%s", buf);
	va_end(args);
	return ret;
}

int res_close(void)
{
	if(outtype != RES_STD)
		fclose(rops[outtype].f);
	return 0;
}
