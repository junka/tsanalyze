#include <stdint.h>
#include <stdio.h>

#include "io.h"

#define MAX_IO_METHOD (10)

static struct io_ops *ioops[MAX_IO_METHOD];

int register_io_ops(struct io_ops *ops)
{
	ioops[ops->type] = ops;
	return 0;
}

int unregister_io_ops(struct io_ops *ops)
{
	ioops[ops->type] = NULL;
	return 0;
}

struct io_ops *lookup_io_ops(int type) { return ioops[type]; }
