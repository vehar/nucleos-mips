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
 * Library function used by system profiling.
 *
 * The variables and functions in this file are used by the procentry/
 * procentry syslib functions when linked with userspace processes. For
 * kernel processes, the same variables and function are defined
 * elsewhere. This enables different functionality and variable sizes,
 * which is needed is a few cases.
 *
 * Changes:
 *   14 Aug, 2006   Created (Rogier Meurs)
 */

#include <nucleos/lib.h>

#include <nucleos/profile.h>
#include <nucleos/syslib.h>

/* A regular sized table is declared for the userspace processes. */
struct cprof_tbl_s cprof_tbl[CPROF_TABLE_SIZE_OTHER];

/* Function that returns table size. */
int profile_get_tbl_size(void)
{
  return CPROF_TABLE_SIZE_OTHER;
}

/* Function that returns on which execution of procentry to announce. */
int profile_get_announce(void)
{
  return CPROF_ANNOUNCE_OTHER;
}

/*
 * Userspace processes announce their control struct and table locations
 * to the kernel through this function.
 */
void profile_register(ctl_ptr, tbl_ptr)
void *ctl_ptr;
void *tbl_ptr;
{
  sys_profbuf(ctl_ptr, tbl_ptr);
}

