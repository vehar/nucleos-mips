/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */

#define _MINIX 1
#define _SYSTEM 1

#include <nucleos/config.h>
#include <ansi.h> 
#include <assert.h> 
#include <sys/types.h>
#include <nucleos/const.h>
#include <nucleos/type.h>

#include <fcntl.h>
#include <unistd.h>
#include <nucleos/syslib.h>
#include <nucleos/sysutil.h>

#include <limits.h>
#include <errno.h>

#define ASYN_NR	100
PRIVATE asynmsg_t msgtable[ASYN_NR];
PRIVATE int first_slot= 0, next_slot= 0;

PUBLIC int asynsend(dst, mp)
endpoint_t dst;
message *mp;
{
	int r, src_ind, dst_ind;
	unsigned flags;
	static int first = 1;
	static int inside = 0;
	int len;

	/* printf() causes asynsend? */
	if(inside) {
		exit(1);
	}

	inside = 1;

	if(first) {
		int i;
		for(i = 0; i < ASYN_NR; i++) {
			msgtable[i].flags = AMF_EMPTY;
		}
		first = 0;
	}

	/* Update first_slot */
	for (; first_slot < next_slot; first_slot++)
	{
		flags= msgtable[first_slot].flags;
		if ((flags & (AMF_VALID|AMF_DONE)) == (AMF_VALID|AMF_DONE))
		{
			if (msgtable[first_slot].result != OK)
			{
#if 0
				printf(
			"asynsend: found completed entry %d with error %d\n",
					first_slot,
					msgtable[first_slot].result);
#endif
			}
			continue;
		}
		if (flags != AMF_EMPTY)
			break;
	}

	if (first_slot >= next_slot)
	{
		/* Reset first_slot and next_slot */
		next_slot= first_slot= 0;
	}

	if (next_slot >= ASYN_NR)
	{
		/* Tell the kernel to stop processing */
		r= senda(NULL, 0);
		if (r != OK)
			panic(__FILE__, "asynsend: senda failed", r);

		dst_ind= 0;
		for (src_ind= first_slot; src_ind<next_slot; src_ind++)
		{
			flags= msgtable[src_ind].flags;
			if ((flags & (AMF_VALID|AMF_DONE)) ==
				(AMF_VALID|AMF_DONE))
			{
				if (msgtable[src_ind].result != OK)
				{
#if 0
					printf(
			"asynsend: found completed entry %d with error %d\n",
						src_ind,
						msgtable[src_ind].result);
#endif
				}
				continue;
			}
			if (flags == AMF_EMPTY)
				continue;
#if 0
			printf("asynsend: copying entry %d to %d\n",
				src_ind, dst_ind);
#endif
			if (src_ind != dst_ind)
				msgtable[dst_ind]= msgtable[src_ind];
			dst_ind++;
		}
		first_slot= 0;
		next_slot= dst_ind;
		if (next_slot >= ASYN_NR)
			panic(__FILE__, "asynsend: msgtable full", NO_NUM);
	}

	msgtable[next_slot].dst= dst;
	msgtable[next_slot].msg= *mp;
	msgtable[next_slot].flags= AMF_VALID;	/* Has to be last. The kernel 
					 	 * scans this table while we
						 * are sleeping.
					 	 */
	next_slot++;

	assert(first_slot < ASYN_NR);
	assert(next_slot >= first_slot);
	len = next_slot-first_slot;
	assert(first_slot + len <= ASYN_NR);

	/* Tell the kernel to rescan the table */
	r = senda(msgtable+first_slot, len);

	inside = 0;

	return r;
}
