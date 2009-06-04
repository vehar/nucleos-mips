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
#define readlink _readlink
#include <unistd.h>
#include <string.h>

PUBLIC int readlink(name, buffer, bufsiz)
_CONST char *name;
char *buffer;
size_t bufsiz;
{
  message m;

  m.m1_i1 = strlen(name) + 1;
  m.m1_i2 = bufsiz;
  m.m1_p1 = (char *) name;
  m.m1_p2 = (char *) buffer;

  return(_syscall(FS, RDLNK, &m));
}