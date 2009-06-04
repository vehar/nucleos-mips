/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
#ifndef AK4531_H
#define AK4531_H
/* best viewed with tabsize=4 */

#include "../../drivers.h"
#include <nucleos/sound.h>

_PROTOTYPE( int ak4531_init, (u16_t base, u16_t status_reg, u16_t bit, 
			u16_t poll) );
_PROTOTYPE( int ak4531_get_set_volume, (struct volume_level *level, int flag) );

#endif