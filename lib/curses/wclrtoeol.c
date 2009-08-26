/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
#include <curses.h>
#include "curspriv.h"

/****************************************************************/
/* Wclrtoeol() fills the half of the cursor line to the right	*/
/* Of the cursor in window 'win' with blanks.			*/
/****************************************************************/

int wclrtoeol(win)
WINDOW *win;
{
  int *maxx, *ptr, *end, y, x, minx, blank;

  y = win->_cury;
  x = win->_curx;
  blank = ' ' | (win->_attrs & ATR_MSK);

  end = &win->_line[y][win->_maxx];
  minx = _NO_CHANGE;
  maxx = &win->_line[y][x];
  for (ptr = maxx; ptr <= end; ptr++) {
	if (*ptr != blank) {
		maxx = ptr;
		if (minx == _NO_CHANGE) minx = ptr - win->_line[y];
		*ptr = blank;
	}			/* if */
  }				/* for */

  if (minx != _NO_CHANGE) {
	if (win->_minchng[y] > minx || win->_minchng[y] == _NO_CHANGE)
		win->_minchng[y] = minx;
	if (win->_maxchng[y] < maxx - win->_line[y])
		win->_maxchng[y] = maxx - win->_line[y];
  }
  return 0;
}
