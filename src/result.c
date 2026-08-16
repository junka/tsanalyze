#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "result.h"

#define LINE_LEN (512)
#define MAX_DEPTH (32)

static int outtype = RES_STD;
static char root_close = '}';   /* closing bracket of the JSON/HTML root */
static int yfirst = 1;           /* first YAML line has no leading newline */

struct res_ops rops[RES_NUM];

static FILE *curf(void)
{
	return rops[outtype].f;
}

static int is_hier(void)
{
	return outtype == RES_JSON || outtype == RES_HTML || outtype == RES_YAML;
}

/* ------------------------------------------------------------------ *
 * Shared container stack.
 *
 * The hierarchical output (JSON / HTML / YAML) is built as a strict
 * nested tree.  Every res_put() call is one of:
 *   - (lv, key, NULL)          : open a container named "key"
 *   - (lv, key, fmt, ...)      : emit a leaf field  "key": "value"
 *   - (lv, NULL, fmt, ...)     : emit an array element "value"
 *   - (0, NULL, NULL)          : res_close() signal, closes the root
 *
 * A container opened at level L is an ARRAY when its first child is an
 * array element (key == NULL), otherwise it is an OBJECT.  Because the
 * type is only known once the first child arrives, the opening is said
 * to be "undecided".  Commas are emitted as LEADING separators.
 * ------------------------------------------------------------------ */
/* a single mapping key observed inside a container (used for YAML dedup) */
struct keyrec {
	char *key;   /* the original key text */
	int count;   /* how many times it has appeared */
};

struct jcnt {
	int lv;          /* level at which the container marker was emitted  */
	int child_lv;    /* level of the container's children (-1: undecided) */
	char close;      /* closing char: '}' or ']'                          */
	int has_child;   /* 1 once at least one child has been emitted        */
	struct keyrec *ek;  /* YAML duplicate-key records for this mapping    */
	int nk, ck;         /* used / allocated entries in ek                 */
};
static struct jcnt jstack[MAX_DEPTH];
static int jdepth = 0;

static void j_write_indent(FILE *f, int n)
{
	while (n-- > 0)
		fputc(' ', f);
}

/* Escape a string as both a JSON and a YAML double-quoted string body. */
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

/* strdup() re-implemented so it also works under MSVC */
static char *jdup(const char *s)
{
	size_t n = strlen(s) + 1;
	char *p = malloc(n);
	if (p)
		memcpy(p, s, n);
	return p;
}

/* release the key records attached to a container */
static void jcnt_key_free(struct jcnt *t)
{
	if (t->ek) {
		int i;
		for (i = 0; i < t->nk; i++)
			free(t->ek[i].key);
		free(t->ek);
		t->ek = NULL;
		t->nk = t->ck = 0;
	}
}

/*
 * YAML forbids duplicate mapping keys.  Track the keys already emitted into
 * the current mapping `m`; on a repeat, hand back "key #N" so every key stays
 * unique while no data is dropped.  `buf` is used only for the repeat case.
 */
static const char *y_dedup_key(struct jcnt *m, const char *key,
			       char *buf, size_t bufsz)
{
	int i;
	for (i = 0; i < m->nk; i++) {
		if (strcmp(m->ek[i].key, key) == 0) {
			m->ek[i].count++;
			snprintf(buf, bufsz, "%s #%d", key, m->ek[i].count);
			return buf;
		}
	}
	if (m->nk >= m->ck) {
		int nc = m->ck ? m->ck * 2 : 8;
		struct keyrec *nk = realloc(m->ek, (size_t)nc * sizeof(*nk));
		if (!nk)
			return key;
		m->ek = nk;
		m->ck = nc;
	}
	m->ek[m->nk].key = jdup(key);
	m->ek[m->nk].count = 1;
	if (!m->ek[m->nk].key)
		return key;
	m->nk++;
	return key;
}

/*
 * JSON/HTML prefix for every child emission at level `lv`.  Maintains the
 * shared stack exactly as described above; only emits JSON bytes.
 */
static void j_prelude(FILE *f, int lv, int is_array_element)
{
	/* 1. close every container that we have ascended out of. */
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

	/* 1b. a keyed member cannot be a direct child of a resolved ARRAY, so
	 * wrap it in a fresh object element ("transport_streams" pattern).
	 * Also applies when the root itself is an array (HTML mode). */
	if (!is_array_element && jdepth >= 1 && jstack[jdepth - 1].close == ']') {
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

	/* 2. resolve an undecided container that is about to receive a child */
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

/* Emit a JSON/HTML leaf field or array element, or open a container. */
static void j_emit(FILE *f, int lv, const char *key, const char *val)
{
	if (key && !val) {
		/* open a container */
		j_prelude(f, lv, 0);
		fputc('"', f);
		j_write_escaped(f, key);
		fputs("\":", f);
		if (jdepth >= MAX_DEPTH)
			return;
		jstack[jdepth].lv = lv;
		jstack[jdepth].child_lv = -1;
		jstack[jdepth].close = 0;
		jstack[jdepth].has_child = 0;
		jdepth++;
	} else if (val) {
		/* leaf field or array element */
		j_prelude(f, lv, key == NULL);
		if (key) {
			fputc('"', f);
			j_write_escaped(f, key);
			fputs("\":", f);
		}
		fputc('"', f);
		j_write_escaped(f, val);
		fputc('"', f);
	}
}

/* ------------------------------------------------------------------ *
 * YAML emitter.  Mirrors the JSON topology (same jstack), but emits
 * block-style YAML:  "key": value  /  - value  /  "key": <block>.
 * The "1b" wrap of the JSON path is represented by a '-' dash item that
 * itself is an implicit mapping.
 * ------------------------------------------------------------------ */
static void y_emit(FILE *f, int lv, const char *key, const char *val)
{
	int is_arr = (key == NULL);

	/* seal: ascend out of containers as the JSON path does */
	int min_close = is_arr ? (lv + 1) : lv;
	while (jdepth > 1) {
		struct jcnt *t = &jstack[jdepth - 1];
		if (t->lv < min_close)
			break;
		jcnt_key_free(t);
		jdepth--;
	}
	int indent = 2 * (jdepth - 1);

	int parent_arr = (jdepth >= 1 && jstack[jdepth - 1].close == ']');
	int wrap = parent_arr && key != NULL;   /* keyed child inside a resolved array */

	/* 1b wrap: push an invisible object so deeper members nest one level */
	if (wrap) {
		if (jdepth >= MAX_DEPTH - 1)
			return;
		jstack[jdepth].lv = lv;
		jstack[jdepth].child_lv = lv;
		jstack[jdepth].close = '}';
		jstack[jdepth].has_child = 0;
		jdepth++;
	}

	/* resolve an undecided container based on this child */
	if (jdepth > 1 && jstack[jdepth - 1].child_lv < 0) {
		struct jcnt *t = &jstack[jdepth - 1];
		t->child_lv = lv;
		t->close = is_arr ? ']' : '}';
		t->has_child = 0;
	}

	/* leading line */
	/* dedupe the mapping key against its parent container (YAML rule) */
	const char *ekey = key;
	char kbuf[64];
	if (key)
		ekey = y_dedup_key(&jstack[jdepth - 1], key, kbuf, sizeof(kbuf));

	if (yfirst)
		yfirst = 0;
	else
		fputc('\n', f);
	j_write_indent(f, indent);
	if (is_arr || wrap)
		fputs("- ", f);
	if (key) {
		fputc('"', f);
		j_write_escaped(f, ekey);
		fputs("\":", f);
		if (val)
			fputc(' ', f);
	}
	if (val) {
		fputc('"', f);
		j_write_escaped(f, val);
		fputc('"', f);
	}

	/* push the current container if we just opened one */
	if (key && !val) {
		if (jdepth >= MAX_DEPTH)
			return;
		jstack[jdepth].lv = lv;
		jstack[jdepth].child_lv = -1;
		jstack[jdepth].close = 0;
		jstack[jdepth].has_child = 0;
		jdepth++;
	}
}

/* Close signal: pop every open container, then the root bracket. */
static void hier_close(FILE *f)
{
	if (outtype == RES_YAML) {
		int i;
		for (i = 0; i < jdepth; i++)
			jcnt_key_free(&jstack[i]);
		jdepth = 0;
		return;
	}
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
	fputc(root_close, f);
	fputc('\n', f);
	jdepth = 0;
}

static void res_init_root(void)
{
	jdepth = 1;
	memset(jstack, 0, sizeof(jstack));
	jstack[0].lv = -1;
	jstack[0].child_lv = 0;
}

int res_settype(int t)
{
	outtype = t;
	return 0;
}

/* HTML head + opening of the <script> that will hold the JSON array.
 * Written once at open, the trailing part is appended at close. */
static const char html_head[] =
	"<!DOCTYPE html>\n"
	"<html lang=\"en\">\n"
	"<head>\n"
	"<meta charset=\"utf-8\">\n"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
	"<title>TS Table Viewer</title>\n"
	"<style>\n"
	"body{font-family:ui-monospace,Menlo,monospace;margin:0;background:#0f1117;color:#e6e6e6;}\n"
	"header{position:sticky;top:0;background:#171a23;border-bottom:1px solid #2a2e3a;padding:10px 16px;z-index:5;}\n"
	"header h1{margin:0;font-size:16px;font-weight:600;}\n"
	"header .sub{color:#8a93a6;font-size:12px;margin-top:2px;}\n"
	"#app{padding:12px;max-width:1200px;margin:0 auto;}\n"
	".grp{border:1px solid #262b38;border-radius:8px;margin:10px 0;background:#14171f;overflow:hidden;}\n"
	".gh{padding:8px 12px;background:#1a1e29;display:flex;align-items:center;gap:10px;flex-wrap:wrap;}\n"
	".gname{font-weight:700;color:#7fd0ff;}\n"
	".gmeta{color:#8a93a6;font-size:12px;}\n"
	".pg{display:flex;align-items:center;gap:8px;padding:6px 12px;border-top:1px solid #262b38;background:#161a24;}\n"
	".pg button{background:#242a3a;border:1px solid #3a415a;color:#e6e6e6;border-radius:5px;padding:3px 10px;cursor:pointer;}\n"
	".pg button:hover{background:#303850;}\n"
	".pg .info{margin:0 6px;font-size:13px;color:#8a93a6;}\n"
	".pg .vchips{display:flex;gap:4px;flex-wrap:wrap;}\n"
	".pg .vchip{font-size:11px;padding:2px 6px;border-radius:10px;background:#242a3a;color:#9fb0c8;}\n"
	".tree,.tree ul{list-style:none;margin:0;padding-left:18px;}\n"
	".tree>ul{padding-left:6px;}\n"
	".node{margin:0;padding:1px 0;}\n"
	".node.c>.kids{display:none;}\n"
	".tw{display:inline-block;width:16px;height:16px;line-height:15px;text-align:center;cursor:pointer;\n"
	"     border:1px solid #3a415a;border-radius:3px;color:#9fb0c8;font-size:12px;user-select:none;margin-right:6px;}\n"
	".tw.leaf{visibility:hidden;}\n"
	".k{color:#d7b66f;}\n"
	".sep{color:#6b7280;}\n"
	".v{color:#8effa1;word-break:break-all;}\n"
	".meta{color:#5c6474;font-size:11px;}\n"
	"pre{margin:2px 0 2px 22px;background:#0b0d12;border:1px solid #2a2e3a;border-radius:4px;\n"
	"    padding:6px;overflow:auto;color:#c9d1e8;font-size:12px;}\n"
	"</style>\n"
	"</head>\n"
	"<body>\n"
	"<header><h1>TS Table Viewer</h1><div class=\"sub\" id=\"sub\"></div></header>\n"
	"<div id=\"app\"></div>\n"
	"<script type=\"application/json\" id=\"tsdata\">\n";

static const char html_foot[] =
	"\n</script>\n"
	"<script>\n"
	"(function(){\n"
	"var raw=document.getElementById('tsdata').textContent;\n"
	"var data=JSON.parse(raw);\n"
	"var groups={},order=[];\n"
	"data.forEach(function(el){var k=Object.keys(el)[0];if(!groups[k]){groups[k]=[];order.push(k);}groups[k].push(el[k]);});\n"
	"function esc(s){return (''+s).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');}\n"
	"function typeOf(v){return Array.isArray(v)?'Array':(v&&typeof v==='object'?'Object':'Scalar');}\n"
	"function sizeOf(v){return Array.isArray(v)?v.length:(v&&typeof v==='object'?Object.keys(v).length:0);}\n"
	"function node(key,val){\n"
	"  if(val&&typeof val==='object'){\n"
	"    var n=sizeOf(val)>0;\n"
	"    var html='<li class=\"node\"><span class=\"tw'+(n?'':' leaf')+'\">'+(n?'-':' ')+'</span>';\n"
	"    if(key)html+='<span class=\"k\">'+esc(key)+'</span>';\n"
	"    html+=' <span class=\"meta\">'+typeOf(val)+' '+sizeOf(val)+'</span>';\n"
	"    html+='<ul class=\"kids\">';\n"
	"    if(Array.isArray(val))val.forEach(function(v){html+=node('',v);});\n"
	"    else Object.keys(val).forEach(function(k2){html+=node(k2,val[k2]);});\n"
	"    html+='</ul></li>';return html;\n"
	"  }else{\n"
	"    var s=val===null?'null':(''+val);\n"
	"    var html='<li class=\"node\"><span class=\"tw leaf\">&nbsp;</span>';\n"
	"    if(key)html+='<span class=\"k\">'+esc(key)+'</span><span class=\"sep\">:&nbsp;</span>';\n"
	"    if(s.indexOf('\\n')>=0)html+='<pre>'+esc(s)+'</pre>';\n"
	"    else if(typeOf(val)==='Scalar')html+='<span class=\"v\">&quot;'+esc(s)+'&quot;</span>';\n"
	"    else html+='<span class=\"v\">'+esc(s)+'</span>';\n"
	"    html+='</li>';return html;\n"
	"  }\n"
	"}\n"
	"function setPage(tb,info,cv,b0,b1,list,cur){\n"
	"  tb.innerHTML=node(list[cur].name,list[cur].value);\n"
	"  info.textContent=(cur+1)+' / '+list.length;\n"
	"  cv.textContent=extver(list[cur].value);\n"
	"  b0.disabled=cur===0;\n"
	"  b1.disabled=cur===list.length-1;\n"
	"}\n"
	"function extver(o){if(!o||typeof o!=='object')return'';var k=Object.keys(o).filter(function(x){return /version/i.test(x);})[0];\n"
	"  return k&&o[k]!==undefined?(' v'+(o[k]==null?'?':(''+o[k]).trim())):'';}\n"
	"var app=document.getElementById('app');\n"
	"order.forEach(function(name){\n"
	"  var vs=groups[name];\n"
	"  var list=vs.map(function(v){return{name:name,value:v};});\n"
	"  var sv=document.createElement('section');sv.className='grp';\n"
	"  var gh=document.createElement('div');gh.className='gh';\n"
	"  gh.innerHTML='<span class=\"gname\">'+esc(name)+'</span><span class=\"gmeta\">'+vs.length+' version(s)</span>';\n"
	"  var pg=document.createElement('div');pg.className='pg';\n"
	"  var b0=document.createElement('button');b0.textContent='\u25C0';b0.className='btnprev';\n"
	"  var b1=document.createElement('button');b1.textContent='\u25B6';b1.className='btnnext';\n"
	"  var info=document.createElement('span');info.className='info';\n"
	"  var cv=document.createElement('span');cv.className='vchip curver';\n"
	"  pg.appendChild(b0);pg.appendChild(info);pg.appendChild(cv);pg.appendChild(b1);sv.appendChild(gh);sv.appendChild(pg);\n"
	"  var body=document.createElement('div');body.className='tree';\n"
	"  var tb=document.createElement('ul');tb.className='treebody';\n"
	"  body.appendChild(tb);\n"
	"  sv.appendChild(body);\n"
	"  var cur=0;\n"
	"  b0.onclick=function(){if(cur>0){cur--;setPage(tb,info,cv,b0,b1,list,cur);}};\n"
	"  b1.onclick=function(){if(cur<list.length-1){cur++;setPage(tb,info,cv,b0,b1,list,cur);}};\n"
	"  setPage(tb,info,cv,b0,b1,list,0);\n"
	"  app.appendChild(sv);\n"
	"});\n"
	"document.getElementById('sub').textContent=data.length+' top-level table(s) parsed';\n"
	"document.addEventListener('click',function(e){\n"
	"  var t=e.target.closest('.tw');if(!t||t.classList.contains('leaf'))return;\n"
	"  var li=t.closest('li.node');if(!li)return;\n"
	"  if(li.classList.contains('c')){li.classList.remove('c');t.textContent='-';}\n"
	"  else{li.classList.add('c');t.textContent='+';}\n"
	"});\n"
	"})();\n"
	"</script>\n"
	"</body>\n"
	"</html>\n";

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
				root_close = '}';
				res_init_root();
				jstack[0].close = '}';
				fputc('{', rops[outtype].f);
			}
			break;
		case RES_HTML:
			snprintf(outfile, LINE_LEN, "%s.html", filename);
			rops[outtype].f = fopen(outfile, "w");
			if (rops[outtype].f) {
				fputs(html_head, rops[outtype].f);
				root_close = ']';
				res_init_root();
				jstack[0].close = ']';
				fputc('[', rops[outtype].f);
			}
			break;
		case RES_YAML:
			snprintf(outfile, LINE_LEN, "%s.yaml", filename);
			rops[outtype].f = fopen(outfile, "w");
			if (rops[outtype].f) {
				res_init_root();
				jstack[0].close = '}';
				yfirst = 1;
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

	if (is_hier()) {
		/* one single string value, hex bytes + printable ASCII; newlines
		 * are escaped later by the shared emitter (JSON & YAML syntax). */
		char raw[LINE_LEN * 4] = {0};
		int out = 0;
		ofs = 0;
		while (ofs < len) {
			int n;
			if (ofs > 0 && out < (int)sizeof(raw))
				raw[out++] = '\n';
			n = snprintf(raw + out, sizeof(raw) - out, "%08x:", ofs);
			if (n < 0 || n >= (int)(sizeof(raw) - out))
				break;
			out += n;
			for (i = 0; i < 16 && ofs + i < len; i++) {
				n = snprintf(raw + out, sizeof(raw) - out, " %02x", data[ofs + i]);
				if (n < 0 || n >= (int)(sizeof(raw) - out))
					goto done;
				out += n;
			}
			for (; i < 16; i++) {
				n = snprintf(raw + out, sizeof(raw) - out, "   ");
				if (n < 0 || n >= (int)(sizeof(raw) - out))
					goto done;
				out += n;
			}
			n = snprintf(raw + out, sizeof(raw) - out, "  | ");
			if (n < 0 || n >= (int)(sizeof(raw) - out))
				goto done;
			out += n;
			for (i = 0; i < 16 && ofs < len; i++, ofs++) {
				unsigned char c = data[ofs];
				raw[out++] = (c >= 0x20 && c < 0x7f) ? (char)c : '.';
			}
		}
done:
		raw[out] = '\0';
		if (outtype == RES_YAML)
			y_emit(curf(), lv, title, raw);
		else
			j_emit(curf(), lv, title, raw);
		return;
	}

	/* non-hierarchical output keeps the classic hexdump layout */
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

	if (is_hier()) {
		FILE *f = curf();

		/* res_close() signal: lv==0, key==NULL, fmt==NULL -> close root */
		if (lv == 0 && key == NULL && fmt == NULL) {
			hier_close(f);
			return 0;
		}

		char valbuf[LINE_LEN] = {0};
		if (fmt) {
			va_start(args, fmt);
			vsnprintf(valbuf, LINE_LEN, fmt, args);
			va_end(args);
		}
		if (outtype == RES_YAML)
			y_emit(f, lv, key, fmt ? valbuf : NULL);
		else
			j_emit(f, lv, key, fmt ? valbuf : NULL);
		return 0;
	}

	/* ---- non-hierarchical output (text / plain) ---- */
	if (fmt)
		va_start(args, fmt);
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
	else if (outtype == RES_YAML)
		hier_close(curf());
	if (outtype == RES_HTML) {
		rout(0, NULL, NULL);
		fputs(html_foot, rops[outtype].f);
	}
	if (outtype != RES_STD)
		fclose(rops[outtype].f);
	return 0;
}