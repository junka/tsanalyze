#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __APPLE__
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "io.h"

static struct io_ops udp_ops;

struct url
{
	char proto[32];
	uint32_t addr;
	uint32_t port;
};

void parse_url(const char *url, const char *protocl, uint32_t *addr, uint32_t *port)
{
	if (url == NULL)
		return;
	char *url_dup = malloc(strlen(url) + 1);
	char *p_colon = NULL;
	char *start = 0;
	memcpy(url_dup, url, strlen(url) + 1);
	if (strncmp(url_dup, protocl, strlen(protocl)) == 0) {
		start = url_dup + strlen(protocl) + 3;
		p_colon = strchr(start, ':');
		if (p_colon != NULL) {
			*port = atoi(p_colon + 1);
			*p_colon = '\0';
		} else {
			*port = 9001;
		}
		*addr = inet_addr(start);
	}
	if (url_dup != NULL) {
		free(url_dup);
		url_dup = NULL;
	}
}

static struct url *parse_url_path(const char *urlpath)
{
	static struct url surl;
	if (strncmp(urlpath, "udp", 3) != 0) {
		return NULL;
	}
	parse_url(urlpath, "udp", &surl.addr, &surl.port);
	return &surl;
}

int udp_open(const char *urlpath)
{
	struct url *surl = parse_url_path(urlpath);
	int ip_shift = 24, i;
	uint8_t ip[4];
	char ipstr[16];
	for (i = 0; i < 4; i++) {
		ip[i] = (surl->addr >> ip_shift) & 0xFF;
		ip_shift -= 8;
	}
	snprintf(ipstr, 16, "%u.%u.%u.%u", ip[3], ip[2], ip[1], ip[0]);
	printf("ip addr %s, port %d \n", ipstr, surl->port);

	int ret;
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(surl->port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY); // inet_addr("127.0.0.1");
	unsigned int len = sizeof(struct sockaddr);

	udp_ops.fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_ops.fd < 0) {
		return -1;
	}
	udp_ops.block_size = 1024 * 2;
	if (udp_ops.fd != -1) {
		if (fcntl(udp_ops.fd, F_SETFD, FD_CLOEXEC) == -1) {
		}
	}
#ifdef SO_NOSIGPIPE
	if (udp_ops.fd != -1)
		setsockopt(udp_ops.fd, SOL_SOCKET, SO_NOSIGPIPE, &(int) { 1 }, sizeof(int));
#endif
	ret = bind(udp_ops.fd, (struct sockaddr *)&addr, len);
	if (ret < 0) {
	}
	getsockname(udp_ops.fd, (struct sockaddr *)&addr, &len);
	// printf("%s \n",inet_ntoa(addr.sin_addr));
	return 0;
}

int udp_read(void **ptr, size_t *len)
{
	static unsigned char buf[2048];
	int size = 2048;
	int ret;
	ret = recv(udp_ops.fd, buf, size, 0);
	*ptr = buf;
	*len = ret;
	return 0;
}

int udp_close(void)
{
	if (udp_ops.fd >= 0)
		close(udp_ops.fd);
	return 0;
}

static int udp_end(void)
{
	/*how to define the end of a stream*/
	return 1;
}

static struct io_ops udp_ops = {
	.type = IO_UDP, .open = udp_open, .read = udp_read, .close = udp_close, .end = udp_end,
};

REGISTER_IO_OPS(udp, &udp_ops);
