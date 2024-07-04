#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>
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

#ifdef _MSC_VER
#define PACK 
#else
#define PACK __attribute__((packed))
#endif

#ifdef __GNUC__
#define __maybe_unused __attribute__((unused))
#else
#define __maybe_unused
#endif

#ifndef uint24_t
typedef struct uint24 {
	uint8_t bits[3];
} PACK uint24_t;
#endif

typedef struct uint40 {
	uint64_t bits : 40;
} PACK uint40_t;

typedef struct uint48 {
	uint64_t bits : 48;
} PACK uint48_t;

typedef struct
{
	uint8_t time[5];
} PACK UTC_time_t;

typedef struct descriptor
{
	uint8_t tag;
	uint8_t length;
	struct list_node n;
#ifdef _MSC_VER
	uint8_t data[1];
#else
	uint8_t data[0];
#endif
} descriptor_t;

#ifdef __cplusplus
}
#endif

#endif /* _TYPES_H_ */
