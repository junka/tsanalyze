#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<arpa/inet.h>
#include <sys/mman.h>
#include <poll.h>
#include <linux/if_packet.h>

#include "io.h"

static struct io_ops udp_ops;

int udp_open(char * urlpath)
{
	int ret;
	struct sockaddr_in addr,dest;
	addr.sin_family =AF_INET;
	addr.sin_port =htons(9900);
	addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	dest.sin_family =AF_INET;
	dest.sin_port =htons(9901);
	dest.sin_addr.s_addr=inet_addr("127.0.0.1");
	int len = sizeof(struct sockaddr);

	udp_ops.fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(udp_ops.fd < 0)
	{
		return -1;
	}
	udp_ops.block_size = 1024*1024*2;
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
	//ret = setsockopt(udp_ops.fd, SOL_SOCKET, SO_RCVBUF, &tmp, sizeof(tmp));
	//fcntl(udp_ops.fd, F_SETFL, fcntl(udp_ops.fd, F_GETFL) | O_NONBLOCK);

	ret = connect(udp_ops.fd, (struct sockaddr *) &dest, sizeof(dest));

#if 0
	struct tpacket_req req;
	
	req.tp_block_size = 4096;
	req.tp_block_nr = udp_ops.block_size/req.tp_block_size;
	req.tp_frame_size = 2048;
	req.tp_frame_nr = udp_ops.block_size/req.tp_frame_size;

	ret = setsockopt(udp_ops.fd, SOL_PACKET, PACKET_RX_RING, (void *)&req, sizeof(req));
	if(ret<0)
	{
		return -1;
	}
	udp_ops.ptr = (char *)mmap(0, udp_ops.block_size, PROT_READ, MAP_SHARED, udp_ops.fd, 0);
	if(udp_ops.ptr == MAP_FAILED)
	{
		return -1;
	}
	struct pollfd pfd;
	pfd.fd = udp_ops.fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	ret = poll(&pfd, 1, -1);
	if(ret<0)
	{
		return -1;
	}
#endif
}

int udp_read(void **ptr,size_t *len)
{
	//int i, nIndex = 0;
	static unsigned char buf[2048];
	int size = 2048;
	//int ev = POLLOUT ;
	//struct pollfd p = { .fd = udp_ops.fd, .events = ev, .revents = 0 };
	int ret;
	//ret = poll(&p, 1, 100);
	ret = recv(udp_ops.fd, buf, size, 0);
	printf("ret %d\n",ret);
#if 0
	struct tpacket_hdr* pHead;// = (struct tpacket_hdr*)(udp_ops.ptr+ nIndex*2048);
	for(i=0; i<udp_ops.block_size/4096; i++)
	{
		pHead = (struct tpacket_hdr*)(udp_ops.ptr+ nIndex*2048);
		pHead->tp_len = 0;
		pHead->tp_status = TP_STATUS_KERNEL;
		nIndex++;
		nIndex%=udp_ops.block_size/2048;
	}
#endif
	*ptr = NULL;
	*len = 0;
	return 0;
}

int udp_close(void)
{
	if(udp_ops.fd>=0)
		close(udp_ops.fd);
	munmap(udp_ops.ptr, udp_ops.block_size);
}

static int udp_end(void)
{
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

