#ifndef _TSIO_H_
#define _TSIO_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct io_ops
{
	int type;
#ifndef _MSC_VER
	int fd;
#else
	long long fd;
#endif
	int block_size;
	size_t total_size;
	size_t offset;
	unsigned char *ptr;
	int (* open)(const char *filename);
	int (* read)(void **ptr, size_t *len);
	int (* close)(void);
	int (* end)(void);
	int (* wip)(void);	/* tell us the process percentage */
};

typedef enum {
	IO_FILE = 0,
	IO_UDP = 1,
} io_enum;

struct io_ops *lookup_io_ops(int type);

int register_io_ops(struct io_ops *ops);

int unregister_io_ops(struct io_ops *ops);


void udp_io_init(void);
void fileio_init(void);

#ifdef __cplusplus
}
#endif

#endif
