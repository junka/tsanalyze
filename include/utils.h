#ifndef _UTILS_H_
#define _UTILS_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX(a, b) (((a) > (b))? (a) : (b))
#define MIN(a, b) (((a) > (b))? (b) : (a))

char *convert_UTC(UTC_time_t *t);

int bitmap64_full(uint64_t *bitmap, uint64_t last);

#ifdef __cplusplus
}
#endif

#endif
