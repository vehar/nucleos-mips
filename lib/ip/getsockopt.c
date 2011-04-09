/*
 *  Copyright (C) 2011  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
#include <assert.h>
#include <nucleos/errno.h>
#include <stdio.h>
#include <nucleos/string.h>
#include <nucleos/ioctl.h>
#include <nucleos/socket.h>
#include <nucleos/types.h>
#include <netinet/tcp.h>

#include <net/in.h>
#include <net/tcp.h>
#include <net/tcp_io.h>
#include <net/udp.h>
#include <net/udp_io.h>

#include <asm/ioctls.h>

#define DEBUG 0

static int _tcp_getsockopt(int socket, int level, int option_name,
	void *__restrict option_value, socklen_t *__restrict option_len);
static int _udp_getsockopt(int socket, int level, int option_name,
	void *__restrict option_value, socklen_t *__restrict option_len);
static void getsockopt_copy(void *return_value, size_t return_len,
	void *__restrict option_value, socklen_t *__restrict option_len);

int getsockopt(int socket, int level, int option_name,
        void *__restrict option_value, socklen_t *__restrict option_len)
{
	int r;
	nwio_tcpopt_t tcpopt;
	nwio_udpopt_t udpopt;

	r= ioctl(socket, NWIOGTCPOPT, &tcpopt);
	if (r != -1 || errno != ENOTTY)
	{
		if (r == -1)
		{
			/* Bad file descriptor */
			return -1;
		}
		return _tcp_getsockopt(socket, level, option_name,
			option_value, option_len);
	}

	r= ioctl(socket, NWIOGUDPOPT, &udpopt);
	if (r != -1 || errno != ENOTTY)
	{
		if (r == -1)
		{
			/* Bad file descriptor */
			return -1;
		}
		return _udp_getsockopt(socket, level, option_name,
			option_value, option_len);
	}

#if DEBUG
	fprintf(stderr, "getsockopt: not implemented for fd %d\n", socket);
#endif
	errno= ENOTSOCK;
	return -1;
}

static void getsockopt_copy(void *return_value, size_t return_len,
	void *__restrict option_value, socklen_t *__restrict option_len)
{
	/* copy as much data as possible */
	if (*option_len < return_len)
		memcpy(option_value, return_value, *option_len);
	else
		memcpy(option_value, return_value, return_len);

	/* return length */
	*option_len = return_len;
}

static int _tcp_getsockopt(int socket, int level, int option_name,
	void *__restrict option_value, socklen_t *__restrict option_len)
{
	int i, r, err;

	if (level == SOL_SOCKET && option_name == SO_KEEPALIVE)
	{
		i = 1;	/* Keepalive is always on */
		getsockopt_copy(&i, sizeof(i), option_value, option_len);
		return 0;
	}
	if (level == SOL_SOCKET && option_name == SO_ERROR)
	{
		r = ioctl(socket, NWIOTCPGERROR, &err);
		if (r != 0)
			return r;

		getsockopt_copy(&err, sizeof(err), option_value, option_len);
		return 0;
	}
	if (level == SOL_SOCKET && option_name == SO_RCVBUF)
	{
		i = 32 * 1024;	/* Receive buffer in the current
				 * implementation
				 */
		getsockopt_copy(&i, sizeof(i), option_value, option_len);
		return 0;
	}
	if (level == SOL_SOCKET && option_name == SO_SNDBUF)
	{
		i = 32 * 1024;	/* Send buffer in the current implementation */
		getsockopt_copy(&i, sizeof(i), option_value, option_len);
		return 0;
	}
	if (level == SOL_SOCKET && option_name == SO_TYPE)
	{
		i = SOCK_STREAM;	/* this is a TCP socket */
		getsockopt_copy(&i, sizeof(i), option_value, option_len);
		return 0;
	}
	if (level == IPPROTO_TCP && option_name == TCP_NODELAY)
	{
		i = 0;	/* nodelay is always off */
		getsockopt_copy(&i, sizeof(i), option_value, option_len);
		return 0;
	}
#if DEBUG
	fprintf(stderr, "_tcp_getsocketopt: level %d, name %d\n",
		level, option_name);
#endif

	errno= ENOPROTOOPT;
	return -1;
}

static int _udp_getsockopt(int socket, int level, int option_name,
	void *__restrict option_value, socklen_t *__restrict option_len)
{
	int i;

	if (level == SOL_SOCKET && option_name == SO_TYPE)
	{
		i = SOCK_DGRAM;	/* this is a TCP socket */
		getsockopt_copy(&i, sizeof(i), option_value, option_len);
		return 0;
	}
#if DEBUG
	fprintf(stderr, "_udp_getsocketopt: level %d, name %d\n",
		level, option_name);
#endif

	errno= ENOSYS;
	return -1;
}
