#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filter.h"
#include "list.h"
#include "ts.h"
#include "utils.h"

/*
 * Per-PID filter dispatch.
 *
 * Every registered filter in this codebase matches on the FIRST byte of the
 * section, i.e. the table_id, with a depth-1 (tableid, mask) pair.  We exploit
 * that so that the common case (mask == 0xFF, one concrete table_id) is
 * dispatched in O(1) by indexing a per-PID map with `data[0]`.  The previous
 * implementation used a fixed-size linked list (MAX_FILTER_NUM = 6) which forced
 * the PSIP tables (9 of them sharing PID 0x1FFB) into a single wildcard filter.
 *
 * Filters whose mask is not 0xFF (pair masks like 0xFE, or wildcard 0x00) are
 * dispatched by a linear scan (marked `multi`).  Slot storage is a growable
 * array, so there is no fixed per-PID registry limit any more.
 */

#define NUM_FILTER_MAX 4096

struct filter_slot {
	filter_t t;
};

struct filter_head {
	int num;               /* number of registered filters on this PID */
	int cap;               /* allocated capacity of slots */
	struct filter_slot *slots;
	uint16_t map[256];     /* table_id -> slot index (0xFFFF = none)   */
	int multi;             /* 1 when linear dispatch is required        */
};

static struct filter_head pid_filter[MAX_TS_PID_NUM];

static void filter_rebuild(struct filter_head *hd)
{
	int i;
	for (i = 0; i < 256; i++)
		hd->map[i] = 0xFFFF;
	hd->multi = 0;
	for (i = 0; i < hd->num; i++) {
		filter_param_t *p = &hd->slots[i].t.para;
		if (p->depth == 1 && p->mask[0] == 0xFF) {
			uint8_t tid = p->coff[0];
			if (hd->map[tid] != 0xFFFF)
				hd->multi = 1;   /* duplicate table_id : fall back */
			else
				hd->map[tid] = (uint16_t)i;
		} else {
			hd->multi = 1;   /* pair / wildcard mask : linear scan */
		}
	}
}

int filter_init(void)
{
	int i = 0;
	for (i = 0; i < MAX_TS_PID_NUM; i++) {
		pid_filter[i].num = 0;
		pid_filter[i].cap = 0;
		pid_filter[i].slots = NULL;
	}
	return 0;
}

filter_t *filter_alloc(uint16_t pid)
{
	struct filter_head *hd = &pid_filter[pid];
	struct filter_slot *ns;
	if (hd->num >= NUM_FILTER_MAX)
		return NULL;
	if (hd->num >= hd->cap) {
		int ncap = hd->cap ? hd->cap * 2 : 4;
		ns = (struct filter_slot *)realloc(hd->slots, (size_t)ncap * sizeof(*ns));
		if (!ns)
			return NULL;
		hd->slots = ns;
		hd->cap = ncap;
	}
	ns = &hd->slots[hd->num];
	memset(ns, 0, sizeof(*ns));
	ns->t.pid = pid;
	hd->num++;
	return &ns->t;
}

int filter_set(filter_t *f, filter_param_t *p, filter_cb func)
{
	if (unlikely(f == NULL))
		return -1;
	if (likely(p != NULL)) {
		f->para.depth = p->depth;
		memcpy(f->para.coff, p->coff, p->depth * sizeof(uint8_t));
		memcpy(f->para.mask, p->mask, p->depth * sizeof(uint8_t));
		memcpy(f->para.negate, p->negate, p->depth * sizeof(uint8_t));
	}
	f->callback = func;
	filter_rebuild(&pid_filter[f->pid]);
	return 0;
}

int filter_free(filter_t *f)
{
	if (f == NULL)
		return -1;
	struct filter_head *hd = &pid_filter[f->pid];
	int i;
	for (i = 0; i < hd->num; i++) {
		if (&hd->slots[i].t == f) {
			/* swap-pop the freed slot, then rebuild the dispatch map */
			hd->slots[i] = hd->slots[hd->num - 1];
			hd->num--;
			filter_rebuild(hd);
			return 0;
		}
	}
	return -1;
}

filter_t *filter_lookup(uint16_t pid, filter_param_t *para)
{
	filter_t *f = NULL;
	if (unlikely(para == NULL))
		return NULL;
	struct filter_head *hd = &pid_filter[pid];
	if (unlikely(hd->num == 0))
		return NULL;
	int i;
	for (i = 0; i < hd->num; i++) {
		filter_t *tf = &hd->slots[i].t;
		if (tf->para.depth == para->depth) {
			if (0 == memcmp(tf->para.coff, para->coff, para->depth * sizeof(uint8_t)) &&
				0 == memcmp(tf->para.mask, para->mask, para->depth * sizeof(uint8_t)) &&
				0 == memcmp(tf->para.negate, para->negate, para->depth * sizeof(uint8_t)))
			{
				f = tf;
				break;
			}
		}
	}
	return f;
}

/* does a section/byte-stream match a filter's param ? */
static int filter_match(const filter_t *t, const uint8_t *data)
{
	const filter_param_t *p = &t->para;
	int i;
	if (p->depth == 0)
		return 1;
	for (i = 0; i < p->depth; i++) {
		if ((data[i] & p->mask[i]) != (p->mask[i] & p->coff[i]))
			return 0;
	}
	return 1;
}

int filter_proc(uint16_t pid, uint8_t *data, uint16_t len)
{
	struct filter_head *hd = &pid_filter[pid];
	int i;
	if (unlikely(hd->num == 0))
		return -1;

	if (hd->multi) {
		/* generic / paired-mask filters : linear scan */
		for (i = 0; i < hd->num; i++) {
			if (filter_match(&hd->slots[i].t, data))
				hd->slots[i].t.callback(pid, data, len);
		}
		return 0;
	}

	/* single 0xFF-mask filters : O(1) table_id dispatch */
	uint16_t idx = hd->map[data[0]];
	if (idx != 0xFFFF && idx < (uint16_t)hd->num)
		hd->slots[idx].t.callback(pid, data, len);
	return 0;
}

void filter_dump(void)
{
	int i = 0;
	struct filter_head *hd;
	printf("\nFilter available:\n");
	for (i = 0; i < MAX_TS_PID_NUM; i++) {
		hd = &pid_filter[i];
		if (unlikely(hd->num == 0))
			continue;
		printf("PID %04d(0x%04x):", i, i);
		for (int j = 0; j < hd->num; j++) {
			printf(" 0x%x ", hd->slots[j].t.para.coff[0]);
		}
		printf("\n");
	}
}