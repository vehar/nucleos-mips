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
 * freopen.c - open a file and associate a stream with it
 */
#include <nucleos/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "loc_incl.h"
#include <nucleos/stat.h>

#define	PMODE		0666

/* Do not "optimize" this file to use the open with O_CREAT if the file
 * does not exist. The reason is given in fopen.c.
 */
#define	O_RDONLY	0
#define	O_WRONLY	1
#define	O_RDWR		2

#define	O_CREAT		0x010
#define	O_TRUNC		0x020
#define	O_APPEND	0x040

int open(const char *path, int flags);
int creat(const char *path, mode_t mode);
int close(int d);

FILE *
freopen(const char *name, const char *mode, FILE *stream)
{
	register int i;
	struct stat st;
	int rwmode = 0, rwflags = 0;
	int fd, flags = stream->_flags & (_IONBF | _IOFBF | _IOLBF | _IOMYBUF);

	(void) fflush(stream);				/* ignore errors */
	(void) close(fileno(stream));

	switch(*mode++) {
	case 'r':
		flags |= _IOREAD;	
		rwmode = O_RDONLY;
		break;
	case 'w':
		flags |= _IOWRITE;
		rwmode = O_WRONLY;
		rwflags = O_CREAT | O_TRUNC;
		break;
	case 'a': 
		flags |= _IOWRITE | _IOAPPEND;
		rwmode = O_WRONLY;
		rwflags |= O_APPEND | O_CREAT;
		break;         
	default:
		goto loser;
	}

	while (*mode) {
		switch(*mode++) {
		case 'b':
			continue;
		case '+':
			rwmode = O_RDWR;
			flags |= _IOREAD | _IOWRITE;
			continue;
		/* The sequence may be followed by aditional characters */
		default:
			break;
		}
		break;
	}

	if ((rwflags & O_TRUNC)
	    || (((fd = open(name, rwmode)) < 0)
		    && (rwflags & O_CREAT))) {
		if (((fd = creat(name, PMODE)) < 0) && flags | _IOREAD) {
			(void) close(fd);
			fd = open(name, rwmode);
		}
	}

	if (fd < 0) {
		goto loser;
	}

	if ( fstat( fd, &st ) == 0 ) {
		if ( S_ISFIFO(st.st_mode) ) flags |= _IOFIFO;
	} else {
		goto loser;
	}
	
	stream->_count = 0;
	stream->_fd = fd;
	stream->_flags = flags;
	return stream;

loser:
	for( i = 0; i < FOPEN_MAX; i++) {
		if (stream == __iotab[i]) {
			__iotab[i] = 0;
			break;
		}
	}
	if (stream != stdin && stream != stdout && stream != stderr)
		free((void *)stream);
	return (FILE *)NULL;	
}
