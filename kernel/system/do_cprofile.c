/*
 *  Copyright (C) 2012  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/* The kernel call that is implemented in this file:
 *   m_type:    SYS_CPROFILE
 *
 * The parameters for this kernel call are:
 *    m_data1:    PROF_ACTION       (get/reset profiling data)
 *    m_data2:    PROF_MEM_SIZE     (available memory for data)
 *    m_data4:    PROF_ENDPT        (endpoint of caller)
 *    m_data5:    PROF_CTL_PTR      (location of info struct)
 *    m_data6:    PROF_MEM_PTR      (location of memory for data)
 *
 * Changes:
 *   14 Aug, 2006   Created (Rogier Meurs)
 */
#include <nucleos/kernel.h>
#include <nucleos/signal.h>
#include <kernel/system.h>
#include <nucleos/string.h>

int cprof_mem_size;              /* available user memory for data */
struct cprof_info_s cprof_info;  /* profiling info for user program */
int cprof_procs_no;              /* number of profiled processes */
struct cprof_proc_info_s cprof_proc_info[NR_SYS_PROCS];

/*===========================================================================*
 *				do_cprofile				     *
 *===========================================================================*/
int do_cprofile(m_ptr)
register kipc_msg_t *m_ptr;    /* pointer to request message */
{
  int proc_nr, i, err = 0, k = 0;
  phys_bytes len;
  vir_bytes vir_dst, vir_src;

  switch (m_ptr->PROF_ACTION) {

  case PROF_RESET:

	/* Reset profiling tables. */

	cprof_ctl_inst.reset = 1;

	printk("CPROFILE notice: resetting tables:");

	for (i=0; i<cprof_procs_no; i++) {

		printk(" %s", cprof_proc_info[i].name);

		/* Test whether proc still alive. */
		if (!isokendpt(cprof_proc_info[i].endpt, &proc_nr)) {
			printk("endpt not valid %u (%s)\n",
			cprof_proc_info[i].endpt, cprof_proc_info[i].name);
			continue;
		}

		/* Set reset flag. */
		data_copy(SYSTEM, (vir_bytes) &cprof_ctl_inst.reset,
			cprof_proc_info[i].endpt, cprof_proc_info[i].ctl_v,
			sizeof(cprof_ctl_inst.reset));
	}
	printk("\n");
	
	return 0;

  case PROF_GET:

	/* Get profiling data.
	 *
	 * Calculate physical addresses of user pointers.  Copy to user
	 * program the info struct.  Copy to user program the profiling
	 * tables of the profiled processes.
	 */

	if(!isokendpt(m_ptr->PROF_ENDPT, &proc_nr))
		return -EINVAL;

	cprof_mem_size = m_ptr->PROF_MEM_SIZE;

	printk("CPROFILE notice: getting tables:");

	/* Copy control structs of profiled processes to calculate total
	 * nr of bytes to be copied to user program and find out if any
	 * errors happened. */
	cprof_info.mem_used = 0;
	cprof_info.err = 0;

	for (i=0; i<cprof_procs_no; i++) {

		printk(" %s", cprof_proc_info[i].name);

		/* Test whether proc still alive. */
		if (!isokendpt(cprof_proc_info[i].endpt, &proc_nr)) {
			printk("endpt not valid %u (%s)\n",
			cprof_proc_info[i].endpt, cprof_proc_info[i].name);
			continue;
		}

		/* Copy control struct from proc to local variable. */
		data_copy(cprof_proc_info[i].endpt, cprof_proc_info[i].ctl_v,
			SYSTEM, (vir_bytes) &cprof_ctl_inst,
			sizeof(cprof_ctl_inst));

	       	/* Calculate memory used. */
		cprof_proc_info[i].slots_used = cprof_ctl_inst.slots_used;
		cprof_info.mem_used += CPROF_PROCNAME_LEN;
		cprof_info.mem_used += sizeof(cprof_proc_info_inst.slots_used);
		cprof_info.mem_used += cprof_proc_info[i].slots_used *
						sizeof(cprof_tbl_inst);
		/* Collect errors. */
		cprof_info.err |= cprof_ctl_inst.err;
	}
	printk("\n");

	/* Do we have the space available? */
	if (cprof_mem_size < cprof_info.mem_used) cprof_info.mem_used = -1;

	/* Copy the info struct to the user process. */
	data_copy(SYSTEM, (vir_bytes) &cprof_info,
		m_ptr->PROF_ENDPT, (vir_bytes) m_ptr->PROF_CTL_PTR,
		sizeof(cprof_info));

	/* If there is no space or errors occurred, don't bother copying. */
	if (cprof_info.mem_used == -1 || cprof_info.err) return 0;

	/* For each profiled process, copy its name, slots_used and profiling
	 * table to the user process. */
	vir_dst = (vir_bytes) m_ptr->PROF_MEM_PTR;
	for (i=0; i<cprof_procs_no; i++) {
		len = (phys_bytes) strlen(cprof_proc_info[i].name);
		data_copy(SYSTEM, (vir_bytes) cprof_proc_info[i].name,
			m_ptr->PROF_ENDPT, vir_dst, len);
		vir_dst += CPROF_PROCNAME_LEN;

		len = (phys_bytes) sizeof(cprof_ctl_inst.slots_used);
		data_copy(cprof_proc_info[i].endpt,
		  cprof_proc_info[i].ctl_v + sizeof(cprof_ctl_inst.reset),
		  m_ptr->PROF_ENDPT, vir_dst, len);
		vir_dst += len;

		len = (phys_bytes)
		(sizeof(cprof_tbl_inst) * cprof_proc_info[i].slots_used);
		data_copy(cprof_proc_info[i].endpt, cprof_proc_info[i].buf_v,
		  m_ptr->PROF_ENDPT, vir_dst, len);
		vir_dst += len;
	}

	return 0;

  default:
	return -EINVAL;
  }
}

