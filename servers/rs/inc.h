/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/* Header file for the system service manager server. 
 *
 * Created:
 *    Jul 22, 2005	by Jorrit N. Herder 
 */

#define _SYSTEM            1    /* get OK and negative error codes */
#define _MINIX             1	/* tell headers to include MINIX stuff */

#define VERBOSE		   0	/* display diagnostics */

#include <ansi.h>
#include <sys/types.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include <nucleos/callnr.h>
#include <nucleos/config.h>
#include <nucleos/type.h>
#include <nucleos/const.h>
#include <nucleos/com.h>
#include <nucleos/syslib.h>
#include <nucleos/sysutil.h>
#include <nucleos/keymap.h>
#include <nucleos/bitmap.h>

#include <asm/kernel/types.h>
#include <timers.h>				/* For priv.h */
#include <kernel/priv.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "proto.h"
#include "manager.h"
