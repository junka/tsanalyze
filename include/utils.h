#ifndef _UTILS_H_
#define _UTILS_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX(a, b) (((a) > (b))? (a) : (b))
#define MIN(a, b) (((a) > (b))? (b) : (a))

void convert_UTC(UTC_time_t *t, char *str, int size);

int bitmap64_full(uint64_t *bitmap, uint64_t last);

int bitmap64_get(const uint64_t *bitmap, uint64_t bit);

void bitmap64_set(uint64_t *bitmap, uint64_t bit);

void bitmap64_clear(uint64_t *bitmap, uint64_t bit);

#ifdef __cplusplus
}
#endif

#endif
