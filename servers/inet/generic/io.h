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
io.h

Created Sept 30, 1991 by Philip Homburg

Copyright 1995 Philip Homburg
*/

#ifndef IO_H
#define IO_H

/* Prototypes */

void writeIpAddr ARGS(( ipaddr_t addr ));
void writeEtherAddr ARGS(( ether_addr_t *addr ));

#endif /* IO_H */

/*
 * $PchId: io.h,v 1.4 1995/11/21 06:45:27 philip Exp $
 */