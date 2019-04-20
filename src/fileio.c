#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>

#include "ts.h"

static int fileio_open(char * filename);
static int fileio_read(void **ptr,size_t *len);
static int fileio_close();


struct io_ops file_ops={
	.open = fileio_open,
	.close = fileio_close,
	.read = fileio_read,
};

int fileio_open(char * filename)
{
	if(filename == NULL)
		return -1;
	file_ops.fd = open(filename, O_RDONLY);
	if(file_ops.fd <0)
		return -1;

	file_ops.ptr = NULL;
	file_ops.block_size = 2048*1024;
	file_ops.offset = 0;
	return 0;
}

int fileio_read(void **ptr,size_t *len)
{
	if ((file_ops.ptr = (unsigned char *)mmap(NULL, file_ops.block_size, PROT_READ,
			MAP_SHARED, file_ops.fd, file_ops.offset)) == (void *)-1) {
		return -1;
	}
	file_ops.offset += file_ops.block_size;
	*ptr = file_ops.ptr;
	*len = file_ops.block_size;
	return 0;
}

int fileio_close()
{
	if(file_ops.fd>=0)
		close(file_ops.fd);
	munmap(file_ops.ptr,file_ops.block_size);
	
	file_ops.ptr = NULL;
	file_ops.offset =0;
	file_ops.block_size = 0;
	return 0;
}

