/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
#include <nucleos/lib.h>
#include <nucleos/stat.h>
#include <nucleos/string.h>

int lstat(const char *name, struct stat *buffer)
{
  message m;
  int r;

  m.m1_i1 = strlen(name) + 1;
  m.m1_p1 = (char *) name;
  m.m1_p2 = (char *) buffer;
  if((r = _syscall(FS_PROC_NR, __NR_lstat, &m)) >= 0 || errno != ENOSYS)
     return r;
  return stat(name, buffer);
}
