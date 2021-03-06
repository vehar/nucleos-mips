/*
 *  Copyright (C) 2012  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/* The <nucleos/stat.h> header defines a struct that is used in the stat() and
 * fstat functions.  The information in this struct comes from the i-node of
 * some file.  These calls are the only approved way to inspect i-nodes.
 */

#ifndef _NUCLEOS_STAT_H
#define _NUCLEOS_STAT_H

#if defined(__KERNEL__) || defined(__UKERNEL__)

#include <nucleos/types.h>
#include <asm/stat.h>

/* Traditional mask definitions for st_mode. */
#define S_IFMT		0170000	/* type of file */
#define S_IFLNK		0120000	/* symbolic link */
#define S_IFREG		0100000	/* regular */
#define S_IFBLK		0060000	/* block special */
#define S_IFDIR		0040000	/* directory */
#define S_IFCHR		0020000	/* character special */
#define S_IFIFO		0010000	/* this is a FIFO */
#define S_ISUID		0004000	/* set user id on execution */
#define S_ISGID		0002000	/* set group id on execution */
				/* next is reserved for future use */
#define S_ISVTX		0001000	/* save swapped text even after use */

/* POSIX masks for st_mode. */
#define S_IRWXU		00700	/* owner:  rwx------ */
#define S_IRUSR		00400	/* owner:  r-------- */
#define S_IWUSR		00200	/* owner:  -w------- */
#define S_IXUSR		00100	/* owner:  --x------ */

#define S_IRWXG		00070	/* group:  ---rwx--- */
#define S_IRGRP		00040	/* group:  ---r----- */
#define S_IWGRP		00020	/* group:  ----w---- */
#define S_IXGRP		00010	/* group:  -----x--- */

#define S_IRWXO		00007	/* others: ------rwx */
#define S_IROTH		00004	/* others: ------r-- */ 
#define S_IWOTH		00002	/* others: -------w- */
#define S_IXOTH		00001	/* others: --------x */

/* Synonyms for above. */
#define S_IEXEC		S_IXUSR
#define S_IWRITE	S_IWUSR
#define S_IREAD		S_IRUSR

/* The following macros test st_mode (from POSIX Sec. 5.6.1.1). */
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)	/* is a reg file */
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)	/* is a directory */
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)	/* is a char spec */
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)	/* is a block spec */
#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)	/* is a symlink */
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)	/* is a pipe/FIFO */

/* Function Prototypes. */
int chmod(const char *_path, mode_t _mode);
int fchmod(int fd, mode_t _mode);
int fstat(int _fildes, struct stat *_buf);
int mkdir(const char *_path, mode_t _mode);
int mkfifo(const char *_path, mode_t _mode);
int stat(const char *_path, struct stat *_buf);
mode_t umask(mode_t _cmask);

/* Open Group Base Specifications Issue 6 (not complete) */
int lstat(const char *_path, struct stat *_buf);

#include <nucleos/time.h>

/* @nucleos: This one is for the VFS and various filesystems. Arch may have its
 *           own definition of `struct stat' (e.g. according to asm-generic/stat.h).
 *           The `struct stat' is not used in concrete FS but _only_ in VFS as
 *           the `struct stat' may change or may be different on various platforms
 *           and we don't want to take care about that in every FS.
 *           The VFS server takes care about that.
 */
struct kstat {
	u64			ino;
	dev_t			dev;
	mode_t			mode;
	unsigned int		nlink;
	uid_t			uid;
	gid_t			gid;
	dev_t			rdev;
	loff_t			size;
	struct timespec		atime;
	struct timespec		mtime;
	struct timespec		ctime;
	unsigned long		blksize;
	unsigned long long	blocks;
};

#endif/* defined(__KERNEL__) || defined(__UKERNEL__) */

#endif /* _NUCLEOS_STAT_H */
