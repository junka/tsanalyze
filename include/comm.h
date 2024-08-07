#ifndef _COMM_H_
#define _COMM_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _MSC_VER
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) !!(x)
#define unlikely(x) !!(x)
#endif

#define stringify(x) #x

#ifndef offsetof
#define offsetof(type, member) ((size_t) & (((type *)0)->member))
#endif

#ifndef container_off
#define container_off(containing_type, member) offsetof(containing_type, member)
#endif

#ifndef container_of
#define container_of(item, type, member) ((type *)((char *)item - (char *)(&((type *)0)->member)))
#endif

//For MSVC cl.exe see https://learn.microsoft.com/zh-cn/cpp/c-language/typeof-c?view=msvc-170
// see https://learn.microsoft.com/zh-cn/cpp/overview/compiler-versions?view=msvc-170

#if _MSC_VER >= 1939
#define typeof(x) __typeof__(x)
#endif

#if (defined HAVE_TYPEOF) || (_MSC_VER >= 1939)
#define container_of_var(member_ptr, container_var, member) container_of(member_ptr, typeof(*container_var), member)
#else
#define container_of_var(member_ptr, container_var, member)                                                            \
	((void *)((char *)(member_ptr) - container_off_var(container_var, member)))
#endif
#if (defined HAVE_TYPEOF) || (_MSC_VER >= 1939)
#define container_off_var(var, member) container_off(typeof(*var), member)
#else
#define container_off_var(var, member) ((const char *)&(var)->member - (const char *)(var))
#endif

#define BUILD_ASSERT_OR_ZERO(cond) (sizeof(char[1 - 2 * !(cond)]) - 1)

#ifdef HAVE_TYPEOF
#define check_type(expr, type) ((typeof(expr) *)0 != (type *)0)

#define check_types_match(expr1, expr2) ((typeof(expr1) *)0 != (typeof(expr2) *)0)
#else
/* Without 'typeof', we can only test the sizes. */
#define check_type(expr, type) BUILD_ASSERT_OR_ZERO(sizeof(expr) == sizeof(type))

#define check_types_match(expr1, expr2) BUILD_ASSERT_OR_ZERO(sizeof(expr1) == sizeof(expr2))
#endif /* HAVE_TYPEOF */

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#ifdef __cplusplus
}
#endif

#endif /*_COMM_H_*/