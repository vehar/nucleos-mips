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
 *                                vm_push_sig			     *
 *===========================================================================*/
PUBLIC int vm_push_sig(endpoint_t ep, vir_bytes *old_sp)
{
    message m;
    int result;

    m.VMPS_ENDPOINT = ep;
    result = _taskcall(VM_PROC_NR, VM_PUSH_SIG, &m);
    *old_sp = (vir_bytes)  m.VMPS_OLD_SP;

    return result;
}
