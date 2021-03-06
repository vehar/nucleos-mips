/*
 *  Copyright (C) 2012  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/* Miscellaneous system calls.				Author: Kees J. Bot
 *								31 Mar 2000
 * The entry points into this file are:
 *   do_reboot: kill all processes, then reboot system
 *   do_procstat: request process status  (Jorrit N. Herder)
 *   do_getsysinfo: request copy of PM data structure  (Jorrit N. Herder)
 *   do_getprocnr: lookup process slot number  (Jorrit N. Herder)
 *   do_getepinfo: get the pid/uid/gid of a process given its endpoint
 *   do_allocmem: allocate a chunk of memory  (Jorrit N. Herder)
 *   do_freemem: deallocate a chunk of memory  (Jorrit N. Herder)
 *   do_getsetpriority: get/set process priority
 *   do_svrctl: process manager control
 */
#include <stdlib.h>
#include <nucleos/kernel.h>
#include "pm.h"
#include <nucleos/unistd.h>
#include <nucleos/signal.h>
#include <nucleos/svrctl.h>
#include <nucleos/resource.h>
#include <nucleos/utsname.h>
#include <nucleos/com.h>
#include <nucleos/sysinfo.h>
#include <nucleos/type.h>
#include <nucleos/vm.h>
#include <nucleos/string.h>
#include <nucleos/lib.h>
#include <nucleos/pci.h>
#include <assert.h>
#include <kernel/proc.h>
#include <asm/ioctls.h>
#include <asm/kernel/const.h>
#include <asm/kernel/types.h>
#include <servers/pm/mproc.h>
#include "param.h"

extern struct utsname uts_val;

#ifdef CONFIG_DEBUG_SERVERS_SYSCALL_STATS
unsigned long calls_stats[NR_syscalls];
#endif

static int getpciinfo(struct pciinfo *pciinfo);

/*===========================================================================*
 *				do_allocmem				     *
 *===========================================================================*/
int do_allocmem()
{
	int r;
	phys_bytes retmembase;

	r = vm_allocmem(m_in.memsize, (phys_clicks *)&retmembase);
	if(r == 0)
		mp->mp_reply.membase = retmembase;
	return r;
}

/*===========================================================================*
 *				do_freemem				     *
 *===========================================================================*/
int do_freemem()
{
#if 1
	return -ENOSYS;
#else
  vir_clicks mem_clicks;
  phys_clicks mem_base;

  /* This call is dangerous. Even memory belonging to other processes can
   * be freed.
   */
  if (mp->mp_effuid != 0)
  {
	printk("PM: unauthorized call of do_freemem by proc %d\n",
		mp->mp_endpoint);
	sys_sysctl_stacktrace(mp->mp_endpoint);
	return -EPERM;
  }

  mem_clicks = (m_in.memsize + CLICK_SIZE -1 ) >> CLICK_SHIFT;
  mem_base = (m_in.membase + CLICK_SIZE -1 ) >> CLICK_SHIFT;
  free_mem(mem_base, mem_clicks);
  return 0;
#endif
}

/*===========================================================================*
 *				do_procstat				     *
 *===========================================================================*/
int do_procstat()
{ 
  /* For the moment, this is only used to return pending signals to 
   * system processes that request the PM for their own status. 
   *
   * Future use might include the VFS_PROC_NR requesting for process status of
   * any user process. 
   */
  
  /* This call should be removed, or made more general. */

  if (m_in.stat_nr == ENDPT_SELF) {
      mp->mp_reply.sig_set = mp->mp_sigpending;
      sigemptyset(&mp->mp_sigpending);
  } 
  else {
      return(-ENOSYS);
  }
  return 0;
}

/*===========================================================================*
 *				do_getsysinfo			       	     *
 *===========================================================================*/
int do_getsysinfo()
{
  struct mproc *proc_addr;
  vir_bytes src_addr, dst_addr;
  struct kinfo kinfo;
  struct loadinfo loadinfo;
  struct pciinfo *pciinfo;
  static struct proc proctab[NR_PROCS+NR_TASKS];
  size_t len;
  int s, r;

  /* This call leaks important information (the contents of registers). */
  if (mp->mp_effuid != 0)
  {
	printk("PM: unauthorized call of do_getsysinfo by proc %d '%s'\n",
		mp->mp_endpoint, mp->mp_name);
	sys_sysctl_stacktrace(mp->mp_endpoint);
	return -EPERM;
  }

  switch(m_in.info_what) {
  case SI_KINFO:			/* kernel info is obtained via PM */
        sys_getkinfo(&kinfo);
        src_addr = (vir_bytes) &kinfo;
        len = sizeof(struct kinfo);
        break;
  case SI_PROC_ADDR:			/* get address of PM process table */
  	proc_addr = &mproc[0];
  	src_addr = (vir_bytes) &proc_addr;
  	len = sizeof(struct mproc *);
  	break; 
  case SI_PROC_TAB:			/* copy entire process table */
        src_addr = (vir_bytes) mproc;
        len = sizeof(struct mproc) * NR_PROCS;
        break;
  case SI_KPROC_TAB:			/* copy entire process table */
	if((r=sys_getproctab(proctab)) != 0)
		return r;
	src_addr = (vir_bytes) proctab;
	len = sizeof(proctab);
        break;
  case SI_LOADINFO:			/* loadinfo is obtained via PM */
        sys_getloadinfo(&loadinfo);
        src_addr = (vir_bytes) &loadinfo;
        len = sizeof(struct loadinfo);
        break;
  case SI_PCI_INFO:			/* PCI info is obtained via PM */
	pciinfo = malloc(sizeof(struct pciinfo));
	if (!pciinfo) {
		printk("PM: no enough memory!\n");
		return -ENOMEM;
	}

	if ((r = getpciinfo(pciinfo)) != 0) {
		free(pciinfo);
		return r;
	}

	src_addr = (vir_bytes)pciinfo;
	len = sizeof(struct pciinfo);
	break;

#ifdef CONFIG_DEBUG_SERVERS_SYSCALL_STATS
  case SI_CALL_STATS:
  	src_addr = (vir_bytes) calls_stats;
  	len = sizeof(calls_stats);
  	break; 
#endif
  default:
  	return(-EINVAL);
  }

  dst_addr = (vir_bytes) m_in.info_where;
  if ((s=sys_datacopy(ENDPT_SELF, src_addr, who_e, dst_addr, len)) != 0)
  	return(s);
  return 0;
}

/*===========================================================================*
 *				do_getsysinfo_up		       	     *
 *===========================================================================*/
int do_getsysinfo_up()
{
  vir_bytes src_addr, dst_addr;
  struct loadinfo loadinfo;
  size_t len, real_len;
  u64_t idle_tsc;
  int s;

  switch(m_in.SIU_WHAT) {
  case SIU_LOADINFO:			/* loadinfo is obtained via PM */
        if ((s = sys_getloadinfo(&loadinfo)) != 0)
        	return s;
        src_addr = (vir_bytes) &loadinfo;
        real_len = sizeof(struct loadinfo);
        break;
  case SIU_SYSTEMHZ:
        src_addr = (vir_bytes) &system_hz;
        real_len = sizeof(system_hz);
	break;
  case SIU_IDLETSC:
	if ((s = sys_getidletsc(&idle_tsc)) != 0)
		return s;
	src_addr = (vir_bytes) &idle_tsc;
	real_len = sizeof(idle_tsc);
	break;
  default:
  	return(-EINVAL);
  }

  /* Let application know what the length was. */
  len = real_len;
  if(len > m_in.SIU_LEN)
	len = m_in.SIU_LEN;

  dst_addr = (vir_bytes) m_in.SIU_WHERE;
  if ((s=sys_datacopy(ENDPT_SELF, src_addr, who_e, dst_addr, len)) != 0)
  	return(s);
  return(real_len);
}

/*===========================================================================*
 *				do_getprocnr			             *
 *===========================================================================*/
int do_getprocnr()
{
  register struct mproc *rmp;
  static char search_key[PROC_NAME_LEN+1];
  int key_len;
  int s;

  /* This call should be moved to DS. */
  if (mp->mp_effuid != 0)
  {
	/* For now, allow non-root processes to request their own endpoint. */
	if (m_in.pid < 0 && m_in.namelen == 0) {
		mp->mp_reply.PM_ENDPT = who_e;
		mp->mp_reply.PM_PENDPT = ENDPT_NONE;
		return 0;
	}

	printk("PM: unauthorized call of do_getprocnr by proc %d\n",
		mp->mp_endpoint);
	sys_sysctl_stacktrace(mp->mp_endpoint);
	return -EPERM;
  }

#if 0
  printk("PM: do_getprocnr(%d) call from endpoint %d, %s\n",
	m_in.pid, mp->mp_endpoint, mp->mp_name);
#endif

  if (m_in.pid >= 0) {			/* lookup process by pid */
	if ((rmp = find_proc(m_in.pid)) != NIL_MPROC) {
		mp->mp_reply.PM_ENDPT = rmp->mp_endpoint;
#if 0
		printk("PM: pid result: %d\n", rmp->mp_endpoint);
#endif
		return(0);
	}
  	return(-ESRCH);			
  } else if (m_in.namelen > 0) {	/* lookup process by name */
  	key_len = MIN(m_in.namelen, PROC_NAME_LEN);
 	if ((s=sys_datacopy(who_e, (vir_bytes) m_in.PMBRK_ADDR, 
 			ENDPT_SELF, (vir_bytes) search_key, key_len)) != 0) 
 		return(s);
 	search_key[key_len] = '\0';	/* terminate for safety */
  	for (rmp = &mproc[0]; rmp < &mproc[NR_PROCS]; rmp++) {
		if (((rmp->mp_flags & (IN_USE | EXITING)) == IN_USE) && 
			strncmp(rmp->mp_name, search_key, key_len)==0) {
  			mp->mp_reply.PM_ENDPT = rmp->mp_endpoint;
  			return(0);
		} 
	}
  	return(-ESRCH);			
  } else {			/* return own/parent process number */
#if 0
	printk("PM: endpt result: %d\n", mp->mp_reply.PM_ENDPT);
#endif
  	mp->mp_reply.PM_ENDPT = who_e;
	mp->mp_reply.PM_PENDPT = mproc[mp->mp_parent].mp_endpoint;
  }

  return(0);
}

/*===========================================================================*
 *				do_getepinfo			             *
 *===========================================================================*/
int do_getepinfo()
{
  register struct mproc *rmp;
  endpoint_t ep;

  /* This call should be moved to DS. */
  if (mp->mp_effuid != 0)
  {
	printk("PM: unauthorized call of do_getepinfo by proc %d\n",
		mp->mp_endpoint);
	sys_sysctl_stacktrace(mp->mp_endpoint);
	return -EPERM;
  }

  ep= m_in.PM_ENDPT;

  for (rmp = &mproc[0]; rmp < &mproc[NR_PROCS]; rmp++) {
	if ((rmp->mp_flags & IN_USE) && (rmp->mp_endpoint == ep)) {
		mp->mp_reply.reply_res2 = rmp->mp_effuid;
		mp->mp_reply.reply_res3 = rmp->mp_effgid;
		return(rmp->mp_pid);
	}
  } 

  /* Process not found */
  return(-ESRCH);
}

/*===========================================================================*
 *				do_reboot				     *
 *===========================================================================*/
int do_reboot()
{
	kipc_msg_t m;

	/* Check permission to abort the system. */
	if (mp->mp_effuid != SUPER_USER) return(-EPERM);

	/* See how the system should be aborted. */
	abort_flag = (unsigned) m_in.reboot_flag;
	if (abort_flag >= RBT_INVALID)
		return(-EINVAL);

	/* Order matters here. When VFS_PROC_NR is told to reboot, it exits all its
	 * processes, and then would be confused if they're exited again by
	 * SIGKILL. So first kill, then reboot. 
	 */

	check_sig(-1, SIGKILL); 		/* kill all users except init */
	sys_stop(INIT_PROC_NR);		/* stop init, but keep it around */

	/* Tell FS to reboot */
	m.m_type = PM_REBOOT;

	tell_fs(&mproc[VFS_PROC_NR], &m);

	return(SUSPEND);	/* don't reply to caller */
}

/*===========================================================================*
 *				do_svrctl				     *
 *===========================================================================*/
int do_svrctl()
{
  int s, req;
  vir_bytes ptr;
  req = m_in.svrctl_req;
  ptr = (vir_bytes) m_in.svrctl_argp;

  /* Is the request indeed for the PM_PROC_NR? */
  if (((req >> 8) & 0xFF) != 'M') return(-EINVAL);

  /* Control operations local to the PM. */
  switch(req) {
  case MMGETPARAM: {
      struct sysgetenv sysgetenv;
      char search_key[64];
      char val_start[64];
      char *value;
      size_t val_len;
      size_t copy_len;

      /* Copy sysgetenv structure to PM. */
      if (sys_datacopy(who_e, ptr, ENDPT_SELF, (vir_bytes) &sysgetenv, 
              sizeof(sysgetenv)) != 0) return(-EFAULT);  

      if (sysgetenv.keylen == 0) {	/* copy all parameters */
          value = cmd_line_params;
          val_len = sizeof(cmd_line_params);
      } else {				/* lookup value for key */
          /* Try to get a copy of the requested key. */
          if (sysgetenv.keylen > sizeof(search_key)) return(-EINVAL);
          if ((s = sys_datacopy(who_e, (vir_bytes) sysgetenv.key,
                  ENDPT_SELF, (vir_bytes) search_key, sysgetenv.keylen)) != 0)
              return(s);

          /* Make sure key is null-terminated and lookup value.
           * First check local overrides.
           */
          search_key[sysgetenv.keylen-1]= '\0';

          if (!parse_bootparam_value(cmd_line_params, search_key, val_start))
               return(-ESRCH);
          val_len = strlen(val_start) + 1;
          value = val_start;
      }

      /* See if it fits in the client's buffer. */
      if (val_len > sysgetenv.vallen)
      	return -E2BIG;

      /* Value found, make the actual copy (as far as possible). */
      copy_len = MIN(val_len, sysgetenv.vallen); 
      if ((s=sys_datacopy(ENDPT_SELF, (vir_bytes) value,
              who_e, (vir_bytes) sysgetenv.val, copy_len)) != 0)
          return(s);

      return 0;
  }

  default:
	return(-EINVAL);
  }
}

/*===========================================================================*
 *				brk				             *
 *===========================================================================*/

extern void *__curbrk;
/* redefine brk */
int brk(brk_addr)
void *brk_addr;
{
	int r;
/* PM wants to call brk() itself. */
	if((r=vm_brk(PM_PROC_NR, brk_addr)) != 0) {
#if 0
		printk("PM: own brk(%p) failed: vm_brk() returned %d\n",
			brk_addr, r);
#endif
		return -1;
	}
	__curbrk = brk_addr;
	return 0;
}

/*===========================================================================*
 *				getpciinfo				     *
 *===========================================================================*/

static int getpciinfo(pciinfo)
struct pciinfo *pciinfo;
{
	int devind, r;
	struct pciinfo_entry *entry;
	char *name;
	u16_t vid, did;

	/* look up PCI process number */
	pci_init();

	/* start enumerating devices */
	entry = pciinfo->pi_entries;
	r = pci_first_dev(&devind, &vid, &did);
	while (r)
	{
		/* fetch device name */
		name = pci_dev_name(vid, did);
		if (!name)
			name = "";

		/* store device information in table */
		assert((char *) entry < (char *) (pciinfo + 1));
		entry->pie_vid = vid;
		entry->pie_did = did;
		strncpy(entry->pie_name, name, sizeof(entry->pie_name));
		entry->pie_name[sizeof(entry->pie_name) - 1] = 0;
		entry++;
		
		/* continue with the next device */
		r = pci_next_dev(&devind, &vid, &did);
	}
	
	/* store number of entries */
	pciinfo->pi_count = entry - pciinfo->pi_entries;
	return 0;
}

/*
 * To avoid negative return values, "getpriority()" will
 * not return the normal nice-value, but a negated value that
 * has been offset by 20 (i.e. it returns 40..1 instead of -20..19)
 * to stay compatible.
 */
int scall_getpriority(void)
{
	int arg_which, arg_who;
	int retval = -ESRCH;
	int niceval;
	struct mproc *rmp;

	arg_which = m_in.m_data1;
	arg_who = m_in.m_data2;

	switch (arg_which) {
	case PRIO_PROCESS: /* Only support PRIO_PROCESS for now. */
		if (arg_who)
			rmp = find_proc(arg_who);
		else
			rmp = mp; /* current process */

		if (rmp) {
			niceval = PRIO_MIN - rmp->mp_nice;
			if (niceval > retval)
				retval = niceval;
		}

		break;

	default:
		return -EINVAL;
	}

	if (mp->mp_effuid != SUPER_USER && mp->mp_effuid != rmp->mp_effuid &&
	    mp->mp_effuid != rmp->mp_realuid)
		return -EPERM;

	return retval;
}

int scall_setpriority(void)
{
	int r, which, who, niceval;
	int error = -EINVAL;
	struct mproc *rmp;

	which = m_in.m_data1;
	who = m_in.m_data2;
	niceval = m_in.m_data3;

	/* @nucleos: Only PRIO_PROCESS is supported */
	if (which > PRIO_USER || which < PRIO_PROCESS)
		goto out;

	/* normalize: avoid signed division (rounding problems) */
	error = -ESRCH;
	if (niceval < -20)
		niceval = -20;
	if (niceval > 19)
		niceval = 19;

	switch (which) {
	case PRIO_PROCESS:
		if (who)
			rmp = find_proc(who);
		else
			rmp = mp; /* current process */

		if (rmp != 0)
			error = 0;
		break;

	default:
		return -EINVAL;
	}

	if (mp->mp_effuid != SUPER_USER && mp->mp_effuid != rmp->mp_effuid &&
	    mp->mp_effuid != rmp->mp_realuid)
		return -EPERM;

	/* Only root is allowed to reduce the nice level. */
	if (rmp->mp_nice > niceval && mp->mp_effuid != SUPER_USER)
		return -EACCES;
	
	if ((r = sys_nice(rmp->mp_endpoint, niceval)) != 0)
		return r;

	rmp->mp_nice = niceval;

out:
	return error;
}

#define p_utsbuf	m_data4

int scall_uname(void)
{
	int err;

	err = sys_vircopy(ENDPT_SELF, D, (vir_bytes)&uts_val,
			  mp->mp_endpoint, D, (vir_bytes)m_in.p_utsbuf,
			  sizeof(struct utsname));

	if (err != 0)
		return -EFAULT;

	return 0;
}
