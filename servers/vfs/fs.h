/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/* This is the master header for fs.  It includes some other files
 * and defines the principal constants.
 */
#define _POSIX_SOURCE      1	/* tell headers to include POSIX stuff */
#define _MINIX             1	/* tell headers to include MINIX stuff */
#define _SYSTEM            1	/* tell headers that this is the kernel */

#define DO_SANITYCHECKS	   0

#if DO_SANITYCHECKS
#define SANITYCHECK do { 			\
	if(!check_vrefs() || !check_pipe()) {				\
	   printf("VFS:%s:%d: call_nr %d who_e %d\n", \
			__FILE__, __LINE__, call_nr, who_e); 	\
	   panic(__FILE__, "sanity check failed", NO_NUM);	\
	}							\
} while(0)
#else
#define SANITYCHECK
#endif

/* The following are so basic, all the *.c files get them automatically. */
#include <nucleos/config.h>	/* MUST be first */
#include <ansi.h>		/* MUST be second */
#include <sys/types.h>
#include <nucleos/const.h>
#include <nucleos/type.h>
#include <nucleos/dmap.h>

#include <limits.h>
#include <errno.h>

#include <nucleos/syslib.h>
#include <nucleos/sysutil.h>

#include "const.h"
#include "dmap.h"
#include "proto.h"
#include "glo.h"