/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * mtt_utils.h -- helper functions for multihreaded tests
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "mtt_utils.h"

/*
 * malloc_aligned -- allocate an aligned chunk of memory
 */
void *
malloc_aligned(size_t size)
{
	long pagesize = sysconf(_SC_PAGESIZE);
	if (pagesize < 0) {
		perror("sysconf");
		return NULL;
	}

	/* allocate a page size aligned local memory pool */
	void *mem;
	int ret = posix_memalign(&mem, (size_t)pagesize, size);
	if (ret) {
		(void) fprintf(stderr, "posix_memalign: %s\n", strerror(ret));
		return NULL;
	}

	/* zero the allocated memory */
	memset(mem, 0, size);

	return mem;
}
