#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "descriptor.h"
#include "ts.h"
#include "utils.h"

struct descriptor_ops des_ops[256];

int parse_reserved_descriptor(uint8_t *buf, uint32_t len, void *ptr)
{
	descriptor_t *desc = (descriptor_t *)ptr;
	desc->tag = buf[0];
	desc->length = buf[1];
	memcpy(desc->data, buf + 2, desc->length);
	return 0;
}

void *alloc_reserved(void)
{
	void *desc = calloc(1, sizeof(descriptor_t) + 255 - 2);
	return desc;
}

void free_reserved(descriptor_t *ptr)
{
	if (ptr != NULL)
		free(ptr);
}

#define BUF_LINE (512)
void dump_reserved(int lv, descriptor_t *p_descriptor)
{
	rout(lv, des_ops[p_descriptor->tag].tag_name, "descriptor 0x%02x len %d", p_descriptor->tag, p_descriptor->length);
	res_hexdump(lv+1, "unknown", p_descriptor->data, p_descriptor->length > 16 ? 16 : p_descriptor->length);
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
		// printf("%s(0x%x) : %d, %d\n", des_ops[ptr[0]].tag_name, ptr[0], l, ptr[1]);
		uint8_t tag = ptr[0];
		des = des_ops[tag].descriptor_alloc();
		more = (descriptor_t *)des;
		des_ops[tag].descriptor_parse(ptr, ptr[1] + 2, des);
		more->tag = tag;
		more->length = ptr[1];
		l -= more->length + 2;
		ptr += more->length + 2;
		list_add_tail(h, &(more->n));
	}
}

void free_descriptors(struct list_head *list)
{
	descriptor_t *t = NULL, *next = NULL;
	list_for_each_safe(list, t, next, n) {
		list_del(&(t->n));
		des_ops[t->tag].descriptor_free(t);
	}
}

void dump_descriptors(int lv, struct list_head *list)
{
	descriptor_t *p = NULL, *next = NULL;
	list_for_each_safe(list, p, next, n) {
		if (likely(des_ops[p->tag].descriptor_dump))
			des_ops[p->tag].descriptor_dump(lv, p);
	}
}

bool has_descritpor_tag(struct list_head *list, uint8_t tag)
{
	descriptor_t *p = NULL, *next = NULL;
	list_for_each_safe(list, p, next, n) {
		if (p->tag == tag) {
			return true;
		}
	}
	return false;
}