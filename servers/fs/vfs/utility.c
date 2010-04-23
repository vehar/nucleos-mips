/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/* This file contains a few general purpose utility routines.
 *
 * The entry points into this file are
 *   clock_time:  ask the clock task for the real time
 *   copy:	  copy a block of data
 *   fetch_name:  go get a path name from user space
 *   no_sys:      reject a system call that FS does not handle
 *   panic:       something awful has occurred;  MINIX cannot continue
 *   conv2:	  do byte swapping on a 16-bit int
 *   conv4:	  do byte swapping on a 32-bit long
 */
#include "fs.h"
#include <nucleos/com.h>
#include <nucleos/endpoint.h>
#include <nucleos/unistd.h>
#include <nucleos/string.h>
#include <stdlib.h>
#include <assert.h>
#include <servers/vfs/fproc.h>
#include <asm/servers/vm/memory.h>

#include "file.h"
#include "param.h"
#include "vmnt.h"

/**
 * Fetch name
 * @path  pointer to the path in user space
 */
int fetch_name(char *path)
{
	/* Go get path and put it in 'user_fullpath'. Copy it from user space. */
	int ret;
	int len = (VM_STACKTOP - (unsigned long)path);

	len = (len < PATH_MAX) ? len : PATH_MAX;
	len = (len <= 0) ? PATH_MAX : len;

	if(len >= sizeof(user_fullpath))
		panic(__FILE__, "fetch_name: len too much for user_fullpath", len);

	memset(user_fullpath, 0, sizeof(user_fullpath));

	/* String is not contained in the message.  Get it from user space. */
	ret = sys_datacopy(who_e, (vir_bytes)path,
		FS_PROC_NR, (vir_bytes)user_fullpath, (phys_bytes)len);

	len = strnlen(user_fullpath, len) + 1;

	if (len > PATH_MAX) {
		err_code = -ENAMETOOLONG;
		return err_code;
	}

	/* Check name length for validity. */
	if (len <= 0) {
		err_code = -EINVAL;
		return err_code;
	}

	if (user_fullpath[len-1] != '\0') {
		err_code = -ENAMETOOLONG;
		return err_code;
	}

	return ret;
}

/*===========================================================================*
 *				no_sys					     *
 *===========================================================================*/
int no_sys()
{
/* Somebody has used an illegal system call number */
  printf("VFS no_sys: call %d from %d (pid %d)\n", call_nr, who_e, who_p);
  return(-ENOSYS);
}

/*===========================================================================*
 *				isokendpt_f				     *
 *===========================================================================*/
int isokendpt_f(char *file, int line, int endpoint, int *proc, int fatal)
{
    int failed = 0;
    endpoint_t ke;
    *proc = _ENDPOINT_P(endpoint);
    if(endpoint == ENDPT_NONE) {
        printf("vfs:%s: endpoint is ENDPT_NONE\n", file, line, endpoint);
        failed = 1;
    } else if(*proc < 0 || *proc >= NR_PROCS) {
        printf("vfs:%s:%d: proc (%d) from endpoint (%d) out of range\n",
                file, line, *proc, endpoint);
        failed = 1;
    } else if((ke=fproc[*proc].fp_endpoint) != endpoint) {
	if(ke == ENDPT_NONE) {
        	printf("vfs:%s:%d: endpoint (%d) points to ENDPT_NONE slot (%d)\n",
                	file, line, endpoint, *proc);
		assert(fproc[*proc].fp_pid == PID_FREE);
	} else {
	        printf("vfs:%s:%d: proc (%d) from endpoint (%d) doesn't match "
			"known endpoint (%d)\n", file, line, *proc, endpoint,
			fproc[*proc].fp_endpoint);
		assert(fproc[*proc].fp_pid != PID_FREE);
	}
        failed = 1;
    }

    if(failed && fatal)
        panic(__FILE__, "isokendpt_f failed", NO_NUM);

  return(failed ? -EDEADSRCDST : 0);
}


/*===========================================================================*
 *				clock_time				     *
 *===========================================================================*/
time_t clock_time()
{
/* This routine returns the time in seconds since 1.1.1970.  MINIX is an
 * astrophysically naive system that assumes the earth rotates at a constant
 * rate and that such things as leap seconds do not exist.
 */

  register int r;
  clock_t uptime;
  time_t boottime;

  r= getuptime2(&uptime, &boottime);
  if (r != 0)
	panic(__FILE__,"clock_time err", r);

  return( (time_t) (boottime + (uptime/system_hz)));
}
