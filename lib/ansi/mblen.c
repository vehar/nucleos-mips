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
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */
/* $Header: /cvsup/minix/src/lib/ansi/mblen.c,v 1.1.1.1 2005/04/21 14:56:05 beng Exp $ */

#include	<stdlib.h>
#include	<nucleos/limits.h>

#define	CHAR_SHIFT	8

int
mblen(const char *s, size_t n)
{
	if (s == (const char *)NULL) return 0;	/* no state dependent codings */
	if (n <= 0) return 0;
	return (*s != 0);
}
