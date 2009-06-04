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
 *                                vm_exit				     *
 *===========================================================================*/
PUBLIC int vm_exit(endpoint_t ep)
{
    message m;
    int result;

    m.VME_ENDPOINT = ep;

    result = _taskcall(VM_PROC_NR, VM_EXIT, &m);
    return(result);
}


/*===========================================================================*
 *                                vm_willexit				     *
 *===========================================================================*/
PUBLIC int vm_willexit(endpoint_t ep)
{
    message m;
    int result;

    m.VMWE_ENDPOINT = ep;

    result = _taskcall(VM_PROC_NR, VM_WILLEXIT, &m);
    return(result);
}
