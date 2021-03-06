/*
 *  Copyright (C) 2012  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/* The system call implemented in this file:
 *   m_type:	SYS_IOPENABLE
 *
 * The parameters for this system call are:
 *    m_data2:	IO_ENDPT	(process to give I/O Protection Level bits)
 *
 * Author:
 *    Jorrit N. Herder <jnherder@cs.vu.nl>
 */

#include <kernel/system.h>
#include <kernel/kernel.h>

#include <kernel/proto.h>

/*===========================================================================*
 *			        do_iopenable				     *
 *===========================================================================*/
int do_iopenable(m_ptr)
register kipc_msg_t *m_ptr;	/* pointer to request message */
{
  int proc_nr;

#if 1 /* ENABLE_USERPRIV && ENABLE_USERIOPL */
  if (m_ptr->IO_ENDPT == ENDPT_SELF) {
	proc_nr = who_p;
  } else if(!isokendpt(m_ptr->IO_ENDPT, &proc_nr))
	return(-EINVAL);
  enable_iop(proc_addr(proc_nr));
  return 0;
#else
  return(-EPERM);
#endif
}


