#include <stdio.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <poll.h>
#include <linux/if_packet.h>

#include "io.h"

static struct io_ops udp_ops;

int udp_open(char * urlpath)
{
	udp_ops.fd = socket(PF_PACKET, SOCK_DGRAM, 0);
	if(udp_ops.fd < 0)
	{
		return -1;
	}
	udp_ops.block_size = 1024*1024*2;
	int ret;
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
	
}

int udp_read(void **ptr,size_t *len)
{
	int i, nIndex = 0;
	struct tpacket_hdr* pHead;// = (struct tpacket_hdr*)(udp_ops.ptr+ nIndex*2048);
	for(i=0; i<udp_ops.block_size/4096; i++)
	{
		pHead = (struct tpacket_hdr*)(udp_ops.ptr+ nIndex*2048);
		pHead->tp_len = 0;
		pHead->tp_status = TP_STATUS_KERNEL;
		nIndex++;
		nIndex%=udp_ops.block_size/2048;
	}
	*ptr = udp_ops.ptr;
	*len = udp_ops.block_size;
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

