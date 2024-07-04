#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "tsio.h"
#include "ts.h"

#ifndef _MSC_VER
#include <unistd.h>
#include <sys/mman.h>
#else
#include <io.h>
#include <Windows.h>

#if defined(_WIN64)
typedef int64_t OffsetType;
#else
typedef uint32_t OffsetType;
#endif

#define PROT_NONE 0
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4

#define MAP_FILE 0
#define MAP_SHARED 1
#define MAP_PRIVATE 2
#define MAP_TYPE 0xf
#define MAP_FIXED 0x10
#define MAP_ANONYMOUS 0x20
#define MAP_ANON MAP_ANONYMOUS

#define MAP_FAILED ((void *)-1)

static int __map_mman_error(const unsigned long err, const int deferr)
{
	if (err == 0)
		return 0;
	// TODO: implement
	return err;
}

static unsigned long __map_mmap_prot_page(const int prot)
{
	unsigned long protect = 0;

	if (prot == PROT_NONE)
		return protect;

	if ((prot & PROT_EXEC) != 0) {
		protect = ((prot & PROT_WRITE) != 0) ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
	} else {
		protect = ((prot & PROT_WRITE) != 0) ? PAGE_READWRITE : PAGE_READONLY;
	}

	return protect;
}

static unsigned long __map_mmap_prot_file(const int prot)
{
	unsigned long desiredAccess = 0;

	if (prot == PROT_NONE)
		return desiredAccess;

	if ((prot & PROT_READ) != 0)
		desiredAccess |= FILE_MAP_READ;
	if ((prot & PROT_WRITE) != 0)
		desiredAccess |= FILE_MAP_WRITE;
	if ((prot & PROT_EXEC) != 0)
		desiredAccess |= FILE_MAP_EXECUTE;

	return desiredAccess;
}

static void *mmap(void *addr, size_t len, int prot, int flags, int fildes, OffsetType off)
{
	HANDLE fm, h;

	void *map = MAP_FAILED;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4293)
#endif

	const unsigned long dwFileOffsetLow = (sizeof(OffsetType) <= sizeof(unsigned long)) ? (unsigned long)off : (unsigned long)(off & 0xFFFFFFFFL);
	const unsigned long dwFileOffsetHigh =
		(sizeof(OffsetType) <= sizeof(unsigned long)) ? (unsigned long)0 : (unsigned long)((off >> 32) & 0xFFFFFFFFL);
	const unsigned long protect = __map_mmap_prot_page(prot);
	const unsigned long desiredAccess = __map_mmap_prot_file(prot);

	const OffsetType maxSize = off + (OffsetType)len;

	const unsigned long dwMaxSizeLow = (sizeof(OffsetType) <= sizeof(unsigned long)) ? (unsigned long)maxSize : (unsigned long)(maxSize & 0xFFFFFFFFL);
	const unsigned long dwMaxSizeHigh =
		(sizeof(OffsetType) <= sizeof(unsigned long)) ? (unsigned long)0 : (unsigned long)((maxSize >> 32) & 0xFFFFFFFFL);

#ifdef _MSC_VER
#pragma warning(pop)
#endif

	errno = 0;

	if (len == 0
		/* Usupported protection combinations */
		|| prot == PROT_EXEC) {
		errno = EINVAL;
		return MAP_FAILED;
	}

	h = ((flags & MAP_ANONYMOUS) == 0) ? (HANDLE)_get_osfhandle(fildes) : INVALID_HANDLE_VALUE;

	if ((flags & MAP_ANONYMOUS) == 0 && h == INVALID_HANDLE_VALUE) {
		errno = EBADF;
		return MAP_FAILED;
	}

	fm = CreateFileMapping(h, NULL, protect, dwMaxSizeHigh, dwMaxSizeLow, NULL);

	if (fm == NULL) {
		errno = __map_mman_error(GetLastError(), EPERM);
		return MAP_FAILED;
	}

	if ((flags & MAP_FIXED) == 0) {
		map = MapViewOfFile(fm, desiredAccess, dwFileOffsetHigh, dwFileOffsetLow, len);
	} else {
		map = MapViewOfFileEx(fm, desiredAccess, dwFileOffsetHigh, dwFileOffsetLow, len, addr);
	}

	CloseHandle(fm);

	if (map == NULL) {
		errno = __map_mman_error(GetLastError(), EPERM);
		return MAP_FAILED;
	}

	return map;
}

static int munmap(void *addr, size_t len)
{
	if (UnmapViewOfFile(addr))
		return 0;

	errno = __map_mman_error(GetLastError(), EPERM);

	return -1;
}

#define open(name, flags) _open(name, flags)
#define lseek(fd, off, start) _lseek(fd, off, start)
#define close(fd) _close(fd)

#endif


static struct io_ops file_ops;

static int fileio_open(const char *filename)
{
	if (filename == NULL)
		return -1;
	file_ops.fd = open(filename, O_RDONLY);
	if (file_ops.fd < 0)
		return -1;
	file_ops.total_size = lseek(file_ops.fd, 0, SEEK_END);
	file_ops.ptr = NULL;
	file_ops.block_size = 2048 * 1024;
	file_ops.offset = 0;
	return 0;
}

static int fileio_read(void **ptr, size_t *len)
{
	size_t size = file_ops.block_size;
	if (file_ops.ptr != NULL) {
		if (munmap(file_ops.ptr, size) < 0) {
			*ptr = NULL;
			*len = 0;
			return -1;
		}
	}
	file_ops.ptr = NULL;
	if ((file_ops.ptr = (unsigned char *)mmap(NULL, size, PROT_READ, 
				MAP_SHARED, file_ops.fd, file_ops.offset)) == (void *)-1)
	{
		*ptr = NULL;
		*len = 0;
		return -1;
	}
	if (file_ops.total_size - file_ops.offset < size)
		size = file_ops.total_size - file_ops.offset;
	file_ops.offset += size;
	*ptr = file_ops.ptr;
	*len = size;
	return 0;
}

static int fileio_close(void)
{
	if (file_ops.fd >= 0)
		close(file_ops.fd);
	munmap(file_ops.ptr, file_ops.block_size);

	file_ops.ptr = NULL;
	file_ops.offset = 0;
	file_ops.block_size = 0;
	return 0;
}

static int fileio_end(void)
{
	return (int)(file_ops.total_size - file_ops.offset);
}

static int fileio_tell(void)
{
	return (int)((file_ops.offset * 100)/file_ops.total_size);
}

static struct io_ops file_ops = {
	.type = IO_FILE,
	.open = fileio_open,
	.close = fileio_close,
	.read = fileio_read,
	.end = fileio_end,
	.wip = fileio_tell,
};


void fileio_init(void) {
	register_io_ops(&file_ops);
}
