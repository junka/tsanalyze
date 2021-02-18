#ifndef _UTILS_H_
#define _UTILS_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

char *convert_UTC(UTC_time_t *t);

int bitmap64_full(uint64_t *bitmap, uint64_t last);

#ifdef __cplusplus
}
#endif

#endif
