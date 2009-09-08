/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
#include <nucleos/errno.h>
#include <stdio.h>
#include <nucleos/uio.h>

ssize_t writev(int fildes, const struct iovec *iov, int iovcnt)
{
#if DEBUG
	fprintf(stderr, "bind: not implemented for fd %d\n", socket);
#endif
	errno= ENOSYS;
	return -1;

#if 0
	int i, r;
	char *p;
	ssize_t l, sum;

	/* We should buffer */
	sum= 0;
	for (i= 0; i<iovcnt; i++)
	{
		p= iov[i].iov_base;
		l= iov[i].iov_len;
		while (l > 0)
		{
			r= write(fildes, p, l);
			if (r <= 0)
			{
				assert(sum == 0);
				return r;
			}
			p += r;
			l -= r;
			sum += r;
		}
	}
	return sum;
#endif
}
