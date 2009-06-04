/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
#include <lib.h>
#define getpprocnr	_getpprocnr
#include <unistd.h>


PUBLIC int getpprocnr()
{
  message m;
  m.m1_i1 = -1;			/* don't pass pid to search for */
  m.m1_i2 = 0;			/* don't pass name to search for */
  if (_syscall(PM_PROC_NR, GETPROCNR, &m) < 0) return(-1);
  return(m.m1_i2);		/* return parent process number */
}
