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
icmp.h

Copyright 1995 Philip Homburg
*/

#ifndef ICMP_H
#define ICMP_H

#define ICMP_MAX_DATAGRAM	8196
#define ICMP_DEF_TTL		96

/* Rate limit. The implementation is a bit sloppy and may send twice the
 * number of packets. 
 */
#define ICMP_MAX_RATE		100	/* This many per interval */
#define ICMP_RATE_INTERVAL	(1*HZ)	/* Interval in ticks */
#define ICMP_RATE_WARN		10	/* Report this many dropped packets */

/* Prototypes */

void icmp_prep(void);
void icmp_init(void);


#endif /* ICMP_H */
