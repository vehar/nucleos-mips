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
 *   m_type:	SYS_EXEC
 *
 * The parameters for this kernel call are:
 *    m1_i1:	PR_ENDPT  		(process that did exec call)
 *    m1_p1:	PR_STACK_PTR		(new stack pointer)
 *    m1_p2:	PR_NAME_PTR		(pointer to program name)
 *    m1_p3:	PR_IP_PTR		(new instruction pointer)
 */
#include <kernel/system.h>
#include <string.h>
#include <signal.h>
#include <nucleos/endpoint.h>

#if USE_EXEC

/*===========================================================================*
 *				do_exec					     *
 *===========================================================================*/
int do_exec(m_ptr)
register message *m_ptr;	/* pointer to request message */
{
/* Handle sys_exec().  A process has done a successful EXEC. Patch it up. */
  register struct proc *rp;
  phys_bytes phys_name;
  char *np;
  int proc;

  if(!isokendpt(m_ptr->PR_ENDPT, &proc))
	return -EINVAL;

  rp = proc_addr(proc);

  /* Save command name for debugging, ps(1) output, etc. */
  if(data_copy(who_e, (vir_bytes) m_ptr->PR_NAME_PTR,
	SYSTEM, (vir_bytes) rp->p_name, (phys_bytes) P_NAME_LEN - 1) != 0)
  	strncpy(rp->p_name, "<unset>", P_NAME_LEN);

  /* Do architecture-specific exec() stuff. */
  arch_pre_exec(rp, (u32_t) m_ptr->PR_IP_PTR, (u32_t) m_ptr->PR_STACK_PTR);

  /* No reply to EXEC call */
  RTS_LOCK_UNSET(rp, RECEIVING);

  return 0;
}
#endif /* USE_EXEC */

