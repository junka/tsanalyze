#ifndef _UTILS_H_
#define _UTILS_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "types.h"

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define list_insert(A, member, type, m, elem) do{ \
	if(A->member == NULL) \
	{ \
		A->member = elem; \
		break;\
	} \
	type * h = A->member; \
	if(elem->m < h->m) { \
		elem->next = h; \
		h->prev = elem; \
		A->member = elem; \
		break; \
	} \
	while(h->next && h->m < elem->m) \
	{ \
		h = h->next; \
	} \
	if(h->m < elem->m)\
	{\
		elem->prev = h;\
		elem->next = h->next;\
		h->next = elem;\
	}\
	else if(h->m > elem->m)\
	{\
		elem->next = h; \
		elem->prev = h->prev;\
		h->prev->next = elem;\
		h->prev = elem; \
	}\
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

#define stringify(x) #x

#define offsetof(type, member) ((size_t)&(((type*)0)->member))

#ifndef container_off
#define container_off(containing_type, member)	\
			offsetof(containing_type, member)
#endif

#ifndef container_of
#define container_of(item, type, member) \
		((type *)((char *)item - (char *)(&((type *)0)->member)))
#endif

#if HAVE_TYPEOF
#define container_of_var(member_ptr, container_var, member) \
	container_of(member_ptr, typeof(*container_var), member)
#else
#define container_of_var(member_ptr, container_var, member)	\
	((void *)((char *)(member_ptr)	-			\
		  container_off_var(container_var, member)))
#endif
#if HAVE_TYPEOF
#define container_off_var(var, member)		\
	container_off(typeof(*var), member)
#else
#define container_off_var(var, member)			\
	((const char *)&(var)->member - (const char *)(var))
#endif

char* convert_UTC(UTC_time_t *t);

#ifdef __cplusplus
}
#endif

#endif
