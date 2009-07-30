/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/*  rawfs.c - Raw Minix file system support.  Author: Kees J. Bot
 *                23 Dec 1991
 *             Based on readfs by Paul Polderman
 */
#define nil 0
#define _POSIX_SOURCE   1
#define _MINIX    1

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "include/sys/types.h"
#include "include/sys/stat.h"
#include "include/fs/const.h"
#include "include/fs/type.h"
#include "include/fs/buf.h"
#include "include/fs/super.h"
#include "include/fs/inode.h"
#include "include/fs/rawfs.h"

#define DO_TRACE 0

void readblock(MNX(off_t) blockno, char *buf, int);

/* The following code handles two file system types: Version 1 with small
 * inodes and 16-bit disk addresses and Version 2 with big inodes and 32-bit
 * disk addresses.
#ifdef FLEX
 * To make matters worse, Minix-vmd knows about the normal Unix Version 7
 * directories and directories with flexible entries.
#endif
 */

/* File system parameters. */
static unsigned nr_dzones;  /* Fill these in after reading superblock. */
static unsigned nr_indirects;
static unsigned inodes_per_block;
static int block_size;

#ifdef FLEX
#include <dirent.h>
#define direct _v7_direct
#else
#include <sys/dir.h>
#endif

static struct MNX(super_block) super; /* Superblock of file system */
#define SUPER_V1 SUPER_MAGIC          /* V1 magic has a weird name. */

static struct MNX(inode) curfil;      /* Inode of file under examination */
static char indir[_MAX_BLOCK_SIZE];   /* Single indirect block. */
static char dindir[_MAX_BLOCK_SIZE];  /* Double indirect block. */
static char dirbuf[_MAX_BLOCK_SIZE];  /* Scratch/Directory block. */
#define scratch dirbuf

static MNX(block_t) a_indir, a_dindir;  /* Addresses of the indirects. */
static MNX(off_t) dirpos;                    /* Reading pos in a dir. */

#define fsbuf(b)  (* (union MNX(fsdata_u)*) (b))

#define zone_shift  (super.s_log_zone_size) /* zone to block ratio */

MNX(off_t) r_super(int *bs)
/* Initialize variables, return size of file system in blocks,
 * (zero on error).
 */
{
  NUCS_TRACE(DO_TRACE,"\n");
  /* Read superblock. (The superblock is always at 1kB offset,
   * that's why we lie to readblock and say the block size is 1024
   * and we want block number 1 (the 'second block', at offset 1kB).)
   */
  readblock(1, scratch, 1024);

  memcpy(&super, scratch, sizeof(super));

  NUCS_TRACE(DO_TRACE,"magic number=0x%X\n",super.s_magic);

  /* Is it really a MINIX file system ? */
  if (super.s_magic == SUPER_V2 || super.s_magic == SUPER_V3) {
    if(super.s_magic == SUPER_V2)
      super.s_block_size = 1024;

    *bs = block_size = super.s_block_size;
    if(block_size < _MIN_BLOCK_SIZE ||
      block_size > _MAX_BLOCK_SIZE) {
      fprintf(stderr,"%s: Wrong block_size %d\n",__FUNCTION__, block_size);

      return 0;
    }
    nr_dzones= V2_NR_DZONES;
    nr_indirects= V2_INDIRECTS(block_size);
    inodes_per_block= V2_INODES_PER_BLOCK(block_size);
    return (MNX(off_t)) super.s_zones << zone_shift;
  } else
  if (super.s_magic == SUPER_V1) {
    *bs = block_size = 1024;
    nr_dzones= V1_NR_DZONES;
    nr_indirects= V1_INDIRECTS;
    inodes_per_block= V1_INODES_PER_BLOCK;
    return (MNX(off_t)) super.s_nzones << zone_shift;
  } else {
    /* Filesystem not recognized as Minix. */
    return 0;
  }
}

void r_stat(MNX(Ino_t) inum, struct MNX(stat) *stp)
/* Return information about a file like stat(2) and remember it. */
{
  NUCS_TRACE(DO_TRACE,"\n");
  MNX(block_t) block;
  MNX(block_t) ino_block;
  MNX(ino_t) ino_offset;
  union MNX(fsdata_u) *blockbuf;

  /* Calculate start of i-list */
  block = START_BLOCK + super.s_imap_blocks + super.s_zmap_blocks;

  /* Calculate block with inode inum */
  ino_block = ((inum - 1) / inodes_per_block);
  ino_offset = ((inum - 1) % inodes_per_block);
  block += ino_block;

  /* Fetch the block */
  blockbuf = (union MNX(fsdata_u) *) scratch;
  readblock(block, (char*)blockbuf, block_size);

  if (super.s_magic == SUPER_V2 || super.s_magic == SUPER_V3) {
    MNX(d2_inode) *dip;
    int i;

    dip= &blockbuf->b__v2_ino[ino_offset];

    curfil.i_mode= dip->d2_mode;
    curfil.i_nlinks= dip->d2_nlinks;
    curfil.i_uid= dip->d2_uid;
    curfil.i_gid= dip->d2_gid;
    curfil.i_size= dip->d2_size;
    curfil.i_atime= dip->d2_atime;
    curfil.i_mtime= dip->d2_mtime;
    curfil.i_ctime= dip->d2_ctime;
    for (i= 0; i < V2_NR_TZONES; i++)
      curfil.i_zone[i]= dip->d2_zone[i];
  } else {
    MNX(d1_inode) *dip;
    int i;

    dip= &blockbuf->b__v1_ino[ino_offset];

    curfil.i_mode= dip->d1_mode;
    curfil.i_nlinks= dip->d1_nlinks;
    curfil.i_uid= dip->d1_uid;
    curfil.i_gid= dip->d1_gid;
    curfil.i_size= dip->d1_size;
    curfil.i_atime= dip->d1_mtime;
    curfil.i_mtime= dip->d1_mtime;
    curfil.i_ctime= dip->d1_mtime;
    for (i= 0; i < V1_NR_TZONES; i++)
      curfil.i_zone[i]= dip->d1_zone[i];
  }
  curfil.i_dev= -1;   /* Can't fill this in alas. */
  curfil.i_num= inum;

  stp->st_dev= curfil.i_dev;
  stp->st_ino= curfil.i_num;
  stp->st_mode= curfil.i_mode;
  stp->st_nlink= curfil.i_nlinks;
  stp->st_uid= curfil.i_uid;
  stp->st_gid= curfil.i_gid;
  stp->st_rdev= (MNX(dev_t)) curfil.i_zone[0];
  stp->st_size= curfil.i_size;
  stp->st_atime= curfil.i_atime;
  stp->st_mtime= curfil.i_mtime;
  stp->st_ctime= curfil.i_ctime;
  a_indir= a_dindir= 0;
  dirpos= 0;
}

MNX(ino_t) r_readdir(char *name)
/* Read next directory entry at "dirpos" from file "curfil". */
{
//  NUCS_TRACE(DO_TRACE,"\n");
  MNX(ino_t) inum= 0;
  int blkpos;
  struct MNX(direct) *dp;

  if (!S_ISDIR(curfil.i_mode)) { errno= ENOTDIR; return -1; }

  if(!block_size) { errno = 0; return -1; }

  while (inum == 0 && dirpos < curfil.i_size) {
    if ((blkpos= (int) (dirpos % block_size)) == 0) {
      /* Need to fetch a new directory block. */

      readblock(r_vir2abs(dirpos / block_size), dirbuf, block_size);
    }
#ifdef FLEX
    if (super.s_flags & S_FLEX) {
      struct _fl_direct *dp;

      dp= (struct _fl_direct *) (dirbuf + blkpos);
      if ((inum= dp->d_ino) != 0) strcpy(name, dp->d_name);

      dirpos+= (1 + dp->d_extent) * FL_DIR_ENTRY_SIZE;
      continue;
    }
#endif
    /* Let dp point to the next entry. */
    dp= (struct MNX(direct) *) (dirbuf + blkpos);

    if ((inum= dp->d_ino) != 0) {
      /* This entry is occupied, return name. */
      strncpy(name, dp->d_name, sizeof(dp->d_name));
      name[sizeof(dp->d_name)]= 0;
    }
    dirpos+= DIR_ENTRY_SIZE;
  }
  return inum;
}

MNX(off_t) r_vir2abs(MNX(off_t) virblk)
/* Translate a block number in a file to an absolute disk block number.
 * Returns 0 for a hole and -1 if block is past end of file.
 */
{
  NUCS_TRACE(DO_TRACE,"\n");
  MNX(block_t) b= virblk;
  MNX(zone_t) zone, ind_zone;
  MNX(block_t) z, zone_index;
  MNX(u32_t) i;

  if(!block_size) return -1;

  /* Check if virblk within file. */
  if (virblk * block_size >= curfil.i_size) return -1;

  /* Calculate zone in which the datablock number is contained */
  zone = (MNX(zone_t)) (b >> zone_shift);

  /* Calculate index of the block number in the zone */
  zone_index = b - ((MNX(block_t)) zone << zone_shift);

  /* Go get the zone */
  if (zone < (MNX(zone_t)) nr_dzones) {   /* direct block */
    zone = curfil.i_zone[(int) zone];
    z = ((MNX(block_t)) zone << zone_shift) + zone_index;
    return z;
  }

  /* The zone is not a direct one */
  zone -= (MNX(zone_t)) nr_dzones;

  /* Is it single indirect ? */
  if (zone < (MNX(zone_t)) nr_indirects) {  /* single indirect block */
    ind_zone = curfil.i_zone[nr_dzones];
  } else {      /* double indirect block */
    /* Fetch the double indirect block */
    if ((ind_zone = curfil.i_zone[nr_dzones + 1]) == 0) return 0;

    z = (MNX(block_t)) ind_zone << zone_shift;
    if (a_dindir != z) {
      readblock(z, dindir, block_size);
      a_dindir= z;
    }
    /* Extract the indirect zone number from it */
    zone -= (MNX(zone_t)) nr_indirects;

    i = zone / (MNX(zone_t)) nr_indirects;
    ind_zone = (super.s_magic == SUPER_V2 || super.s_magic == SUPER_V3)
        ? fsbuf(dindir).b__v2_ind[i]
        : fsbuf(dindir).b__v1_ind[i];
    zone %= (MNX(zone_t)) nr_indirects;
  }
  if (ind_zone == 0) return 0;

  /* Extract the datablock number from the indirect zone */
  z = (MNX(block_t)) ind_zone << zone_shift;
  if (a_indir != z) {
    readblock(z, indir, block_size);
    a_indir= z;
  }
  zone = (super.s_magic == SUPER_V2 || super.s_magic == SUPER_V3)
    ? fsbuf(indir).b__v2_ind[(int) zone]
    : fsbuf(indir).b__v1_ind[(int) zone];

  /* Calculate absolute datablock number */
  z = ((MNX(block_t)) zone << zone_shift) + zone_index;
  return z;
}

MNX(ino_t) r_lookup(MNX(Ino_t) cwd, char *path)
/* Translates a pathname to an inode number.  This is just a nice utility
 * function, it only needs r_stat and r_readdir.
 */
{
  NUCS_TRACE(DO_TRACE,"\n");
  char name[NAME_MAX+1], r_name[NAME_MAX+1];
  char *n;
  /* just to be more telling i.e. we use minix `struct stat' in r_stat */
  struct MNX(stat) MNX(st);

  MNX(ino_t) ino;

  ino= (path[0] == '/') ? ROOT_INO : cwd;

  NUCS_TRACE(DO_TRACE,"ino=%lu\n",ino);
  NUCS_TRACE(DO_TRACE,"path=%s\n",path);

  for (;;) {
    if (ino == 0) {
      errno= ENOENT;
      return 0;
    }

    while (*path == '/') path++;

    if (*path == 0) return ino;

    r_stat(ino, &MNX(st));

    if (!S_ISDIR(MNX(st).st_mode)) {
      errno= ENOTDIR;
      return 0;
    }

    n= name;
    while (*path != 0 && *path != '/')
      if (n < name + NAME_MAX)
        *n++ = *path++;

    *n= 0;

    while ((ino= r_readdir(r_name)) != 0 &&
           strcmp(name, r_name) != 0) {
    }
  }
}
