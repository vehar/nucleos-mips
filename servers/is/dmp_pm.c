/*
 *  Copyright (C) 2012  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/* This file contains procedures to dump to PM' data structures.
 *
 * The entry points into this file are
 *   mproc_dmp:   	display PM process table	  
 *
 * Created:
 *   May 11, 2005:	by Jorrit N. Herder
 */

#include "inc.h"
#include <servers/pm/mproc.h>
#include <nucleos/timer.h>
#include <nucleos/type.h>

struct mproc mproc[NR_PROCS];

/*===========================================================================*
 *				mproc_dmp				     *
 *===========================================================================*/
static char *flags_str(int flags)
{
	static char str[14];
	str[0] = (flags & WAITING) ? 'W' : '-';
	str[1] = (flags & ZOMBIE)  ? 'Z' : '-';
	str[2] = (flags & PAUSED)  ? 'P' : '-';
	str[3] = (flags & ALARM_ON)  ? 'A' : '-';
	str[4] = (flags & EXITING) ? 'E' : '-';
	str[5] = (flags & STOPPED)  ? 'S' : '-';
	str[6] = (flags & SIGSUSPENDED)  ? 'U' : '-';
	str[7] = (flags & REPLY)  ? 'R' : '-';
	str[8] = (flags & FS_CALL) ? 'F' : '-';
	str[9] = (flags & PM_SIG_PENDING) ? 's' : '-';
	str[10] = (flags & PRIV_PROC)  ? 'p' : '-';
	str[11] = (flags & PARTIAL_EXEC) ? 'x' : '-';
	str[12] = (flags & DELAY_CALL) ? 'd' : '-';
	str[13] = '\0';

	return str;
}

void mproc_dmp()
{
  struct mproc *mp;
  int i, n=0;
  static int prev_i = 0;

  printk("Process manager (PM) process table dump\n");

  getsysinfo(PM_PROC_NR, SI_PROC_TAB, mproc);

  printk("-process- -nr-pnr-tnr- --pid--ppid--pgrp- -uid--  -gid--  -nice- -flags-------\n");
  for (i=prev_i; i<NR_PROCS; i++) {
  	mp = &mproc[i];
  	if (mp->mp_pid == 0 && i != PM_PROC_NR) continue;
  	if (++n > 22) break;
  	printk("%8.8s %4d%4d%4d  %5d %5d %5d  ", 
  		mp->mp_name, i, mp->mp_parent, mp->mp_tracer, mp->mp_pid, mproc[mp->mp_parent].mp_pid, mp->mp_procgrp);
  	printk("%2d(%2d)  %2d(%2d)   ",
  		mp->mp_realuid, mp->mp_effuid, mp->mp_realgid, mp->mp_effgid);
  	printk(" %3d  %s  ", 
  		mp->mp_nice, flags_str(mp->mp_flags)); 
  	printk("\n");
  }
  if (i >= NR_PROCS) i = 0;
  else printk("--more--\r");
  prev_i = i;
}

/*===========================================================================*
 *				sigaction_dmp				     *
 *===========================================================================*/
void sigaction_dmp()
{
  struct mproc *mp;
  int i, n=0;
  static int prev_i = 0;
  clock_t uptime;

  printk("Process manager (PM) signal action dump\n");

  getsysinfo(PM_PROC_NR, SI_PROC_TAB, mproc);
  getuptime(&uptime);

  printk("-process- -nr- --ignore- --catch- --block- -tomess- -pending- -alarm---\n");
  for (i=prev_i; i<NR_PROCS; i++) {
  	mp = &mproc[i];
  	if (mp->mp_pid == 0 && i != PM_PROC_NR) continue;
  	if (++n > 22) break;
  	printk("%8.8s  %3d  ", mp->mp_name, i);
  	printk(" %08x %08x %08x %08x  ", 
  		mp->mp_ignore, mp->mp_catch, mp->mp_sigmask, mp->mp_sig2mess); 
  	printk("%08x  ", mp->mp_sigpending);
  	if (mp->mp_flags & ALARM_ON) printk("%8u", mp->mp_timer.tmr_exp_time-uptime);
  	else printk("       -");
  	printk("\n");
  }
  if (i >= NR_PROCS) i = 0;
  else printk("--more--\r");
  prev_i = i;
}


