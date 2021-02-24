#ifndef _IO_H_
#define _IO_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct io_ops
{
	int type;
	int fd;
	int block_size;
	size_t total_size;
	size_t offset;
	unsigned char *ptr;
	int (*open)(const char *filename);
	int (*read)(void **ptr, size_t *len);
	int (*close)(void);
	int (*end)(void);
};

typedef enum {
	IO_FILE = 0,
	IO_UDP = 1,
} io_enum;

struct io_ops *lookup_io_ops(int type);

int register_io_ops(struct io_ops *ops);

int unregister_io_ops(struct io_ops *ops);

#define REGISTER_IO_OPS(nm, x)                                                                                         \
	static void __attribute__((constructor)) register_io_ops_##nm(void)                                                \
	{                                                                                                                  \
		register_io_ops(x);                                                                                            \
	}

#ifdef __cplusplus
}
#endif

#endif
