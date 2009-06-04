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
 * icompute.c - compute an integer
 */
/* $Header: /cvsup/minix/src/lib/stdio/icompute.c,v 1.1.1.1 2005/04/21 14:56:35 beng Exp $ */

#include	"loc_incl.h"

/* This routine is used in doprnt.c as well as in tmpfile.c and tmpnam.c. */

char *
_i_compute(unsigned long val, int base, char *s, int nrdigits)
{
	int c;

	c= val % base ;
	val /= base ;
	if (val || nrdigits > 1)
		s = _i_compute(val, base, s, nrdigits - 1);
	*s++ = (c>9 ? c-10+'a' : c+'0');
	return s;
}