/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/*

   getsockname()

   from socket emulation library for Minix 2.0.x

*/


#include <nucleos/errno.h>
#include <stdio.h>
#include <nucleos/string.h>
#include <nucleos/ioctl.h>
#include <nucleos/socket.h>
#include <netinet/in.h>

#include <net/in.h>
#include <net/tcp.h>
#include <net/tcp_io.h>
#include <net/udp.h>
#include <net/udp_io.h>

#include <asm/ioctls.h>

/*
#define DEBUG 0
*/

/*
   getsockname...
*/
int getsockname(int fd, struct sockaddr *__restrict address, 
   socklen_t *__restrict address_len)
{
	nwio_tcpconf_t tcpconf;
	socklen_t len;
	struct sockaddr_in sin;

#ifdef DEBUG
	fprintf(stderr,"mnx_getsockname: ioctl fd %d.\n", fd);
#endif
	if (ioctl(fd, NWIOGTCPCONF, &tcpconf)==-1) {
#ifdef DEBUG
	   fprintf(stderr,"mnx_getsockname: error %d\n", errno);
#endif
	   return (-1);
	   }
#ifdef DEBUG1
	fprintf(stderr, "mnx_getsockname: from %s, %u",
			inet_ntoa(tcpconf.nwtc_remaddr),
			ntohs(tcpconf.nwtc_remport));
	fprintf(stderr," for %s, %u\n",
			inet_ntoa(tcpconf.nwtc_locaddr),
			ntohs(tcpconf.nwtc_locport));
#endif
/*
	addr->sin_addr.s_addr = tcpconf.nwtc_remaddr ;
	addr->sin_port = tcpconf.nwtc_locport;
*/
	memset(&sin, '\0', sizeof(sin));
	sin.sin_family= AF_INET;
	sin.sin_addr.s_addr= tcpconf.nwtc_locaddr ;
	sin.sin_port= tcpconf.nwtc_locport;

	len= *address_len;
	if (len > sizeof(sin))
		len= sizeof(sin);
	memcpy(address, &sin, len);
	*address_len= len;

	return 0;
}








