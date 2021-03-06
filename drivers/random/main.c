/*
 *  Copyright (C) 2012  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/* This file contains the device dependent part of the drivers for the
 * following special files:
 *     /dev/random	- random number generator
 */

#include <nucleos/drivers.h>
#include <nucleos/driver.h>
#include <nucleos/type.h>
#include <asm/ioctls.h>

#include "assert.h"
#include "random.h"

#define NR_DEVS            1		/* number of minor devices */
#  define RANDOM_DEV  0			/* minor device for /dev/random */

#define KRANDOM_PERIOD    1 		/* ticks between krandom calls */

static struct device m_geom[NR_DEVS];  /* base and size of each device */
static int m_device;			/* current device */

extern int errno;			/* error number for PM calls */

static char *r_name(void);
static struct device *r_prepare(int device);
static int r_transfer(int proc_nr, int opcode, u64_t position,
				iovec_t *iov, unsigned nr_req);
static int r_do_open(struct driver *dp, kipc_msg_t *m_ptr);
static void r_init(void);
static int r_ioctl(struct driver *dp, kipc_msg_t *m_ptr);
static void r_geometry(struct partition *entry);
static void r_random(struct driver *dp, kipc_msg_t *m_ptr);
static void r_updatebin(int source, struct k_randomness_bin *rb);

/* Entry points to this driver. */
static struct driver r_dtab = {
  r_name,	/* current device's name */
  r_do_open,	/* open or mount */
  do_nop,	/* nothing on a close */
  r_ioctl,	/* specify ram disk geometry */
  r_prepare,	/* prepare for I/O on a given minor device */
  r_transfer,	/* do the I/O */
  nop_cleanup,	/* no need to clean up */
  r_geometry,	/* device "geometry" */
  nop_signal,	/* system signals */
  r_random, 	/* get randomness from kernel (alarm) */
  nop_cancel,
  nop_select,
  NULL,
  NULL
};

/* Buffer for the /dev/random number generator. */
#define RANDOM_BUF_SIZE 		1024
static char random_buf[RANDOM_BUF_SIZE];

/*===========================================================================*
 *				   main 				     *
 *===========================================================================*/
int main(void)
{
  r_init();				/* initialize the memory driver */
  driver_task(&r_dtab, DRIVER_ASYN);	/* start driver's main loop */
  return(0);
}

/*===========================================================================*
 *				 r_name					     *
 *===========================================================================*/
static char *r_name()
{
/* Return a name for the current device. */
  static char name[] = "random";
  return name;  
}

/*===========================================================================*
 *				r_prepare				     *
 *===========================================================================*/
static struct device *r_prepare(device)
int device;
{
/* Prepare for I/O on a device: check if the minor device number is ok. */

  if (device < 0 || device >= NR_DEVS) return(NIL_DEV);
  m_device = device;

  return(&m_geom[device]);
}

/*===========================================================================*
 *				r_transfer				     *
 *===========================================================================*/
static int r_transfer(proc_nr, opcode, position, iov, nr_req)
int proc_nr;			/* process doing the request */
int opcode;			/* DEV_GATHER or DEV_SCATTER */
u64_t position;			/* offset on device to read or write */
iovec_t *iov;			/* pointer to read or write request vector */
unsigned nr_req;		/* length of request vector */
{
/* Read or write one the driver's minor devices. */
  unsigned count, left, chunk;
  vir_bytes user_vir;
  struct device *dv;
  unsigned long dv_size;
  int r;
  size_t vir_offset = 0;

  /* Get minor device number and check for /dev/null. */
  dv = &m_geom[m_device];
  dv_size = cv64ul(dv->dv_size);

  while (nr_req > 0) {

	/* How much to transfer and where to / from. */
	count = iov->iov_size;
	user_vir = iov->iov_addr;

	switch (m_device) {

	/* Random number generator. Character instead of block device. */
	case RANDOM_DEV:
	    if (opcode == DEV_GATHER_S && !random_isseeded())
		    return(-EAGAIN);
	    left = count;
	    while (left > 0) {
	    	chunk = (left > RANDOM_BUF_SIZE) ? RANDOM_BUF_SIZE : left;
 	        if (opcode == DEV_GATHER_S) {
		    random_getbytes(random_buf, chunk);
		    r= sys_safecopyto(proc_nr, user_vir, vir_offset,
			(vir_bytes) random_buf, chunk, D);
		    if (r != 0)
		    {
			printk(
			"random: sys_safecopyto failed for proc %d, grant %d\n",
				proc_nr, user_vir);
			return r;
		    }
 	        } else if (opcode == DEV_SCATTER_S) {
		    r= sys_safecopyfrom(proc_nr, user_vir, vir_offset,
			(vir_bytes) random_buf, chunk, D);
		    if (r != 0)
		    {
			printk(
		"random: sys_safecopyfrom failed for proc %d, grant %d\n",
				proc_nr, user_vir);
			return r;
		    }
	    	    random_putbytes(random_buf, chunk);
 	        }
 	        vir_offset += chunk;
	    	left -= chunk;
	    }
	    break;

	/* Unknown (illegal) minor device. */
	default:
	    return(-EINVAL);
	}

	/* Book the number of bytes transferred. */
	position= add64u(position, count);
  	if ((iov->iov_size -= count) == 0) { iov++; nr_req--; vir_offset = 0; }

  }
  return 0;
}

/*============================================================================*
 *				r_do_open				      *
 *============================================================================*/
static int r_do_open(dp, m_ptr)
struct driver *dp;
kipc_msg_t *m_ptr;
{
/* Check device number on open.  
 */
  if (r_prepare(m_ptr->DEVICE) == NIL_DEV) return(-ENXIO);

  return 0;
}

/*===========================================================================*
 *				r_init					     *
 *===========================================================================*/
static void r_init()
{
  static struct k_randomness krandom;
  int i, s;

  random_init();
  r_random(NULL, NULL);				/* also set periodic timer */

  /* Retrieve first randomness buffer with parameters. */
  if ((s=sys_getrandomness(&krandom)) != 0) {
  	report("RANDOM", "sys_getrandomness failed", s);
	exit(1);
  }

  /* Do sanity check on parameters. */
  if(krandom.random_sources != RANDOM_SOURCES ||
     krandom.random_elements != RANDOM_ELEMENTS) {
     printk("random: parameters (%d, %d) don't match kernel's (%d, %d)\n",
	RANDOM_SOURCES, RANDOM_ELEMENTS,
	krandom.random_sources, krandom.random_elements);
     exit(1);
  }

  /* Feed initial batch. */
  for(i = 0; i < RANDOM_SOURCES; i++)
	r_updatebin(i, &krandom.bin[i]);
}

/*===========================================================================*
 *				r_ioctl					     *
 *===========================================================================*/
static int r_ioctl(dp, m_ptr)
struct driver *dp;			/* pointer to driver structure */
kipc_msg_t *m_ptr;				/* pointer to control message */
{
  struct device *dv;
  if ((dv = r_prepare(m_ptr->DEVICE)) == NIL_DEV) return(-ENXIO);

  switch (m_ptr->REQUEST) {

    default:
  	return(do_diocntl(&r_dtab, m_ptr));
  }
  return 0;
}

#define UPDATE(binnumber, bp, startitem, elems) 	{	\
		rand_t *r, *r2;					\
		int n = elems, item = startitem;\
		int high;					\
		assert(binnumber >= 0 && binnumber < RANDOM_SOURCES);	 \
		assert(item >= 0 && item < RANDOM_ELEMENTS);	\
		if(n > 0) {					\
			high = item+n-1;			\
			assert(high >= item);				\
			assert(high >= 0 && high < RANDOM_ELEMENTS);	\
			r = &bp->r_buf[item];		\
			r2 = &bp->r_buf[high];		\
	  		random_update(binnumber, r, n);			\
		}							\
}

static void r_updatebin(int source, struct k_randomness_bin *rb)
{
  	int r_next, r_size, r_high;

  	r_next= rb->r_next;
  	r_size= rb->r_size;

	assert(r_next >= 0 && r_next < RANDOM_ELEMENTS);
	assert(r_size >= 0 && r_size <= RANDOM_ELEMENTS);

  	r_high= r_next+r_size;

  	if (r_high <= RANDOM_ELEMENTS) {
		UPDATE(source, rb, r_next, r_size);
	} else {
		assert(r_next < RANDOM_ELEMENTS);
		UPDATE(source, rb, r_next, RANDOM_ELEMENTS-r_next);
		UPDATE(source, rb, 0, r_high-RANDOM_ELEMENTS);
	}

	return;
}

/*============================================================================*
 *				r_random				      *
 *============================================================================*/
static void r_random(dp, m_ptr)
struct driver *dp;			/* pointer to driver structure */
kipc_msg_t *m_ptr;				/* pointer to alarm message */
{
  /* Fetch random information from the kernel to update /dev/random. */
  int s;
  static int bin = 0;
  static struct k_randomness_bin krandom_bin;
  u32_t hi, lo;
  rand_t r;

  bin = (bin+1) % RANDOM_SOURCES;

  if(sys_getrandom_bin(&krandom_bin, bin) == 0)
	r_updatebin(bin, &krandom_bin);

  /* Add our own timing source. */
  read_tsc(&hi, &lo);
  r = lo;
  random_update(RND_TIMING, &r, 1);

  /* Schedule new alarm for next m_random call. */
  if ((s=sys_setalarm(KRANDOM_PERIOD, 0)) != 0)
  	report("RANDOM", "sys_setalarm failed", s);
}

/*============================================================================*
 *				r_geometry				      *
 *============================================================================*/
static void r_geometry(entry)
struct partition *entry;
{
  /* Memory devices don't have a geometry, but the outside world insists. */
  entry->cylinders = div64u(m_geom[m_device].dv_size, SECTOR_SIZE) / (64 * 32);
  entry->heads = 64;
  entry->sectors = 32;
}

