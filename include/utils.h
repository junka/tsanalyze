#ifndef _UTILS_H_
#define _UTILS_H_

#ifdef __cplusplus
extern "C"{
#endif

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define list_insert(A, member, type, m, elem) do{ \
	if(A->member == NULL) \
	{ \
		A->member = elem; \
	} \
	type * h = A->member; \
	if(elem->m < h->m) { \
		elem->next = h; \
		h->next = elem; \
		A->member = elem; \
		break; \
	} \
	while(h->next && h->m < elem->m) \
	{ \
		h = h->next; \
	} \
	if(h->m ==elem->m)\
	{\
		return;\
	}\
	elem->prev = h;\
	elem->next = h->next;\
	h->next = elem;\
}while(0)


#define list_remove(A, member, type) do{ \
	type *p = A->member; \
	type *n ; \
	while(p) \
	{ \
		n = p->next; \
		free(p); \
		p = n; \
	} \
	A->member = NULL; \
}while(0)

#define list_modify(A, member, type, m, val, m1, val1) do{ \
	type* p = A->member; \
	while(p->m!= val) \
	{ \
		p = p->next ; \
	} \
	if(p->m1 != val1) \
		p->m1 = val1; \
} while(0)

void hexdump(uint8_t *buf, uint32_t len);


#ifdef __cplusplus
}
#endif

#endif
