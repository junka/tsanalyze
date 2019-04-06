#ifndef _DESCRIPTOR_H_
#define _DESCRIPTOR_H_

#ifdef __cplusplus
extern "C"{
#endif

struct descriptor{
	uint8_t tag;
	uint8_t length;
	struct descriptor * next;
	uint8_t data[0];
};



#ifdef __cplusplus
}
#endif

#endif
