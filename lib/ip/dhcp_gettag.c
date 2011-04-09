/*
 *  Copyright (C) 2011  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/*	dhcp_gettag()					Author: Kees J. Bot
 *								1 Dec 2000
 */
#define nil ((void*)0)
#include <nucleos/stddef.h>
#include <nucleos/string.h>
#include <nucleos/types.h>
#include <net/hton.h>
#include <net/in.h>
#include <net/dhcp.h>

#define arraysize(a)	(sizeof(a) / sizeof((a)[0]))

int dhcp_gettag(dhcp_t *dp, int searchtag, u8_t **pdata, size_t *plen)
{
    /* Find a tag in the options field, or possibly in the file or sname
     * fields.  Return true iff found, and return the data and/or length if
     * their pointers are non-null.
     */
    u8_t *p;
    u8_t *optfield[3];
    size_t optlen[3];
    int i, tag, len;

    /* The DHCP magic number must be correct, or no tags. */
    if (dp->magic != DHCP_MAGIC) return 0;

    optfield[0]= dp->options;
    optlen[0]= arraysize(dp->options);
    optfield[1]= dp->file;
    optlen[1]= 0;		/* Unknown if used for options yet. */
    optfield[2]= dp->sname;
    optlen[2]= 0;

    for (i= 0; i < 3; i++) {
	p= optfield[i];
	while (p < optfield[i] + optlen[i]) {
	    tag= *p++;
	    if (tag == 255) break;
	    len= tag == 0 ? 0 : *p++;
	    if (tag == searchtag) {
		if (pdata != nil) *pdata= p;
		if (plen != nil) *plen= len;
		return 1;
	    }
	    if (tag == DHCP_TAG_OVERLOAD) {
		/* There are also options in the file or sname field. */
		if (*p & 1) optlen[1]= arraysize(dp->file);
		if (*p & 2) optlen[1]= arraysize(dp->sname);
	    }
	    p += len;
	}
    }
    return 0;
}
