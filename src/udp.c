#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "io.h"
#include "utils.h"

static struct io_ops udp_ops;

int udp_open(char * urlpath)
{
	int ret;
	struct sockaddr_in addr;
	addr.sin_family =AF_INET;
	addr.sin_port =htons(9900);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("127.0.0.1");
	int len = sizeof(struct sockaddr);

	udp_ops.fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(udp_ops.fd < 0)
	{
		return -1;
	}
	udp_ops.block_size = 1024*2;
	if (udp_ops.fd != -1) {
		if (fcntl(udp_ops.fd, F_SETFD, FD_CLOEXEC) == -1)
		{
		}
	}
#ifdef SO_NOSIGPIPE
		if (udp_ops.fd != -1)
			setsockopt(udp_ops.fd, SOL_SOCKET, SO_NOSIGPIPE, &(int){1}, sizeof(int));
#endif
	ret = bind(udp_ops.fd,(struct sockaddr *)&addr, len);
	if(ret < 0)
	{
	}
	getsockname(udp_ops.fd, (struct sockaddr *)&addr, &len);
	printf("%s \n",inet_ntoa(addr.sin_addr));


}

int udp_read(void **ptr,size_t *len)
{
	static unsigned char buf[2048];
	int size = 2048;
	struct sockaddr_in peer;
	int ret;
	ret = recv(udp_ops.fd, buf, size, 0);
	*ptr = buf;
	*len = ret;
	return 0;
}

int udp_close(void)
{
	if(udp_ops.fd>=0)
		close(udp_ops.fd);
}

static int udp_end(void)
{
	/*how to define the end of a stream*/
	return 1;
}

static struct io_ops udp_ops={
	.type = IO_UDP,
	.open = udp_open,
	.read = udp_read,
	.close = udp_close,
	.end = udp_end,
};


REGISTER_IO_OPS(udp,&udp_ops);

