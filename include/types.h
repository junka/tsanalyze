#ifndef _TYPES_H_
#define _TYPES_H_

#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

/** C extension macro for environments lacking C11 features. */
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#define EXT_STD_C11 __extension__
#else
#define EXT_STD_C11
#endif

// #ifndef uint24_t
// typedef struct uint24 {
// 	uint32_t bits : 24;
// } __attribute__((packed)) uint24_t;
// #endif

typedef uint8_t uint24_t[3];
typedef uint8_t uint40_t[5];
typedef uint8_t uint48_t[6];
// typedef struct uint40 {
// 	uint64_t bits : 40;
// } __attribute__((packed)) uint40_t;

typedef struct
{
	uint8_t time[5];
} __attribute__((packed)) UTC_time_t;

typedef struct descriptor
{
	uint8_t tag;
	uint8_t length;
	struct list_node n;
	uint8_t data[0];
} descriptor_t;

#ifdef __cplusplus
}
#endif

#endif /* _TYPES_H_ */
