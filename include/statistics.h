#ifndef _STATISTICS_H_
#define _STATISTICS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct stats
{
	uint64_t pat_sections;
	uint64_t cat_sections;
	uint64_t pmt_sections;
	uint64_t tsdt_sections;
	uint64_t nit_actual_sections;
	uint64_t nit_other_sections;
	uint64_t sdt_actual_sections;
	uint64_t sdt_other_sections;
	uint64_t eit_actual_sections;
	uint64_t eit_other_sections;
	uint64_t bat_sections;
	uint64_t tdt_sections;
	uint64_t tot_sections;

	// private sections for CA system
	uint64_t ecm_sections;
	uint64_t emm_sections;
} stats_t;

#ifdef __cplusplus
}
#endif

#endif
