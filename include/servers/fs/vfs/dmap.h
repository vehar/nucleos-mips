/*
 *  Copyright (C) 2012  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
#ifndef __SERVERS_FS_VFS_DMAP_H
#define __SERVERS_FS_VFS_DMAP_H

/*===========================================================================*
 *               	 Device <-> Driver Table  			     *
 *===========================================================================*/

/* Device table.  This table is indexed by major device number.  It provides
 * the link between major device numbers and the routines that process them.
 * The table can be update dynamically. The field 'dmap_flags' describe an 
 * entry's current status and determines what control options are possible. 
 */
#define DMAP_MUTABLE		0x01	/* mapping can be overtaken */
#define DMAP_BUSY		0x02	/* driver busy with request */
#define DMAP_BABY		0x04	/* driver exec() not done yet */

struct dmap {
  int (*dmap_opcl)(int, dev_t, int, int);
  int (*dmap_io)(int, kipc_msg_t *);
  endpoint_t dmap_driver;
  int dmap_flags;
  char dmap_label[16];
  int dmap_async_driver;
  struct filp *dmap_sel_filp;
};

extern struct dmap dmap[];

#endif /* __SERVERS_FS_VFS_DMAP_H */
