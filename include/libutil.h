/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
#ifndef _LIBUTIL_H
#define _LIBUTIL_H 1

#include <termios.h>

int openpty(int *, int *, char *, struct termios *, struct winsize *);

#endif