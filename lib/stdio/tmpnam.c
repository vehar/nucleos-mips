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
 * tmpnam.c - create a unique filename
 */
#include <nucleos/types.h>
#include <stdio.h>
#include <nucleos/string.h>
#include "loc_incl.h"

pid_t getpid(void);

char *
tmpnam(char *s) {
	static char name_buffer[L_tmpnam] = "/tmp/tmp.";
	static unsigned long count = 0;
	static char *name = NULL;

	if (!name) { 
		name = name_buffer + strlen(name_buffer);
		name = _i_compute((unsigned long)getpid(), 10, name, 5);
		*name++ = '.';
		*name = '\0';
	}
	if (++count > TMP_MAX) count = 1;	/* wrap-around */
	*_i_compute(count, 10, name, 3) = '\0';
	if (s) return strcpy(s, name_buffer);
	else return name_buffer;
}
