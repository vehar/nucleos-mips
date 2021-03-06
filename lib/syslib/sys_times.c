/*
 *  Copyright (C) 2012  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
#include <nucleos/syslib.h>

int sys_times(proc_ep, user_time, sys_time, uptime, boottime)
endpoint_t proc_ep;		/* proc_ep whose times are needed */
clock_t *user_time;		/* time spend in the process itself */
clock_t *sys_time;		/* time spend in system on behalf of the
				 * process
				 */
clock_t *uptime;		/* time the system is running */
time_t *boottime;		/* boot time */
{
/* Fetch the accounting info for a proc_ep. */
  kipc_msg_t m;
  int r;

  m.T_ENDPT = proc_ep;
  r = ktaskcall(SYSTASK, SYS_TIMES, &m);
  if (user_time) *user_time = m.T_USER_TIME;
  if (sys_time) *sys_time = m.T_SYSTEM_TIME;
  if (uptime) *uptime = m.T_BOOT_TICKS;
  if (boottime) *boottime = m.T_BOOTTIME;
  return(r);
}
