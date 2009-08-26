/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/* The kernel call implemented in this file:
 *   m_type:	SYS_NICE
 *
 * The parameters for this kernel call are:
 *    m1_i1:	PR_ENDPT   	process number to change priority
 *    m1_i2:	PR_PRIORITY	the new priority
 */

#include <kernel/system.h>
#include <nucleos/type.h>
#include <sys/resource.h>

#if USE_NICE

/*===========================================================================*
 *				  do_nice				     *
 *===========================================================================*/
int do_nice(message *m_ptr)
{
/* Change process priority or stop the process. */
  int proc_nr, pri, new_q ;
  register struct proc *rp;

  /* Extract the message parameters and do sanity checking. */
  if(!isokendpt(m_ptr->PR_ENDPT, &proc_nr)) return -EINVAL;
  if (iskerneln(proc_nr)) return(-EPERM);
  pri = m_ptr->PR_PRIORITY;
  rp = proc_addr(proc_nr);

  if (pri == PRIO_STOP) {
      /* Take process off the scheduling queues. */
      RTS_LOCK_SET(rp, NO_PRIORITY);
      return 0;
  }
  else if (pri >= PRIO_MIN && pri <= PRIO_MAX) {

      /* The value passed in is currently between PRIO_MIN and PRIO_MAX. 
       * We have to scale this between MIN_USER_Q and MAX_USER_Q to match 
       * the kernel's scheduling queues.
       */
      new_q = MAX_USER_Q + (pri-PRIO_MIN) * (MIN_USER_Q-MAX_USER_Q+1) / 
          (PRIO_MAX-PRIO_MIN+1);
      if (new_q < MAX_USER_Q) new_q = MAX_USER_Q;	/* shouldn't happen */
      if (new_q > MIN_USER_Q) new_q = MIN_USER_Q;	/* shouldn't happen */

      /* Make sure the process is not running while changing its priority. 
       * Put the process back in its new queue if it is runnable.
       */
      RTS_LOCK_SET(rp, NO_PRIORITY);
      rp->p_max_priority = rp->p_priority = new_q;
      RTS_LOCK_UNSET(rp, NO_PRIORITY);

      return 0;
  }
  return(-EINVAL);
}

#endif /* USE_NICE */

