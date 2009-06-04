/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/*
posix/_tcdrain.c

Created:	July 26, 1994 by Philip Homburg
*/

#define tcdrain _tcdrain
#define ioctl _ioctl
#include <termios.h>
#include <sys/ioctl.h>

int tcdrain(fd)
int fd;
{
  return(ioctl(fd, TCDRAIN, (void *)0));
}