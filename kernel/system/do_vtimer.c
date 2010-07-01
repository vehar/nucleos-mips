/*
 *  Copyright (C) 2010  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/* Copyright (c) 2009 Vrije Universiteit, Amsterdam.
 * See the copyright notice in file LICENSE.minix3.
 */
/* The kernel call implemented in this file:
 *   m_type:	SYS_VTIMER
 *
 * The parameters for this kernel call are:
 *    m_data1:	VT_WHICH		(the timer: VT_VIRTUAL or VT_PROF)
 *    m_data2:	VT_SET			(whether to set, or just retrieve)
 *    m_data4:	VT_VALUE		(new/old expiration time, in ticks)
 *    m_data5:	VT_ENDPT		(process to which the timer belongs)
 */

#include <kernel/system.h>

#include <nucleos/signal.h>
#include <nucleos/endpoint.h>

#if USE_VTIMER

/*===========================================================================*
 *				do_vtimer				     *
 *===========================================================================*/
int do_vtimer(m_ptr)
kipc_msg_t *m_ptr;			/* pointer to request message */
{
/* Set and/or retrieve the value of one of a process' virtual timers. */
  struct proc *rrp;		/* pointer to requesting process */
  struct proc *rp;		/* pointer to process the timer belongs to */
  register int pt_flag;		/* the misc on/off flag for the req.d timer */
  register clock_t *pt_left;	/* pointer to the process' ticks-left field */ 
  clock_t old_value;		/* the previous number of ticks left */
  int proc_nr, proc_nr_e;

  /* The requesting process must be privileged. */
  rrp = proc_addr(who_p);
  if (! (priv(rrp)->s_flags & SYS_PROC)) return(-EPERM);

  if (m_ptr->VT_WHICH != VT_VIRTUAL && m_ptr->VT_WHICH != VT_PROF)
      return(-EINVAL);

  /* The target process must be valid. */
  proc_nr_e = (m_ptr->VT_ENDPT == ENDPT_SELF) ? m_ptr->m_source : m_ptr->VT_ENDPT;
  if (!isokendpt(proc_nr_e, &proc_nr)) return(-EINVAL);
  rp = proc_addr(proc_nr);

  /* Determine which flag and which field in the proc structure we want to
   * retrieve and/or modify. This saves us having to differentiate between
   * VT_VIRTUAL and VT_PROF multiple times below.
   */
  if (m_ptr->VT_WHICH == VT_VIRTUAL) {
      pt_flag = MF_VIRT_TIMER;
      pt_left = &rp->p_virt_left;
  } else { /* VT_PROF */
      pt_flag = MF_PROF_TIMER;
      pt_left = &rp->p_prof_left;
  }

  /* Retrieve the old value. */
  if (rp->p_misc_flags & pt_flag) {
      old_value = *pt_left;

      if (old_value < 0) old_value = 0;
  } else {
      old_value = 0;
  }

  /* Set the new value, if requested. This is called from the system task, so
   * we can be interrupted by the clock interrupt, but not by the clock task.
   * Therefore we only have to protect against interference from clock.c's
   * clock_handler(). We can do this without disabling interrupts, by removing
   * the timer's flag before changing the ticks-left field; in that case the
   * clock interrupt will not touch the latter anymore.
   */
  if (m_ptr->VT_SET) {
      rp->p_misc_flags &= ~pt_flag;	/* disable virtual timer */

      if (m_ptr->VT_VALUE > 0) {
          *pt_left = m_ptr->VT_VALUE;	/* set new timer value */
          rp->p_misc_flags |= pt_flag;	/* (re)enable virtual timer */
      } else {
          *pt_left = 0;			/* clear timer value */
      }
  }

  m_ptr->VT_VALUE = old_value;

  return 0;
}

#endif /* USE_VTIMER */

/*===========================================================================*
 *				vtimer_check				     *
 *===========================================================================*/
void vtimer_check(rp)
struct proc *rp;			/* pointer to the process */
{
  /* This is called from the clock task, so we can be interrupted by the clock
   * interrupt, but not by the system task. Therefore we only have to protect
   * against interference from the clock handler. We can safely perform the
   * following actions without locking as well though, as the clock handler
   * never alters p_misc_flags, and only decreases p_virt_left/p_prof_left.
   */

  /* Check if the virtual timer expired. If so, send a SIGVTALRM signal. */
  if ((rp->p_misc_flags & MF_VIRT_TIMER) && rp->p_virt_left <= 0) {
      rp->p_misc_flags &= ~MF_VIRT_TIMER;
      rp->p_virt_left = 0;
      cause_sig(rp->p_nr, SIGVTALRM);
  }

  /* Check if the profile timer expired. If so, send a SIGPROF signal. */
  if ((rp->p_misc_flags & MF_PROF_TIMER) && rp->p_prof_left <= 0) {
      rp->p_misc_flags &= ~MF_PROF_TIMER;
      rp->p_prof_left = 0;
      cause_sig(rp->p_nr, SIGPROF);
  }
}
