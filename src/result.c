#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "result.h"

#define LINE_LEN (512)
#define MAX_DEPTH (32)

static int outtype = RES_STD;

struct res_ops rops[RES_NUM];

/* ------------------------------------------------------------------ *
 * JSON container stack.
 *
 * The JSON output is built as a strict nested tree.  Every res_put()
 * call is one of:
 *   - (lv, key, NULL)          : open a container named "key"
 *   - (lv, key, fmt, ...)      : emit a leaf field  "key": "value"
 *   - (lv, NULL, fmt, ...)     : emit an array element "value"
 *   - (0, NULL, NULL)          : res_close() signal, closes the root
 *
 * A container opened at level L is an ARRAY when its first child is an
 * array element (key == NULL), otherwise it is an OBJECT.  Because the
 * type is only known once the first child arrives, the opening bracket
 * is deferred ("undecided") until the next call.  Commas are emitted as
 * LEADING separators (before each child except the very first), which
 * avoids the trailing-comma problem of the old emitter.
 * ------------------------------------------------------------------ */
struct jcnt {
	int lv;          /* level at which the container marker was emitted  */
	int child_lv;    /* level of the container's children (-1: undecided) */
	char close;      /* closing char: '}' or ']'                          */
	int has_child;   /* 1 once at least one child has been emitted        */
};
static struct jcnt jstack[MAX_DEPTH];
static int jdepth = 0;

static void j_write_indent(FILE *f, int n)
{
	while (n-- > 0)
		fputc(' ', f);
}

/* Escape a string as a JSON string-body (no surrounding quotes). */
static void j_write_escaped(FILE *f, const char *s)
{
	const unsigned char *p = (const unsigned char *)s;
	while (*p) {
		unsigned char c = *p++;
		switch (c) {
		case '"':  fputs("\\\"", f); break;
		case '\\': fputs("\\\\", f); break;
		case '\n': fputs("\\n", f);  break;
		case '\r': fputs("\\r", f);  break;
		case '\t': fputs("\\t", f);  break;
		default:
			if (c < 0x20)
				fprintf(f, "\\u%04x", c);
			else
				fputc(c, f);
		}
	}
}

/*
 * Common prefix for every JSON child emission at level `lv`.
 *
 * The caller's convention is:
 *   - a leaf / array element (res_put with fmt != NULL) is a CHILD of the
 *     innermost open container whose level is <= lv.  So it closes every
 *     container opened at level > lv (we have ascended past it), but keeps
 *     a container also at level lv (same-level array elements, e.g. the
 *     PAT "program_number @ PMT_PID" list).
 *   - a container-open (res_put with fmt == NULL) at level lv is a SIBLING
 *     of any container opened at level >= lv, so those are closed first.
 *
 * `is_array_element` only drives the bracket type when the containing
 * container is still "undecided": an array element makes it '[' / ']',
 * anything else makes it '{' / '}'.
 */
static void j_prelude(int lv, int is_array_element)
{
	FILE *f = rops[RES_JSON].f;

	/* 1. close every container that we have ascended out of.
	 *    A leaf keeps containers at level <= lv; a container-open also
	 *    closes containers at the same level (they are siblings). */
	int min_close = is_array_element ? (lv + 1) : lv;
	while (jdepth > 1) {
		struct jcnt *t = &jstack[jdepth - 1];
		if (t->lv < min_close)
			break;
		fputc('\n', f);
		j_write_indent(f, 2 * (jdepth - 1));
		if (t->child_lv < 0)
			fputs("[]", f);       /* undecided container: empty */
		else
			fputc(t->close, f);
		jdepth--;
	}

	/* 1b. a keyed member (leaf or container-open) cannot be a direct child
	 * of a resolved ARRAY, so wrap it in a fresh object element.  This is
	 * the "transport_streams" pattern: a bare string element at one level
	 * followed by keyed members one level deeper. */
	if (!is_array_element && jdepth > 1 && jstack[jdepth - 1].close == ']') {
		struct jcnt *arr = &jstack[jdepth - 1];
		if (arr->has_child)
			fputc(',', f);
		fputc('\n', f);
		j_write_indent(f, 2 * (jdepth - 1));
		fputc('{', f);
		jstack[jdepth].lv = lv;
		jstack[jdepth].child_lv = lv;
		jstack[jdepth].close = '}';
		jstack[jdepth].has_child = 0;
		jdepth++;
		arr->has_child = 1;
	}

	/* 2. resolve an undecided container that is about to receive this child */
	if (jdepth > 1 && jstack[jdepth - 1].child_lv < 0) {
		struct jcnt *t = &jstack[jdepth - 1];
		t->child_lv = lv;
		t->close = is_array_element ? ']' : '}';
		t->has_child = 0;
		fputc(is_array_element ? '[' : '{', f);
	}

	/* 3. leading comma + indentation for this child */
	struct jcnt *parent = &jstack[jdepth - 1];
	if (parent->has_child)
		fputc(',', f);
	fputc('\n', f);
	j_write_indent(f, 2 * (jdepth - 1));
	parent->has_child = 1;
}

/* Emit "key": and push an undecided container as its value. */
static void j_open_container(int lv, const char *key)
{
	FILE *f = rops[RES_JSON].f;
	j_prelude(lv, 0);
	fputc('"', f);
	j_write_escaped(f, key);
	fputc('"', f);
	fputc(':', f);
	if (jdepth >= MAX_DEPTH)
		return;
	jstack[jdepth].lv = lv;
	jstack[jdepth].child_lv = -1;
	jstack[jdepth].close = 0;
	jstack[jdepth].has_child = 0;
	jdepth++;
}

/* Begin a leaf field ("key": " ... ) or array element (" ... ). */
static void j_begin_leaf(int lv, const char *key)
{
	FILE *f = rops[RES_JSON].f;
	j_prelude(lv, key == NULL);
	if (key) {
		fputc('"', f);
		j_write_escaped(f, key);
		fputc('"', f);
		fputc(':', f);
	}
	fputc('"', f);
}

static void j_end_value(void)
{
	fputc('"', rops[RES_JSON].f);
}

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
			if (rops[outtype].f) {
				/* implicit root object */
				jdepth = 1;
				memset(jstack, 0, sizeof(jstack));
				jstack[0].lv = -1;
				jstack[0].child_lv = 0;
				jstack[0].close = '}';
				fputc('{', rops[outtype].f);
			}
			break;
		case RES_STD:
		default:
			rops[outtype].f = stdout;
			break;
	}
	return 0;
}

void res_hexdump(int lv, const char *title, uint8_t *buf, uint32_t len)
{
	unsigned int i, ofs, j;
	const unsigned char *data = buf;

	if (outtype == RES_JSON) {
		FILE *f = rops[RES_JSON].f;
		j_begin_leaf(lv, title);
		for (ofs = 0; ofs < len; ) {
			if (ofs > 0)
				fputs("\\n", f);
			fprintf(f, "%08x:", ofs);
			for (i = 0; i < 16 && ofs + i < len; i++)
				fprintf(f, " %02x", data[ofs + i]);
			for (; i < 16; i++)
				fputs("   ", f);
			fputs("  | ", f);
			for (i = 0; i < 16 && ofs < len; i++, ofs++) {
				unsigned char c = data[ofs];
				if (c == '"' || c == '\\')
					fprintf(f, "\\%c", c);
				else if (c >= 0x20 && c < 0x7f)
					fputc(c, f);
				else
					fputc('.', f);
			}
		}
		j_end_value();
		return;
	}

	/* non-JSON output keeps the classic hexdump layout */
	char line[LINE_LEN] = {0};
	int n = 0, out = 0;
	ofs = 0;
	if (lv == 0)
		out = snprintf(line, LINE_LEN, "\n");
	j = lv;
	while (j-- > 0) {
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
		while (j-- > 0) {
			n = snprintf(line + out, LINE_LEN - out, "  ");
			if (n < 0 || n >= LINE_LEN - out)
				goto printline;
			out += n;
		}
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

int res_put(int lv, const char *key, const char *fmt, ...)
{
	va_list args;
	int n = 0;
	char buf[LINE_LEN] = {0};
	int ret = 0;

	if (fmt)
		va_start(args, fmt);

	if (outtype == RES_JSON) {
		FILE *f = rops[RES_JSON].f;

		/* res_close() signal: lv==0, key==NULL, fmt==NULL -> close root */
		if (lv == 0 && key == NULL && fmt == NULL) {
			while (jdepth > 1) {
				struct jcnt *t = &jstack[jdepth - 1];
				fputc('\n', f);
				j_write_indent(f, 2 * (jdepth - 1));
				if (t->child_lv < 0)
					fputs("[]", f);
				else
					fputc(t->close, f);
				jdepth--;
			}
			fputc('\n', f);
			fputc('}', f);
			fputc('\n', f);
			jdepth = 0;
			if (fmt)
				va_end(args);
			return 0;
		}

		if (key && !fmt) {
			/* open a container */
			j_open_container(lv, key);
		} else if (fmt) {
			/* leaf field or array element */
			char valbuf[LINE_LEN] = {0};
			vsnprintf(valbuf, LINE_LEN, fmt, args);
			j_begin_leaf(lv, key);
			j_write_escaped(f, valbuf);
			j_end_value();
		}
		if (fmt)
			va_end(args);
		return 0;
	}

	/* ---- non-JSON output (text / plain) ---- */
	int lv_ = lv;
	while (lv_-- > 0) {
		n = snprintf(buf + ret, LINE_LEN - ret, "  ");
		if (n < 0 || n >= LINE_LEN - ret)
			goto end;
		ret += n;
	}
	if (lv == 0) {
		if (ret + 1 > LINE_LEN)
			goto end;
		buf[ret++] = '\n';
	}
	if (key) {
		n = snprintf(buf + ret, LINE_LEN - ret, "%s", key);
		if (n < 0 || n >= LINE_LEN - ret)
			goto end;
		ret += n;
	}
	if (fmt) {
		if (key) {
			if (ret + 2 > LINE_LEN)
				goto end;
			buf[ret++] = ':';
			buf[ret++] = '\t';
		}
		n = vsnprintf(buf + ret, LINE_LEN - ret, fmt, args);
		if (n < 0 || n >= (LINE_LEN - ret))
			goto end;
		ret += n;
	}
	if (ret + 1 > LINE_LEN)
		goto end;
	buf[ret++] = '\n';

end:
	buf[ret] = '\0';
	ret = fprintf(rops[outtype].f, "%s", buf);
	if (fmt)
		va_end(args);
	return ret;
}

int res_close(void)
{
	if (outtype == RES_JSON)
		rout(0, NULL, NULL);
	if (outtype != RES_STD)
		fclose(rops[outtype].f);
	return 0;
}