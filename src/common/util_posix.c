// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2015-2020, Intel Corporation */

/*
 * util_posix.c -- Abstraction layer for misc utilities (Posix implementation)
 */

#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include "os.h"
#include "out.h"
#include "util.h"

/* pass through for Posix */
void
util_strerror(int errnum, char *buff, size_t bufflen)
{
	strerror_r(errnum, buff, bufflen);
}

/*
 * util_strwinerror -- should never be called on posix OS - abort()
 */
void
util_strwinerror(unsigned long err, char *buff, size_t bufflen)
{
	abort();
}

/*
 * util_part_realpath -- get canonicalized absolute pathname
 *
 * As paths used in a poolset file have to be absolute (checked when parsing
 * a poolset file), here we only have to resolve symlinks.
 */
char *
util_part_realpath(const char *path)
{
	return realpath(path, NULL);
}

/*
 * util_compare_file_inodes -- compare device and inodes of two files;
 *                             this resolves hard links
 */
int
util_compare_file_inodes(const char *path1, const char *path2)
{
	struct stat sb1, sb2;
	if (os_stat(path1, &sb1)) {
		if (errno != ENOENT) {
			ERR("!stat failed for %s", path1);
			return -1;
		}
		LOG(1, "stat failed for %s", path1);
		errno = 0;
		return strcmp(path1, path2) != 0;
	}

	if (os_stat(path2, &sb2)) {
		if (errno != ENOENT) {
			ERR("!stat failed for %s", path2);
			return -1;
		}
		LOG(1, "stat failed for %s", path2);
		errno = 0;
		return strcmp(path1, path2) != 0;
	}

	return sb1.st_dev != sb2.st_dev || sb1.st_ino != sb2.st_ino;
}

/*
 * util_aligned_malloc -- allocate aligned memory
 */
void *
util_aligned_malloc(size_t alignment, size_t size)
{
	void *retval = NULL;

	errno = posix_memalign(&retval, alignment, size);

	return retval;
}

/*
 * util_aligned_free -- free allocated memory in util_aligned_malloc
 */
void
util_aligned_free(void *ptr)
{
	free(ptr);
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
