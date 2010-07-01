/*
 *  Copyright (C) 2010  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/* The <nucleos/fcntl.h> header is needed by the open() and fcntl() system calls,
 * which  have a variety of parameters and flags.  They are described here.  
 * The formats of the calls to each of these are:
 *
 *	open(path, oflag [,mode])	open a file
 *	fcntl(fd, cmd [,arg])		get or set file attributes
 * 
 */
#ifndef __ASM_GENERIC_FCNTL_H
#define __ASM_GENERIC_FCNTL_H

#include <nucleos/types.h>

/* These values are used for cmd in fcntl().  POSIX Table 6-1.  */
#define F_DUPFD		0	/* duplicate file descriptor */
#define F_GETFD		1	/* get file descriptor flags */
#define F_SETFD		2	/* set file descriptor flags */
#define F_GETFL		3	/* get file status flags */
#define F_SETFL		4	/* set file status flags */
#define F_GETLK		5	/* get record locking information */
#define F_SETLK		6	/* set record locking information */
#define F_SETLKW	7	/* set record locking info; wait if blocked */
#define F_FREESP	8	/* free a section of a regular file */

/* File descriptor flags used for fcntl().  POSIX Table 6-2. */
#define FD_CLOEXEC	1	/* close on exec flag for third arg of fcntl */

/* L_type values for record locking with fcntl().  POSIX Table 6-3. */
#define F_RDLCK		1	/* shared or read lock */
#define F_WRLCK		2	/* exclusive or write lock */
#define F_UNLCK		3	/* unlock */

/* Oflag values for open().  POSIX Table 6-4. */
#define O_CREAT		00100	/* creat file if it doesn't exist */
#define O_EXCL		00200	/* exclusive use flag */
#define O_NOCTTY	00400	/* do not assign a controlling terminal */
#define O_TRUNC		01000	/* truncate flag */

/* File status flags for open() and fcntl().  POSIX Table 6-5. */
#define O_APPEND	02000	/* set append mode */
#define O_NONBLOCK	04000	/* no delay */
#define O_REOPEN	010000	/* automatically re-open device after driver
				 * restart
				 */

/* File access modes for open() and fcntl().  POSIX Table 6-6. */
#define O_RDONLY	0	/* open(name, O_RDONLY) opens read only */
#define O_WRONLY	1	/* open(name, O_WRONLY) opens write only */
#define O_RDWR		2	/* open(name, O_RDWR) opens read/write */

/* Mask for use with file access modes.  POSIX Table 6-7. */
#define O_ACCMODE	03	/* mask for file access modes */

/* Struct used for locking.  POSIX Table 6-8. */
struct flock {
	short l_type;			/* type: F_RDLCK, F_WRLCK, or F_UNLCK */
	short l_whence;		/* flag for starting offset */
	off_t l_start;		/* relative offset in bytes */
	off_t l_len;			/* size; if 0, then until EOF */
	pid_t l_pid;			/* process id of the locks' owner */
};

#if defined(__KERNEL__) || defined(__UKERNEL__)

/* Function Prototypes. */
int creat(const char *_path, mode_t _mode);
int fcntl(int _filedes, int _cmd, ...);
int open(const char *_path, int _oflag, ...);
int flock(int fd, int mode);

#endif /* defined(__KERNEL__) || defined(__UKERNEL__) */

/* For locking files. */
#define LOCK_SH		F_RDLCK		/* Shared lock */
#define LOCK_EX		F_WRLCK		/* Exclusive lock */
#define LOCK_NB		0x0080		/* Do not block when locking */
#define LOCK_UN		F_UNLCK		/* Unlock */

#endif /* __ASM_GENERIC_FCNTL_H */
