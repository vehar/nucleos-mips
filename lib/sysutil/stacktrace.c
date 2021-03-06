/*
 *  Copyright (C) 2012  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/*
stacktrace.c

Created:	Jan 19, 1993 by Philip Homburg

Copyright 1995 Philip Homburg
*/

#include <stdio.h>
#include <nucleos/string.h>

typedef unsigned int reg_t;

#define FUNC_STACKTRACE(statement) 				\
{								\
	reg_t bp, pc, hbp;					\
	extern reg_t get_bp(void);				\
								\
	bp= get_bp();						\
	while(bp)						\
	{							\
		pc= ((reg_t *)bp)[1];				\
		hbp= ((reg_t *)bp)[0];				\
		statement;					\
		if (hbp != 0 && hbp <= bp)			\
		{						\
			pc = -1;				\
			statement;				\
			break;					\
		}						\
		bp= hbp;					\
	}							\
}

void util_nstrcat(char *str, unsigned long number)
{
	int n = 10, lead = 1;
	char nbuf[12], *p;
	p = nbuf;
	*p++ = '0';
	*p++ = 'x';
	for(n = 0; n < 8; n++) {
		int i;
		i = (number >> ((7-n)*4)) & 0xF;
		if(!lead || i) {
			*p++ = i < 10 ? '0' + i : 'a' + i - 10;
			lead = 0;
		}
	}
	if(lead) *p++ = '0';
	*p++ = ' ';
	*p++ = '\0';
	strcat(str, nbuf);
}

void util_stacktrace(void)
{
	FUNC_STACKTRACE(printk("0x%lx ", (unsigned long) pc));
	printk("\n");
}

void util_stacktrace_strcat(char *str)
{
	FUNC_STACKTRACE(util_nstrcat(str, pc));
}

