/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/*  $Revision: 1.1.1.1 $
**
**  Editline system header file for Unix.
*/

#define CRLF		"\r\n"

#include <nucleos/types.h>
#include <nucleos/stat.h>

#if	defined(USE_DIRENT)
#include <nucleos/dirent.h>
typedef struct dirent	DIRENTRY;
#else
#include <nucleos/dir.h>
typedef struct direct	DIRENTRY;
#endif	/* defined(USE_DIRENT) */

#if	!defined(S_ISDIR)
#define S_ISDIR(m)		(((m) & S_IFMT) == S_IFDIR)
#endif	/* !defined(S_ISDIR) */
