#ifndef _FILTER_H_
#define _FILTER_H_

#ifdef __cplusplus
extern "C" {
#endif

enum filter_type {
	FILTER_SECTION = 0,
	FILTER_PES = 1,
};

#define MAX_FILTER_DEPTH 8

typedef struct filter_param {
	uint8_t depth;
	uint8_t coff[MAX_FILTER_DEPTH];
	uint8_t mask[MAX_FILTER_DEPTH];
	uint8_t negete[MAX_FILTER_DEPTH];
} filter_param_t;

typedef int (*filter_cb)(uint16_t pid, uint8_t *data, uint16_t len);

typedef struct filter {
	uint16_t pid;
	filter_param_t para;
	filter_cb callback;
} filter_t;

int filter_init(void);

filter_t *filter_alloc(uint16_t pid);

int filter_set(filter_t *f, filter_param_t *p, filter_cb func);

int filter_free(filter_t *f);

int filter_proc(uint16_t pid, uint8_t *data, uint16_t len);

filter_t *filter_lookup(uint16_t pid, filter_param_t *param);

#ifdef __cplusplus
}
#endif

#endif /*_FILTER_H_*/
