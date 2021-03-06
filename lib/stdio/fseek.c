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
 * fseek.c - perform an fseek
 */
#include	<stdio.h>

#if	(SEEK_CUR != 1) || (SEEK_END != 2) || (SEEK_SET != 0)
#error SEEK_* values are wrong
#endif

#include	"loc_incl.h"

#include	<nucleos/types.h>

off_t lseek(int fildes, off_t offset, int whence);

int fseek(FILE *stream, long int offset, int whence)
{
	int adjust = 0;
	long pos;

	stream->_flags &= ~(_IOEOF | _IOERR);
	/* Clear both the end of file and error flags */

	if (io_testflag(stream, _IOREADING)) {
		if (whence == SEEK_CUR
		    && stream->_buf
		    && !io_testflag(stream,_IONBF))
			adjust = stream->_count;
		stream->_count = 0;
	} else if (io_testflag(stream,_IOWRITING)) {
		fflush(stream);
	} else	/* neither reading nor writing. The buffer must be empty */
		/* EMPTY */ ;

	pos = lseek(fileno(stream), offset - adjust, whence);
	if (io_testflag(stream, _IOREAD) && io_testflag(stream, _IOWRITE))
		stream->_flags &= ~(_IOREADING | _IOWRITING);

	stream->_ptr = stream->_buf;
	return ((pos == -1) ? -1 : 0);
}
