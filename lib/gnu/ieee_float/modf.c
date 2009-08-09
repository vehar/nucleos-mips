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
libc/ieee_float/modf.c

Created:	Oct 14, 1993 by Philip Homburg <philip@cs.vu.nl>

Implementation of modf that directly manipulates the exponent bits in an
ieee float
*/

#include <nucleos/types.h>
#include <math.h>

#include "ieee_float.h"

double modf(value, iptr)
double value;
double *iptr;
{
	struct f64 *f64p;
	double tmp;
	int exp;
	int mask_bits;
	u32_t mant;

	f64p= (struct f64 *)&value;

	exp= F64_GET_EXP(f64p);
	exp -= F64_EXP_BIAS;
	if (exp < 0)
	{
		*iptr= 0;
		return value;
	}
	mask_bits= 52-exp;
	if (mask_bits <= 0)
	{
		*iptr= value;
		return 0;
	}
	tmp= value;
	if (mask_bits >= 32)
	{
		F64_SET_MANT_LOW(f64p, 0);
		mask_bits -= 32;
		mant= F64_GET_MANT_HIGH(f64p);
		mant &= ~((1 << mask_bits)-1);
		F64_SET_MANT_HIGH(f64p, mant);
	}
	else
	{
		mant= F64_GET_MANT_LOW(f64p);
		mant &= ~((1 << mask_bits)-1);
		F64_SET_MANT_LOW(f64p, mant);
	}
	*iptr= value;
	return tmp-value;
}
