/*
 *  Copyright (C) 2011  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
#ifndef __NUCS_FS_RAWFS_H
#define __NUCS_FS_RAWFS_H
/*  rawfs.h - Raw Minix file system support.        Author: Kees J. Bot
 *
 *  off_t r_super(int *block_size);
 *    Initialize variables, returns the size of a valid Minix
 *    file system blocks, but zero on error.
 *
 *  void r_stat(ino_t file, struct stat *stp);
 *    Return information about a file like stat(2) and
 *    remembers file for the next two calls.
 *
 *  off_t r_vir2abs(off_t virblockno);
 *    Translate virtual block number in file to absolute
 *    disk block number.  Returns 0 if the file contains
 *    a hole, or -1 if the block lies past the end of file.
 *
 *  ino_t r_readdir(char *name);
 *    Return next directory entry or 0 if there are no more.
 *    Returns -1 and sets errno on error.
 *
 *  ino_t r_lookup(ino_t cwd, char *path);
 *    A utility function that translates a pathname to an
 *    inode number.  It starts from directory "cwd" unless
 *    path starts with a '/', then from ROOT_INO.
 *    Returns 0 and sets errno on error.
 *
 *  One function needs to be provided by the outside world:
 *
 *  void readblock(off_t blockno, char *buf, int block_size);
 *    Read a block into the buffer.  Outside world handles errors.
 */
#include "../include/sys/types.h"

#ifndef ROOT_INO
#define ROOT_INO ((MNX(ino_t)) 1)
#endif

struct MNX(stat);

MNX(off_t) r_super(int *);
void r_stat(MNX(Ino_t) file, struct MNX(stat) *stp);
MNX(off_t) r_vir2abs(MNX(off_t) virblockno);
MNX(ino_t) r_readdir(char *name);
MNX(ino_t) r_lookup(MNX(Ino_t) cwd, char *path);
#endif /* !__NUCS_FS_RAWFS_H */
