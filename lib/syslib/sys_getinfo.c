/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */

#include <string.h>

#include "syslib.h"

/*===========================================================================*
 *                                sys_getinfo				     *
 *===========================================================================*/
int sys_getinfo(request, ptr, len, ptr2, len2)
int request; 				/* system info requested */
void *ptr;				/* pointer where to store it */
int len;				/* max length of value to get */
void *ptr2;				/* second pointer */
int len2;				/* length or process nr */ 
{
    message m;

    m.I_REQUEST = request;
    m.I_ENDPT = SELF;			/* always store values at caller */
    m.I_VAL_PTR = ptr;
    m.I_VAL_LEN = len;
    m.I_VAL_PTR2 = ptr2;
    m.I_VAL_LEN2_E = len2;

    return(_taskcall(SYSTASK, SYS_GETINFO, &m));
}

/*===========================================================================*
 *                                sys_whoami				     *
 *===========================================================================*/
int sys_whoami(endpoint_t *who_ep, char *who_name, int len)
{
	message m;
	int r;
	int lenmin;

	m.I_REQUEST = GET_WHOAMI;

	if(len < 2)
		return -EINVAL;

	if((r = _taskcall(SYSTASK, SYS_GETINFO, &m)) != 0)
		return r;

	lenmin = MIN(len, sizeof(m.GIWHO_NAME)) - 1;

	strncpy(who_name, m.GIWHO_NAME, lenmin);
	who_name[lenmin] = '\0';
	*who_ep = m.GIWHO_EP;

	return 0;
}

