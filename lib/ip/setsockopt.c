/*
 *  Copyright (C) 2012  Ladislav Klenovic <klenovic@nucleonsoft.com>
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

static int _tcp_setsockopt(int socket, int level, int option_name,
	const void *option_value, socklen_t option_len);

static int _udp_setsockopt(int socket, int level, int option_name,
	const void *option_value, socklen_t option_len);

int setsockopt(int socket, int level, int option_name,
        const void *option_value, socklen_t option_len)
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
		return _tcp_setsockopt(socket, level, option_name,
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
		return _udp_setsockopt(socket, level, option_name,
			option_value, option_len);
	}

#if DEBUG
	fprintf(stderr, "setsockopt: not implemented for fd %d\n", socket);
#endif
	errno= ENOTSOCK;
	return -1;
}

static int _tcp_setsockopt(int socket, int level, int option_name,
	const void *option_value, socklen_t option_len)
{
	int i;

	if (level == SOL_SOCKET && option_name == SO_KEEPALIVE)
	{
		if (option_len != sizeof(i))
		{
			errno= EINVAL;
			return -1;
		}
		i= *(int *)option_value;
		if (!i)
		{
			/* At the moment there is no way to turn off 
			 * keepalives.
			 */
			errno= ENOSYS;
			return -1;
		}
		return 0;
	}
	if (level == SOL_SOCKET && option_name == SO_RCVBUF)
	{
		if (option_len != sizeof(i))
		{
			errno= EINVAL;
			return -1;
		}
		i= *(int *)option_value;
		if (i > 32*1024)
		{
			/* The receive buffer is limited to 32K at the moment.
			 */
			errno= ENOSYS;
			return -1;
		}
		/* There is no way to reduce the receive buffer, do we have to
		 * let this call fail for smaller buffers?
		 */
		return 0;
	}
	if (level == SOL_SOCKET && option_name == SO_SNDBUF)
	{
		if (option_len != sizeof(i))
		{
			errno= EINVAL;
			return -1;
		}
		i= *(int *)option_value;
		if (i > 32*1024)
		{
			/* The send buffer is limited to 32K at the moment.
			 */
			errno= ENOSYS;
			return -1;
		}
		/* There is no way to reduce the send buffer, do we have to
		 * let this call fail for smaller buffers?
		 */
		return 0;
	}
	if (level == IPPROTO_TCP && option_name == TCP_NODELAY)
	{
		if (option_len != sizeof(i))
		{
			errno= EINVAL;
			return -1;
		}
		i= *(int *)option_value;
		if (i)
		{
			/* At the moment there is no way to turn on 
			 * nodelay.
			 */
			errno= ENOSYS;
			return -1;
		}
		return 0;
	}
#if DEBUG
	fprintf(stderr, "_tcp_setsocketopt: level %d, name %d\n",
		level, option_name);
#endif

	errno= ENOSYS;
	return -1;
}

static int _udp_setsockopt(int socket, int level, int option_name,
	const void *option_value, socklen_t option_len)
{
	int i;

#if DEBUG
	fprintf(stderr, "_udp_setsocketopt: level %d, name %d\n",
		level, option_name);
#endif

	errno= ENOSYS;
	return -1;
}

