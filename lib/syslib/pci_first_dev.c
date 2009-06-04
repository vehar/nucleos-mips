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
pci_first_dev.c
*/

#include "pci.h"
#include "syslib.h"
#include <nucleos/sysutil.h>

/*===========================================================================*
 *				pci_first_dev				     *
 *===========================================================================*/
PUBLIC int pci_first_dev(devindp, vidp, didp)
int *devindp;
u16_t *vidp;
u16_t *didp;
{
	int r;
	message m;

	m.m_type= BUSC_PCI_FIRST_DEV;
	r= sendrec(pci_procnr, &m);
	if (r != 0)
		panic("syslib/" __FILE__, "pci_first_dev: can't talk to PCI", r);
	if (m.m_type == 1)
	{
		*devindp= m.m1_i1;
		*vidp= m.m1_i2;
		*didp= m.m1_i3;
#if DEBUG
		printf("pci_first_dev: got device %d, %04x/%04x\n", 
			*devindp, *vidp, *didp);
#endif
		return 1;
	}
	if (m.m_type != 0)
		panic("syslib/" __FILE__, "pci_first_dev: got bad reply from PCI", m.m_type);

#if DEBUG
	printf("pci_first_dev: got nothing\n");
#endif
	return 0;
}