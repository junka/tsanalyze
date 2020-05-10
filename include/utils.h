#ifndef _UTILS_H_
#define _UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

void hexdump(uint8_t *buf, uint32_t len);

#define stringify(x) #x

#define offsetof(type, member) ((size_t) & (((type *)0)->member))

#ifndef container_off
#define container_off(containing_type, member) offsetof(containing_type, member)
#endif

#ifndef container_of
#define container_of(item, type, member) ((type *)((char *)item - (char *)(&((type *)0)->member)))
#endif

#if HAVE_TYPEOF
#define container_of_var(member_ptr, container_var, member) container_of(member_ptr, typeof(*container_var), member)
#else
#define container_of_var(member_ptr, container_var, member)                                                            \
	((void *)((char *)(member_ptr)-container_off_var(container_var, member)))
#endif
#if HAVE_TYPEOF
#define container_off_var(var, member) container_off(typeof(*var), member)
#else
#define container_off_var(var, member) ((const char *)&(var)->member - (const char *)(var))
#endif

#define BUILD_ASSERT_OR_ZERO(cond) (sizeof(char[1 - 2 * !(cond)]) - 1)

#if HAVE_TYPEOF
#define check_type(expr, type) ((typeof(expr) *)0 != (type *)0)

#define check_types_match(expr1, expr2) ((typeof(expr1) *)0 != (typeof(expr2) *)0)
#else
/* Without typeof, we can only test the sizes. */
#define check_type(expr, type) BUILD_ASSERT_OR_ZERO(sizeof(expr) == sizeof(type))

#define check_types_match(expr1, expr2) BUILD_ASSERT_OR_ZERO(sizeof(expr1) == sizeof(expr2))
#endif /* HAVE_TYPEOF */

char *convert_UTC(UTC_time_t *t);

#ifdef __cplusplus
}
#endif

#endif
