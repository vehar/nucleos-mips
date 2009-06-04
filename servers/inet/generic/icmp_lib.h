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
icmp_lib.h

Created Sept 30, 1991 by Philip Homburg

Copyright 1995 Philip Homburg
*/

#ifndef ICMP_LIB_H
#define ICMP_LIB_H

/* Prototypes */

void icmp_snd_parmproblem ARGS(( acc_t *pack ));
void icmp_snd_time_exceeded ARGS(( int port_nr, acc_t *pack, int code ));
void icmp_snd_unreachable ARGS(( int port_nr, acc_t *pack, int code ));
void icmp_snd_redirect ARGS(( int port_nr, acc_t *pack, int code,
							ipaddr_t gw ));
void icmp_snd_mtu ARGS(( int port_nr, acc_t *pack, U16_t mtu ));

#endif /* ICMP_LIB_H */

/*
 * $PchId: icmp_lib.h,v 1.6 2002/06/08 21:32:44 philip Exp $
 */