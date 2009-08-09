/*
 *  Copyright (C) 2009  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/*	uname(3) - describe the machine.		Author: Kees J. Bot
 *								5 Dec 1992
 */

#define uname	_uname
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <nucleos/types.h>
#include <sys/utsname.h>

#define uts_get(field, string) \
	if (sysuname(_UTS_GET, field, name->string, sizeof(name->string)) < 0) \
		return -1; \
	name->string[sizeof(name->string)-1]= 0;

int uname(name)
struct utsname *name;
{
  int hf, n, err;
  char *nl;

	/* Get each of the strings with a sysuname call.  Null terminate them,
	 * because the buffers in the kernel may grow before this and the
	 * programs are recompiled.
	 */
	uts_get(_UTS_SYSNAME, sysname);
	uts_get(_UTS_NODENAME, nodename);
	uts_get(_UTS_RELEASE, release);
	uts_get(_UTS_VERSION, version);
	uts_get(_UTS_MACHINE, machine);
	uts_get(_UTS_ARCH, arch);
#if 0
	uts_get(_UTS_KERNEL, kernel);
	uts_get(_UTS_HOSTNAME, hostname);
	uts_get(_UTS_BUS, bus);
#endif

	/* Try to read the node name from /etc/hostname.file. This information
	 * should be stored in the kernel.
	 */
  if ((hf = open("/etc/hostname.file", O_RDONLY)) < 0) {
	if (errno != ENOENT) return(-1);
  } else {
	n = read(hf, name->nodename, sizeof(name->nodename) - 1);
	err = errno;
	close(hf);
	errno = err;
	if (n < 0) return(-1);
	name->nodename[n] = 0;
	if ((nl = strchr(name->nodename, '\n')) != NULL) {
			memset(nl, 0, (name->nodename +
				sizeof(name->nodename)) - nl);
	}
  }
	return 0;
}
