#ifndef _UTILS_H_
#define _UTILS_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void hexdump(uint8_t *buf, uint32_t len);

char *convert_UTC(UTC_time_t *t);

#ifdef __cplusplus
}
#endif

#endif
