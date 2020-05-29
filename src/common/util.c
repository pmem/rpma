// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2014-2020, Intel Corporation */

/*
 * util.c -- very basic utilities
 */

#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include "out.h"

/*
 * util_snprintf -- run snprintf; in case of truncation or a failure
 * return a negative value, or the number of characters printed otherwise.
 */
int
util_snprintf(char *str, size_t size, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int ret = vsnprintf(str, size, format, ap);
	va_end(ap);

	if (ret < 0) {
		if (!errno)
			errno = EIO;
		goto err;
	} else if ((size_t)ret >= size) {
		errno = ENOBUFS;
		goto err;
	}

	return ret;
err:
	return -1;
}

/* pass through for Posix */
void
util_strerror(int errnum, char *buff, size_t bufflen)
{
	strerror_r(errnum, buff, bufflen);
}

/*
 * util_getexecname -- return name of current executable
 */
char *
util_getexecname(char *path, size_t pathlen)
{
	ASSERT(pathlen != 0);
	ssize_t cc;

	cc = readlink("/proc/self/exe", path, pathlen);
	if (cc == -1) {
		strncpy(path, "unknown", pathlen);
		path[pathlen - 1] = '\0';
	} else {
		path[cc] = '\0';
	}

	return path;
}
