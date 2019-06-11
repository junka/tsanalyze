#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "list.h"
#include "utils.h"
#include "filter.h"

typedef struct filter_slot{
	filter_t * flt;
	struct list_head h;
};

filter_t * filter_alloc(uint16_t pid)
{
	filter_t * f;
	return f;
}

int filter_set(filter_t *f, filter_param_t p,filter_cb func)
{
	return 0;
}

int filter_free(filter_t *f)
{
	return 0;
}


