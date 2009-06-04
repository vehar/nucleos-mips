/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
#ifndef SRC_H
#define SRC_H

#include "es1371.h"
#include "wait.h"
#include "pci_helper.h"

_PROTOTYPE( int src_init, (DEV_STRUCT * DSP) );
_PROTOTYPE( void src_set_rate, (DEV_STRUCT * DSP, char src_base, u16_t rate) );

#define SRC_SYNTH_BASE      0x70
#define SRC_DAC_BASE        0x74
#define SRC_ADC_BASE        0x78

#define SRC_BUSY_BIT        23

#define SRC_RAM_WE		0x01000000
#define SRC_RAM_BUSY	0x00800000
#define SRC_DISABLE		0x00400000
#define DIS_P1			0x00200000
#define DIS_P2			0x00100000
#define DIS_REC			0x00080000

#define SRC_CTLMASK		(DIS_REC|DIS_P2|DIS_P1|SRC_DISABLE)

#endif /* SRC_H */