#ifndef _STATISTICS_H_
#define _STATISTICS_H_

#ifdef __cplusplus
extern "C"{
#endif

struct stats{
	uint64_t i_bits;  /* ingress */
    uint64_t e_bits;  /* error bits */
    uint64_t o_bits;  /* egress */
    uint64_t null_bits;  /* null */
};


#ifdef __cplusplus
}
#endif

#endif

