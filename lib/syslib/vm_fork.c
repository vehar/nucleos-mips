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

#include <nucleos/vm.h>

/*===========================================================================*
 *                                vm_fork				     *
 *===========================================================================*/
PUBLIC int vm_fork(endpoint_t ep, int slot, int *childep)
{
    message m;
    int result;

    m.VMF_ENDPOINT = ep;
    m.VMF_SLOTNO = slot;

    result = _taskcall(VM_PROC_NR, VM_FORK, &m);

    *childep = m.VMF_CHILD_ENDPOINT;

    return(result);
}
