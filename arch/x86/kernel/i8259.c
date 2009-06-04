/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/* This file contains routines for initializing the 8259 interrupt controller:
 *      put_irq_handler: register an interrupt handler
 *      rm_irq_handler: deregister an interrupt handler
 *      intr_handle:    handle a hardware interrupt
 *      intr_init:      initialize the interrupt controller(s)
 */

#include <kernel/kernel.h>
#include <kernel/proc.h>
#include <kernel/proto.h>
#include <nucleos/com.h>
#include <nucleos/portio.h>
#include <ibm/cpu.h>

#define ICW1_AT         0x11    /* edge triggered, cascade, need ICW4 */
#define ICW1_PC         0x13    /* edge triggered, no cascade, need ICW4 */
#define ICW1_PS         0x19    /* level triggered, cascade, need ICW4 */
#define ICW4_AT_SLAVE   0x01    /* not SFNM, not buffered, normal EOI, 8086 */
#define ICW4_AT_MASTER  0x05    /* not SFNM, not buffered, normal EOI, 8086 */
#define ICW4_PC_SLAVE   0x09    /* not SFNM, buffered, normal EOI, 8086 */
#define ICW4_PC_MASTER  0x0D    /* not SFNM, buffered, normal EOI, 8086 */

#define set_vec(nr, addr)  ((void)0)

/*===========================================================================*
 *				intr_init				     *
 *===========================================================================*/
PUBLIC int intr_init(mine)
int mine;
{
/* Initialize the 8259s, finishing with all interrupts disabled.  This is
 * only done in protected mode, in real mode we don't touch the 8259s, but
 * use the BIOS locations instead.  The flag "mine" is set if the 8259s are
 * to be programmed for MINIX, or to be reset to what the BIOS expects.
 */
  int i;

  intr_disable();

      /* The AT and newer PS/2 have two interrupt controllers, one master,
       * one slaved at IRQ 2.  (We don't have to deal with the PC that
       * has just one controller, because it must run in real mode.)
       */

      outb(INT_CTL, machine.ps_mca ? ICW1_PS : ICW1_AT);
      outb(INT_CTLMASK, mine ? IRQ0_VECTOR : BIOS_IRQ0_VEC);
                                                        /* ICW2 for master */
      outb( INT_CTLMASK, (1 << CASCADE_IRQ));
					/* ICW3 tells slaves */
      outb(INT_CTLMASK, ICW4_AT_MASTER);
      outb(INT_CTLMASK, ~(1 << CASCADE_IRQ));           /* IRQ 0-7 mask */
      outb(INT2_CTL, machine.ps_mca ? ICW1_PS : ICW1_AT);
      outb(INT2_CTLMASK, mine ? IRQ8_VECTOR : BIOS_IRQ8_VEC);
                                                        /* ICW2 for slave */
      outb(INT2_CTLMASK, CASCADE_IRQ);          /* ICW3 is slave nr */
      outb(INT2_CTLMASK, ICW4_AT_SLAVE);
      outb(INT2_CTLMASK, ~0);                           /* IRQ 8-15 mask */

      /* Copy the BIOS vectors from the BIOS to the Minix location, so we
       * can still make BIOS calls without reprogramming the i8259s.
       */
#if IRQ0_VECTOR != BIOS_IRQ0_VEC
      phys_copy(BIOS_VECTOR(0) * 4L, VECTOR(0) * 4L, 8 * 4L);
#endif
#if IRQ8_VECTOR != BIOS_IRQ8_VEC
      phys_copy(BIOS_VECTOR(8) * 4L, VECTOR(8) * 4L, 8 * 4L);
#endif

  return OK;
}

/*===========================================================================*
 *				intr_disabled			     	     *
 *===========================================================================*/
PUBLIC int intr_disabled(void)
{
	if(!(read_cpu_flags() & X86_FLAG_I))
		return 1;
	return 0;
}