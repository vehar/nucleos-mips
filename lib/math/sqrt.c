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
 * (c) copyright 1988 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 *
 * Author: Ceriel J.H. Jacobs
 */
/* $Header: /cvsup/minix/src/lib/math/sqrt.c,v 1.1.1.1 2005/04/21 14:56:26 beng Exp $ */

#include	<math.h>
#include	<float.h>
#include	<errno.h>

#define NITER	5

double
sqrt(double x)
{
	int exponent;
	double val;

	if (__IsNan(x)) {
		errno = EDOM;
		return x;
	}
	if (x <= 0) {
		if (x < 0) errno = EDOM;
		return 0;
	}

	if (x > DBL_MAX) return x;	/* for infinity */

	val = frexp(x, &exponent);
	if (exponent & 1) {
		exponent--;
		val *= 2;
	}
	val = ldexp(val + 1.0, exponent/2 - 1);
	/* was: val = (val + 1.0)/2.0; val = ldexp(val, exponent/2); */
	for (exponent = NITER - 1; exponent >= 0; exponent--) {
		val = (val + x / val) / 2.0;
	}
	return val;
}