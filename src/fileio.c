#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>

struct io_ops{
	int fd;
	int block_size;
	unsigned char *ptr;
	int (*open)(char *filename);
	int (*read)();
	int (*close)();
};

static struct io_ops file_ops;

int fileio_open(char * filename)
{
	if(filename == NULL)
		return -1;
	file_ops.fd = open(filename, O_RDONLY);
	if(file_ops.fd <0)
		return -1;
	
	return 0;
}

int fileio_read()
{
	if ((file_ops.ptr = (unsigned char *)mmap(NULL, file_ops.block_size, PROT_READ,
			MAP_SHARED, file_ops.fd, 0)) == (void *)-1) {
		return -1;
	}
	
	return 0;
}

int fileio_close()
{
	if(file_ops.fd>=0)
		close(file_ops.fd);
	munmap(file_ops.ptr,file_ops.block_size);
	return 0;
}

