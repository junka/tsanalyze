#ifndef _RESULT_H_
#define _RESULT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif

enum {
    RES_STD = 0,
    RES_TXT,
    RES_JSON,
    RES_YAML,
    RES_NUM,
};

struct res_ops {
    const char *filename;
    FILE *f;
    uint8_t t;
    const char *additional;
};

int res_settype(int t);
int res_open(const char *filename);
int res_put(int lv, const char * key, const char *fmt, ...);
int res_close(void);


void res_hexdump(int lv, char * title, uint8_t *buf, uint32_t len);

#define rout(l, key, ...) res_put(l, key, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /*_RESULT_H_*/