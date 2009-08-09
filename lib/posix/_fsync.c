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
#define sync	_sync
#include <unistd.h>

int fsync(int fd)
{
  message m;

  m.m1_i1 = fd;

  return(_syscall(FS, FSYNC, &m));
}
