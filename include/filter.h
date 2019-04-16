#ifndef _FILTER_H_
#define _FILTER_H_

#ifdef __cplusplus
extern "C"{
#endif

#define MAX_FILTER_DEPTH 8

typedef struct filter
{
	uint16_t pid;
	uint8_t depth;
	uint8_t coff[MAX_FILTER_DEPTH];
	uint8_t mask[MAX_FILTER_DEPTH];
	uint8_t negete[MAX_FILTER_DEPTH];
	int (*callback)(uint16_t pid,uint8_t *data,uint16_t len);
}filter_t;



#ifdef __cplusplus
}
#endif

#endif /*_FILTER_H_*/