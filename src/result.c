#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "result.h"

#define LINE_LEN (512)

static uint8_t outtype = RES_STD;

struct res_ops rops[RES_NUM];

#define MAX_DEPTH (16)

int res_settype(int t)
{
	outtype = t;
	return 0;
}

int res_open(const char *filename)
{
	char outfile[LINE_LEN] = {0};
	switch (outtype) {
		case RES_TXT:
			snprintf(outfile, LINE_LEN, "%s.txt", filename);
			rops[outtype].f = fopen(outfile, "w");
			break;
		case RES_JSON:
			snprintf(outfile, LINE_LEN, "%s.json", filename);
			rops[outtype].f = fopen(outfile, "w");
			break;
		case RES_STD:
		default:
			rops[outtype].f = stdout;
			break;
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
	if (lv == 0) {
		out = snprintf(line, LINE_LEN, outtype == RES_JSON ? "\n{\"" : "\n");
	}
	while(j-- > 0) {
		n = snprintf(line + out, LINE_LEN - out, "  ");
		if (n < 0 || n >= LINE_LEN - out)
			break;
		out += n;
	}
	if (outtype == RES_JSON) {
		n = snprintf(line + out, LINE_LEN - out, "\"%s (len %u)\":\n\"", title, len);
	} else {
		n = snprintf(line + out, LINE_LEN - out, "%s: len %u\n", title, len);
	}
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
		if (outtype == RES_JSON) {
			n = snprintf(line + out, LINE_LEN - out, "\\n");
		} else {
			n = snprintf(line + out, LINE_LEN - out, "\n");
		}
		if (n < 0 || n >= LINE_LEN - out)
			goto printline;
		out += n;
printline:
		fprintf(rops[outtype].f, "%s", line);
		out = 0;
	}
	if (len > 0 && outtype == RES_JSON) {
		fprintf(rops[outtype].f, "\"");
	}
}

int res_put(int lv, const char * key, const char *fmt, ...)
{
	static int last_put_lv = -1;
	static bool prev_fmt_null = true;
	static char comma[128] = {0};
	static int lv_dep[MAX_DEPTH] = {0};
	static int depth = 0;
	int lv_ = lv;
	va_list args;
	char buf[LINE_LEN] = {0};
	int ret = 0;
	int n = 0;
	if (fmt)
		va_start(args, fmt);
	if (lv == 0) {
		prev_fmt_null = true;
	}
	while(lv_-- > 0) {
		n = snprintf(buf + ret, LINE_LEN - ret, "  ");
		if (n < 0 || n >= LINE_LEN - ret)
			goto end;
		ret += n;
	}
	if (outtype == RES_JSON) {
		if (lv > last_put_lv) {
			if (prev_fmt_null) {
				if (ret + 1 > LINE_LEN)
					goto end;
				buf[ret++] = '{';
				/* push a pair */
				comma[depth++] = '}';
				lv_dep[lv] ++;
			}
		} else if (lv < last_put_lv) {
			for (int i = last_put_lv; i >= lv; i --) {
				for (int j = lv_dep[i]; j > 0; j --) {
					if (ret + 2 > LINE_LEN)
						goto end;
					buf[ret++] = comma[--depth];
					buf[ret++] = ',';
					lv_dep[i]--;
				}
			}
			if (key) {
				if (ret + 1 > LINE_LEN)
					goto end;
				buf[ret++] = '{';
				/* push a pair */
				comma[depth++] = '}';
				lv_dep[lv] ++;
			}
		}
		if (lv == 0) {
			if (ret + 1 > LINE_LEN)
				goto end;
			buf[ret++] = '\n';
		}
		if (key || lv > 0) {
			if (ret + 1 > LINE_LEN)
				goto end;
			buf[ret++] = '\"';
		}
	}
	if (lv == 0 && outtype != RES_JSON) {
		n = snprintf(buf + ret, LINE_LEN - ret, "\n");
		if (n < 0 || n >= LINE_LEN - ret)
			goto end;
		ret += n;
	}
	if (key) {
		n = snprintf(buf + ret, LINE_LEN - ret, "%s", key);
		if (n < 0 || n >= LINE_LEN - ret)
			goto end;
		ret += n;
	}
	if (fmt) {
		if (key) {
			if (outtype == RES_JSON) {
				if (ret + 1 > LINE_LEN)
					goto end;
				buf[ret ++] = '\"';
			}
			if (ret + 2 > LINE_LEN)
				goto end;
			buf[ret++] = ':';
			buf[ret++] = '\t';
			if (outtype == RES_JSON) {
				if (ret + 1 > LINE_LEN)
					goto end;
				buf[ret ++] = '\"';
			}
		}
		n = vsnprintf(buf + ret, LINE_LEN - ret, fmt, args);
		if (n < 0 || n >= (LINE_LEN - ret))
			goto end;
		ret += n;
		if (outtype == RES_JSON) {
			if (ret + 2 > LINE_LEN)
				goto end;
			buf[ret++] = '\"';
			buf[ret++] = ',';
		}
		prev_fmt_null = false;
	}
	if (outtype == RES_JSON) {
		if (lv == 0 && key) {
			if (ret + 2 > LINE_LEN)
				goto end;
			buf[ret++] = '\"';
			buf[ret++] = ':';
		}
		if (!fmt) {
			if (key && lv > 0) {
				/* means array list following */
				if (ret + 3 > LINE_LEN)
					goto end;
				buf[ret++] = '\"';
				buf[ret++] = ':';
				buf[ret ++] = '[';
				comma[depth++] = ']';
				lv_dep[lv]++;
			}
			prev_fmt_null = true;
		}
	}
	if (ret + 1 > LINE_LEN)
		goto end;
	buf[ret++] = '\n';

end:
	buf[ret] = '\0';
	ret = fprintf(rops[outtype].f, "%s", buf);
	if (fmt)
		va_end(args);
	last_put_lv = lv;
	return ret;
}

int res_close(void)
{
	if (outtype == RES_JSON)
		rout(0, NULL, NULL);
	if(outtype != RES_STD)
		fclose(rops[outtype].f);
	return 0;
}
