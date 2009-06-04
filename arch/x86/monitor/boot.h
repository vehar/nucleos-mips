/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
#ifndef __ARCH_X86_BOOT_BOOT_H
#define __ARCH_X86_BOOT_BOOT_H

#ifndef DEBUG
#define DEBUG 0
#endif

/* Constants describing the metal: */

#define SECTOR_SIZE	512
#define SECTOR_SHIFT	9
#define RATIO(b)		((b) / SECTOR_SIZE)

#define PARAMSEC	1	/* Sector containing boot parameters. */

#define DSKBASE		0x1E	/* Floppy disk parameter vector. */
#define DSKPARSIZE	11	/* There are this many bytes of parameters. */

#define ESC		'\33'	/* Escape key. */

#define HEADERPOS      0x00600L /* Place for an array of struct exec's. */

#define FREEPOS	       0x08000L /* Memory from FREEPOS to caddr is free to
                                 * play with. */
#define MSEC_PER_TICK        55 /* Clock does 18.2 ticks per second. */
#define TICKS_PER_DAY 0x1800B0L /* After 24 hours it wraps. */

#define BOOTPOS	       0x07C00L	/* Bootstraps are loaded here. */
#define SIGNATURE	0xAA55	/* Proper bootstraps have this signature. */
#define SIGNATOFF	510	/* Offset within bootblock. */

/* BIOS video modes. */
#define MONO_MODE	0x07	/* 80x25 monochrome. */
#define COLOR_MODE	0x03	/* 80x25 color. */

typedef struct MNX(vector) { /* 8086 vector */
  MNX(u16_t) offset;
  MNX(u16_t) segment;
} vector;

vector rem_part;		/* Boot partition table entry. */

MNX(u32_t) caddr, daddr; /* Code and data address of the boot program. */
MNX(u32_t) runsize;   /* Size of this program. */

MNX(u16_t) device;    /* Drive being booted from. */

typedef struct {		/* One chunk of free memory. */
	MNX(u32_t)	base;		/* Start byte. */
	MNX(u32_t)	size;		/* Number of bytes. */
} memory;

memory mem[3];		/* List of available memory. */
int mon_return;		/* Monitor stays in memory? */

typedef struct bios_env
{
  MNX(u16_t) ax;
  MNX(u16_t) bx;
  MNX(u16_t) cx;
  MNX(u16_t) flags;
} bios_env_t;

#define FL_CARRY	0x0001	/* carry flag */

/* Functions defined by boothead.s: */

/* Exit the monitor. */
void exit(int code);

/* Local monitor address to absolute address. */
MNX(u32_t) mon2abs(void *ptr);

/* Vector to absolute address. */
MNX(u32_t) vec2abs(vector *vec);

#define MAX_RAWCOPY_ADDR 0xffffff

/* Copy bytes from anywhere up to MAX_RAWCOPY_ADDR */
void raw_copy(MNX(u32_t) dstaddr, MNX(u32_t) srcaddr, MNX(u32_t) count);

#define MAX_GETWORD_ADDR 0xfffff

/* Get a word from somewhere in the first 1MB of memory */
MNX(u16_t) get_word(MNX(u32_t) addr);

/* Put a word anywhere. */
void put_word(MNX(u32_t) addr, MNX(U16_t) word);

/* Switch to a copy of this program. */
void relocate(void);

/* Open device and determine params / close device. */
int dev_open(void), dev_close(void);

 /* True if sector is on a track boundary. */
int dev_boundary(MNX(u32_t) sector);

/* Read 1 or more sectors from "device". */
int readsectors(MNX(u32_t) bufaddr, MNX(u32_t) sector, MNX(U8_t) count);

/* Write 1 or more sectors to "device". */
int writesectors(MNX(u32_t) bufaddr, MNX(u32_t) sector, MNX(U8_t) count);

/* Read a keypress. */
int getch(void);

/* Read keypress directly from kb controller. */
void scan_keyboard(void);

/* Undo a keypress. */
void ungetch(int c);

/* True if escape typed. */
int escape(void);

/* Send a character to the screen. */
void putch(int c);

/* Wait for an interrupt. */
void pause(void);

/* Enable copying console I/O to a serial line. */
void serial_init(int line);

/* Set video mode */
void set_mode(unsigned mode);

/* clear the screen. */
void clear_screen(void);

/* System bus type, XT, AT, or MCA. */
MNX(u16_t) get_bus(void);

/* Display type, MDA to VGA. */
MNX(u16_t) get_video(void);

/* Current value of the clock tick counter. */
MNX(u32_t) get_tick(void);

/* Execute a bootstrap routine for a different O.S. */
void bootstrap(int device, struct part_entry *entry);

/* Start Minix. */
void minix(MNX(u32_t) koff, MNX(u32_t) kcs, MNX(u32_t) kds,
           char *bootparams, size_t paramsize, MNX(u32_t) aout);

void int15(bios_env_t *);

/* Shared between boot.c and bootimage.c: */

/* Sticky attributes. */
#define E_SPECIAL  0x01 /* These are known to the program. */
#define E_DEV      0x02 /* The value is a device name. */
#define E_RESERVED 0x04 /* May not be set by user, e.g. 'boot' */
#define E_STICKY   0x07 /* Don't go once set. */

/* Volatile attributes. */
#define E_VAR       0x08 /* Variable */
#define E_FUNCTION  0x10 /* Function definition. */

/* Variables, functions, and commands. */
typedef struct environment {
  struct environment *next;
  char flags;
  char *name;   /* name = value */
  char *arg;    /* name(arg) {value} */
  char *value;
  char *defval; /* Safehouse for default values. */
} environment;

environment *env;/* Lists the environment. */

/* Get/set the value of a variable. */
char *b_value(char *name);
int b_setvar(int flags, char *name, char *value);
void b_unset(char *name);

void parse_code(char *code);	/* Parse boot monitor commands. */

extern int fsok;	/* True if the boot device contains an FS. */
MNX(u32_t) lowsec;	/* Offset to the file system on the boot device. */

/* Called by boot.c: */
void bootminix(void);		/* Load and start a Minix image. */

/* Called by bootimage.c: */
/* Report a read error. */
void readerr(off_t sec, int err);

/* Transform MNX(u32_t) to ASCII at base b or base 10. */
char *ul2a(MNX(u32_t) n, unsigned b), *ul2a10(MNX(u32_t) n);

/* Cheap atol(). */
long a2l(char *a);

/* ASCII to hex. */
unsigned a2x(char *a);

/* Translate a device name to a device number. */
dev_t name2dev(char *name);

/* True for a numeric prefix. */
int numprefix(char *s, char **ps);

/* True for a numeric string. */
int numeric(char *s);

/* Give a descriptive text for some UNIX errors. */
char *unix_err(int err);

/* Run the trailer function. */
int run_trailer(void); 

void readblock(off_t, char *, int);
void delay(char *);
#endif /* __ARCH_X86_BOOT_BOOT_H */