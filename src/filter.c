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
	f->pid = pid;
	struct filter_slot *fs = malloc(sizeof(struct filter_slot));
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
		memcpy(&f->para, p, sizeof(filter_param_t));
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

filter_t *filter_lookup(uint16_t pid, filter_param_t *param)
{
	filter_t *f = NULL;
	if (unlikely(pid_filter[pid].filter_num == 0))
		return NULL;
	struct list_head *lh = &pid_filter[pid].h;
	struct filter_slot *ix;
	if (unlikely(list_empty(lh)))
		return NULL;
	list_for_each(lh, ix, n)
	{
		if (0 == memcmp(&(ix->t->para), param, sizeof(filter_param_t))) {
			f = ix->t;
			break;
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
