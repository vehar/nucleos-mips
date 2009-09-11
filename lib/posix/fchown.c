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
#include <nucleos/string.h>
#include <nucleos/unistd.h>

int fchown(int fd, uid_t owner, gid_t grp)
{
  message m;

  m.m1_i1 = fd;
  m.m1_i2 = owner;
  m.m1_i3 = grp;
  return(_syscall(FS_PROC_NR, __NR_fchown, &m));
}