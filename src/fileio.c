#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "io.h"
#include "ts.h"

static struct io_ops file_ops;

static int fileio_open(char * filename)
{
	if(filename == NULL)
		return -1;
	file_ops.fd = open(filename, O_RDONLY);
	if(file_ops.fd <0)
		return -1;
	file_ops.total_size = lseek(file_ops.fd, 0, SEEK_END);
	file_ops.ptr = NULL;
	file_ops.block_size = 2048*1024;
	file_ops.offset = 0;
	return 0;
}

static int fileio_read(void **ptr,size_t *len)
{
	size_t size = file_ops.block_size;
	if(file_ops.ptr != NULL)
	{
		if(munmap(file_ops.ptr, size) < 0)
		{
			*ptr = NULL;
			*len = 0;
			return -1;
		}
	}
	file_ops.ptr = NULL;
	if ((file_ops.ptr = (unsigned char *)mmap(NULL, size, PROT_READ,
			MAP_SHARED, file_ops.fd, file_ops.offset)) == (void *)-1) {
		*ptr = NULL;
		*len = 0;
		return -1;
	}
	if(file_ops.total_size -file_ops.offset < size)
		size = file_ops.total_size -file_ops.offset;
	file_ops.offset += size;
	*ptr = file_ops.ptr;
	*len = size;
	return 0;
}

static int fileio_close(void)
{
	if(file_ops.fd>=0)
		close(file_ops.fd);
	munmap(file_ops.ptr,file_ops.block_size);
	
	file_ops.ptr = NULL;
	file_ops.offset =0;
	file_ops.block_size = 0;
	return 0;
}

static int fileio_end(void)
{
	return (file_ops.total_size - file_ops.offset);
}

static struct io_ops file_ops={
	.type = IO_FILE,
	.open = fileio_open,
	.close = fileio_close,
	.read = fileio_read,
	.end = fileio_end,
};

REGISTER_IO_OPS(file,&file_ops);

