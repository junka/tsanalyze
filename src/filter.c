#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "list.h"
#include "utils.h"
#include "filter.h"
#include "ts.h"

#define MAX_FILTER_NUM (6)

struct filter_slot
{
	filter_t *t;
	struct list_node n;
} filter_slot;

struct filter_head
{
	int filter_num;
	struct list_head h;
};

static struct filter_head pid_filter[MAX_TS_PID_NUM];

int filter_init(void)
{
	int i = 0;
	for (i = 0; i < MAX_TS_PID_NUM; i++) {
		pid_filter[i].filter_num = 0;
		list_head_init(&pid_filter[i].h);
	}
	return 0;
}

filter_t *filter_alloc(uint16_t pid)
{
	if (unlikely(pid_filter[pid].filter_num >= MAX_FILTER_NUM))
		return NULL;
	filter_t *f = malloc(sizeof(filter_t));
	memset(f, 0, sizeof(filter_t));
	f->pid = pid;
	struct filter_slot *fs = malloc(sizeof(struct filter_slot));
	memset(fs, 0, sizeof(struct filter_slot));
	fs->t = f;
	list_add(&pid_filter[pid].h, &(fs->n));
	pid_filter[pid].filter_num++;
	return f;
}

int filter_set(filter_t *f, filter_param_t *p, filter_cb func)
{
	if (unlikely(f == NULL))
		return -1;
	if (likely(p != NULL)) {
		f->para.depth = p->depth;
		memcpy(f->para.coff, p->coff, p->depth * sizeof(uint8_t));
		memcpy(f->para.mask, p->mask, p->depth * sizeof(uint8_t));
		memcpy(f->para.negete, p->negete, p->depth * sizeof(uint8_t));
	}
	f->callback = func;
	return 0;
}

int filter_free(filter_t *f)
{
	if (f == NULL)
		return -1;
	struct list_head *lh = &pid_filter[f->pid].h;
	struct filter_slot *ix, *next;
	list_for_each_safe(lh, ix, next, n)
	{
		if (ix->t == f) {
			list_del(&ix->n);
			pid_filter[f->pid].filter_num--;
			free(ix);
			free(f);
			break;
		}
	}
	return 0;
}

filter_t *filter_lookup(uint16_t pid, filter_param_t *para)
{
	filter_t *f = NULL;
	int i = 0;
	if (unlikely(para == NULL))
		return NULL;
	if (unlikely(pid_filter[pid].filter_num == 0))
		return NULL;
	struct list_head *lh = &pid_filter[pid].h;
	struct filter_slot *ix = NULL;
	if (unlikely(list_empty(lh)))
		return NULL;
	list_for_each(lh, ix, n)
	{
		if (ix->t->para.depth == para->depth) {
			if (0 == memcmp(ix->t->para.coff, para->coff, para->depth * sizeof(uint8_t)) &&
				0 == memcmp(ix->t->para.mask, para->mask, para->depth * sizeof(uint8_t)) &&
				0 == memcmp(ix->t->para.negete, para->negete, para->depth * sizeof(uint8_t))) {
				f = ix->t;
				break;
			}
		}
	}
	return f;
}

int filter_proc(uint16_t pid, uint8_t *data, uint16_t len)
{
	struct list_head *lh = &pid_filter[pid].h;
	struct filter_slot *ix, *next;
	if (unlikely(list_empty(lh)))
		return -1;
	list_for_each_safe(lh, ix, next, n)
	{
		if ((data[0] & ix->t->para.mask[0]) == (ix->t->para.mask[0] & ix->t->para.coff[0]))
			ix->t->callback(pid, data, len);
	}
	return 0;
}

void filter_dump(void)
{
	int i = 0;
	struct list_head *lh;
	struct filter_slot *ix;
	for (i = 0; i < MAX_TS_PID_NUM; i++) {
		lh = &pid_filter[i].h;
		if (unlikely(list_empty(lh)))
			continue;
		printf("PID %0x4d(0x%04x):", i, i);
		list_for_each(lh, ix, n)
		{
			printf(" 0x%x ", ix->t->para.coff[0]);
		}
		printf("\n");
	}
}
