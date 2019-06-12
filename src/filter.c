#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "list.h"
#include "utils.h"
#include "filter.h"

typedef struct filter_slot{
	filter_t t;
	struct list_node n;
}filter_slot_t;

struct filter_head
{
	int filter_num;
	struct list_head h;
};

static struct filter_head pid_filter[8192];
static filter_slot_t filter_slots[8192][6];
//static filter_t filters[8192][6];
int __attribute__((constructor(101))) filter_init(void)
{
	int i =0 ;
	for(i=0;i<8192;i++)
	{
		pid_filter[i].filter_num = 0;
		list_head_init(&pid_filter[i].h);
	}
	return 0;
}


filter_t * filter_alloc(uint16_t pid)
{
	//= &filters[pid][pid_filter[pid].filter_num]; //malloc(sizeof(filter_t));
	if(pid_filter[pid].filter_num>=6)
		return NULL;
	struct filter_slot *fs = &filter_slots[pid][pid_filter[pid].filter_num]; //malloc(sizeof(struct filter_slot));
	filter_t * f = &fs->t;
	f->pid = pid;
	list_add(&pid_filter[pid].h,&(fs->n));
	pid_filter[pid].filter_num++;
	return f;
}

int filter_set(filter_t *f, filter_param_t *p,filter_cb func)
{
	if(p != NULL)
	{
		memcpy(&f->para,p,sizeof(filter_param_t));
	}
	f->callback = func;
	return 0;
}

int filter_free(filter_t *f)
{
	struct list_head *lh = &pid_filter[f->pid].h;
	struct filter_slot *ix ,*next;
	list_for_each_safe(lh, ix,next, n)
	{
		if(&ix->t ==f)
		{
			list_del(&ix->n);
			memset(ix,0,sizeof(struct filter_slot));
			pid_filter[f->pid].filter_num--;
			//free(ix);
			//free(f);
			break;
		}
	}
	return 0;
}

int filter_proc(uint16_t pid,uint8_t *data,uint16_t len)
{
	struct list_head *lh = &pid_filter[pid].h;
	struct filter_slot *ix,*next;
	if(list_empty(lh))
		return -1;
	list_for_each_safe(lh, ix,next, n)
	{
		if(data[0] & ix->t.para.mask[0] == ix->t.para.mask[0]& ix->t.para.coff[0])
			ix->t.callback(pid,data,len);
	}
	return 0;
}

void filter_dump()
{
	int i = 0;
	struct list_head *lh;
	struct filter_slot *ix,*next;
	for(i=0;i<8192;i++)
	{
		lh = &pid_filter[i].h;
		if(list_empty(lh))
			continue;
		printf("PID %0x4d(0x%04x):",i,i);
		list_for_each_safe(lh, ix,next, n)
		{
			printf(" 0x%x ",ix->t.para.coff[0]);
		}
		printf("\n");
	}

}
