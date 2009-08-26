/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/* The <nucleos/errno.h> header defines the numbers of the various errors that can
 * occur during program execution.  They are visible to user programs and 
 * should be small positive integers.  However, they are also used within 
 * MINIX, where they must be negative.  For example, the READ system call is 
 * executed internally by calling do_read().  This function returns either a 
 * (negative) error number or a (positive) number of bytes actually read.
 *
 * To solve the problem of having the error numbers be negative inside the
 * the system and positive outside, the following mechanism is used.  All the
 * definitions are are the form:
 *
 *	#define EPERM		(_SIGN 1)
 *
 * If the macro _SYSTEM is defined, then  _SIGN is set to "-", otherwise it is
 * set to "".  Thus when compiling the operating system, the  macro _SYSTEM
 * will be defined, setting EPERM to (- 1), whereas when when this
 * file is included in an ordinary user program, EPERM has the value ( 1).
 */

#ifndef __NUCLEOS_ERRNO_H		/* check if <nucleos/errno.h> is already included */
#define __NUCLEOS_ERRNO_H		/* it is not included; note that fact */

extern int errno;		/* place where the error numbers go */

/* Here are the numerical values of the error numbers. */
#define _NERROR			70  /* number of errors */

#define EGENERIC		99  /* generic error */
#define EPERM			1   /* operation not permitted */
#define ENOENT			2   /* no such file or directory */
#define ESRCH			3   /* no such process */
#define EINTR			4   /* interrupted function call */
#define EIO			5   /* input/output error */
#define ENXIO			6   /* no such device or address */
#define E2BIG			7   /* arg list too long */
#define ENOEXEC			8   /* exec format error */
#define EBADF			9   /* bad file descriptor */
#define ECHILD			10  /* no child process */
#define EAGAIN			11  /* resource temporarily unavailable */
#define ENOMEM			12  /* not enough space */
#define EACCES			13  /* permission denied */
#define EFAULT			14  /* bad address */
#define ENOTBLK			15  /* Extension: not a block special file */
#define EBUSY			16  /* resource busy */
#define EEXIST			17  /* file exists */
#define EXDEV			18  /* improper link */
#define ENODEV			19  /* no such device */
#define ENOTDIR			20  /* not a directory */
#define EISDIR			21  /* is a directory */
#define EINVAL			22  /* invalid argument */
#define ENFILE			23  /* too many open files in system */
#define EMFILE			24  /* too many open files */
#define ENOTTY			25  /* inappropriate I/O control operation */
#define ETXTBSY			26  /* no longer used */
#define EFBIG			27  /* file too large */
#define ENOSPC			28  /* no space left on device */
#define ESPIPE			29  /* invalid seek */
#define EROFS			30  /* read-only file system */
#define EMLINK			31  /* too many links */
#define EPIPE			32  /* broken pipe */
#define EDOM			33  /* domain error (from ANSI C std) */
#define ERANGE			34  /* result too large (from ANSI C std) */
#define EDEADLK			35  /* resource deadlock avoided */
#define ENAMETOOLONG		36  /* file name too long */
#define ENOLCK			37  /* no locks available */
#define ENOSYS			38  /* function not implemented */
#define ENOTEMPTY		39  /* directory not empty */
#define ELOOP			40  /* too many levels of symlinks detected */
#define ERESTART		41  /* driver restarted */

/* The following errors relate to networking. */
#define EPACKSIZE		50  /* invalid packet size for some protocol */
#define EOUTOFBUFS		51  /* not enough buffers left */
#define EBADIOCTL		52  /* illegal ioctl for device */
#define EBADMODE		53  /* badmode in ioctl */
#define EWOULDBLOCK		54  /* call would block on nonblocking socket */
#define EBADDEST		55  /* not a valid destination address */
#define EDSTNOTRCH		56  /* destination not reachable */
#define EISCONN			57  /* already connected */
#define EADDRINUSE		58  /* address in use */
#define ECONNREFUSED		59  /* connection refused */
#define ECONNRESET		60  /* connection reset */
#define ETIMEDOUT		61  /* connection timed out */
#define EURG			62  /* urgent data present */
#define ENOURG			63  /* no urgent data present */
#define ENOTCONN		64  /* no connection (yet or anymore) */
#define ESHUTDOWN		65  /* a write call to a shutdown connection */
#define ENOCONN			66  /* no such connection */
#define EAFNOSUPPORT		67  /* address family not supported */
#define EPROTONOSUPPORT		68 /* protocol not supported by AF */
#define EPROTOTYPE		69  /* Protocol wrong type for socket */
#define EINPROGRESS		70  /* Operation now in progress */
#define EADDRNOTAVAIL		71  /* Can't assign requested address */
#define EALREADY		72  /* Connection already in progress */
#define EMSGSIZE		73  /* Message too long */
#define ENOTSOCK		74  /* Socket operation on non-socket */
#define ENOPROTOOPT		75  /* Protocol not available */
#define EOPNOTSUPP		76  /* Operation not supported */

/* The following are not POSIX errors, but they can still happen. 
 * All of these are generated by the kernel and relate to message passing.
 */
#define ELOCKED			101  /* can't send message due to deadlock */
#define EBADCALL		102  /* illegal system call number */
#define EBADSRCDST		103  /* bad source or destination process */
#define ECALLDENIED 		104  /* no permission for system call */
#define EDEADSRCDST		105  /* source or destination is not alive */
#define ENOTREADY		106  /* source or destination is not ready */
#define EBADREQUEST		107  /* destination cannot handle request */
#define ESRCDIED		108  /* source just died */
#define EDSTDIED		109  /* destination just died */
#define ETRAPDENIED		110  /* IPC trap not allowed */
#define EDONTREPLY		201  /* pseudo-code: don't send a reply */

#endif /* __NUCLEOS_ERRNO_H */
