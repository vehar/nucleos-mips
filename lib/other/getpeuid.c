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
#include <unistd.h>

PUBLIC uid_t getpeuid(ep)
endpoint_t ep;
{
  message m;

  m.m1_i1= ep;
  if (_syscall(MM, GETPUID, &m) < 0) return ( (uid_t) -1);
  return( (uid_t) m.m2_i1);
}