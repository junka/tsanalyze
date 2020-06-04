#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ts.h"
#include "utils.h"
#include "descriptor.h"

struct descriptor_ops des_ops[256];

int parse_reserved_descriptor(uint8_t *buf, uint32_t len, void *ptr) { return 0; }

void *alloc_reserved(void)
{
	void *rsv_m = malloc(sizeof(descriptor_t));
	return rsv_m;
}

void free_reserved(descriptor_t *ptr)
{
	if (ptr != NULL)
		free(ptr);
}

void dump_reserved(const char *str, descriptor_t *p_descriptor)
{
	int i = 0;
	printf("\"");
	for (i = 0; i < p_descriptor->length; i++)
		printf("%s%c", str, p_descriptor->data[i]);
	printf("\"\n");
}

void init_descriptor_parsers(void)
{
	uint16_t i = 0;
	for (i = 0; i <= 0xFF; i++) {
		des_ops[i].tag = i;
		snprintf(des_ops[i].tag_name, MAX_TAG_NAME, "reserved");
		des_ops[i].descriptor_parse = parse_reserved_descriptor;
		des_ops[i].descriptor_alloc = alloc_reserved;
		des_ops[i].descriptor_free = free_reserved;
		des_ops[i].descriptor_dump = dump_reserved;
	}

#define _(a, b)                                                                                                        \
	des_ops[b].tag = b;                                                                                                \
	des_ops[b].descriptor_parse = parse_##a##_descriptor;                                                              \
	des_ops[b].descriptor_alloc = alloc_##a##_descriptor;                                                              \
	des_ops[b].descriptor_free = free_##a##_descriptor;                                                                \
	des_ops[b].descriptor_dump = dump_##a##_descriptor;                                                                \
	snprintf(des_ops[b].tag_name, MAX_TAG_NAME, #a "_descriptor");
	foreach_enum_descriptor
#undef _
}

void parse_descriptors(struct list_head *h, uint8_t *buf, int len)
{
	int l = len;
	uint8_t *ptr = buf;
	descriptor_t *more = NULL;
	void *des = NULL;
	while (l > 0) {
		// hexdump( ptr, l);
		// printf("%s : %d, %d\n",des_ops[ptr[0]].tag_name,l,ptr[1]);
		des = des_ops[ptr[0]].descriptor_alloc();
		more = (descriptor_t *)des;
		des_ops[ptr[0]].descriptor_parse(ptr, ptr[1] + 2, des);
		more->tag = ptr[0];
		more->length = ptr[1];
		l -= more->length + 2;
		ptr += more->length + 2;
		list_add(h, &more->n);
	}
}

void free_descriptors(struct list_head *list)
{
	descriptor_t *t = NULL, *next = NULL;
	list_for_each_safe(list, t, next, n)
	{
		list_del(&(t->n));
		des_ops[t->tag].descriptor_free(t);
	}
}

void dump_descriptors(const char *str, struct list_head *list)
{
	descriptor_t *p = NULL, *next = NULL;
	list_for_each_safe(list, p, next, n)
	{
		//printf("%s 0x%02x (%s) : len %d\n", str, p->tag, des_ops[p->tag].tag_name, p->length);
		//printf("%s ", str);
		des_ops[p->tag].descriptor_dump(str, p);
	}
}
