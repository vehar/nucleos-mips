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
#define mkfifo	_mkfifo
#define mknod	_mknod
#include <sys/stat.h>
#include <unistd.h>

PUBLIC int mkfifo(name, mode)
_CONST char *name;
_mnx_Mode_t mode;
{
  return mknod(name, mode | S_IFIFO, (Dev_t) 0);
}