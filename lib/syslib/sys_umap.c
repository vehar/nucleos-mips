/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
#include "syslib.h"

/*===========================================================================*
 *                                sys_umap				     *
 *===========================================================================*/
PUBLIC int sys_umap(proc_nr, seg, vir_addr, bytes, phys_addr)
int proc_nr; 				/* process number to do umap for */
int seg;				/* T, D, or S segment */
vir_bytes vir_addr;			/* address in bytes with segment*/
vir_bytes bytes;			/* number of bytes to be copied */
phys_bytes *phys_addr;			/* placeholder for result */
{
    message m;
    int result;

    m.CP_SRC_ENDPT = proc_nr;
    m.CP_SRC_SPACE = seg;
    m.CP_SRC_ADDR = vir_addr;
    m.CP_NR_BYTES = bytes;

    result = _taskcall(SYSTASK, SYS_UMAP, &m);
    *phys_addr = m.CP_DST_ADDR;
    return(result);
}
