/*
 *  Copyright (C) 2012  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/*
pci_attr_r8.c
*/
#include <nucleos/syslib.h>
#include <nucleos/sysutil.h>
#include <nucleos/pci.h>

/*===========================================================================*
 *				pci_attr_r8				     *
 *===========================================================================*/
u8_t pci_attr_r8(devind, port)
int devind;
int port;
{
	int r;
	kipc_msg_t m;

	m.m_type= BUSC_PCI_ATTR_R8;
	m.m_data1= devind;
	m.m_data2= port;

	r= kipc_module_call(KIPC_SENDREC, 0, pci_procnr, &m);
	if (r != 0)
		panic("syslib/" __FILE__, "pci_attr_r8: can't talk to PCI", r);

	if (m.m_type != 0)
		panic("syslib/" __FILE__, "pci_attr_r8: got bad reply from PCI", m.m_type);

	return m.m_data4;
}

