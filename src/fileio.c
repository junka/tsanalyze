#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>

struct io_ops{
	int fd;
	int size;
	char *ptr;
	int (*open)(char *filename);
};

static struct io_ops file_ops;

int fileio_open(char * filename)
{
	if(filename == NULL)
		return -1;
	file_ops.fd = open(filename, O_RDONLY|O_BINARY);
	if(file_ops.fd <0)
		return -1;
	
	return 0;
}

int fileio_read()
{
	if ((file_ops.ptr = (char *)mmap(NULL, file_ops.size, PROT_READ,
			MAP_SHARED, file_ops.fd, 0)) == (void *)-1) {
		return -1;
	}

	return 0;
}

int fileio_close()
{
	if(file_ops.fd>=0)
		close(file_ops.fd);
	munmap(file_ops.ptr,file_ops.size);
	return 0;
}

