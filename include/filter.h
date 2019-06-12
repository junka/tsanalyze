#ifndef _FILTER_H_
#define _FILTER_H_

#ifdef __cplusplus
extern "C"{
#endif

#define MAX_FILTER_DEPTH 8

typedef struct filter_param
{
	uint8_t depth;
	uint8_t coff[MAX_FILTER_DEPTH];
	uint8_t mask[MAX_FILTER_DEPTH];
	uint8_t negete[MAX_FILTER_DEPTH];
}filter_param_t;

typedef int (*filter_cb)(uint16_t pid,uint8_t *data,uint16_t len);

typedef struct filter
{
	uint16_t pid;
	filter_param_t para;
	filter_cb callback;
}filter_t;

int filter_init(void);

filter_t * filter_alloc(uint16_t pid);

int filter_set(filter_t *f, filter_param_t *p,filter_cb func);

int filter_free(filter_t *f);

int filter_proc(uint16_t pid,uint8_t *data,uint16_t len);


#define REGISTER_TABLE_FILTER(pid,tableid,msk,func) \
	void __attribute__((constructor(102))) init_table_filter_##tableid() { \
		filter_t *f = filter_alloc(pid); \
		filter_param_t para; \
		para.depth = 1;\
		para.coff[0] = tableid;\
		para.mask[0] = msk;\
		filter_set(f,&para, func);\
	}


#ifdef __cplusplus
}
#endif

#endif /*_FILTER_H_*/
