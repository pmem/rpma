// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * common-pmem_map_file.c -- a function to map PMem using libpmem
 */

#include <stdio.h>
#include "common-pmem_map_file.h"

int
common_pmem_map_file(char *path, size_t min_size, struct common_mem *mem)
{
	mem->is_pmem = 0;

	/* map the file */
	mem->mr_ptr = pmem_map_file(path, 0 /* len */, 0 /* flags */,
			0 /* mode */, &mem->mr_size, &mem->is_pmem);
	if (mem->mr_ptr == NULL) {
		(void) fprintf(stderr, "pmem_map_file() for %s failed\n", path);
		return -1;
	}

	if (mem->mr_size < min_size) {
		(void) fprintf(stderr, "mapped size for %s is too small\n", path);
		(void) pmem_unmap(mem->mr_ptr, mem->mr_size);
		return -1;
	}

	/* pmem is expected */
	if (!mem->is_pmem) {
		(void) fprintf(stderr, "%s is not an actual PMEM\n", path);
		(void) pmem_unmap(mem->mr_ptr, mem->mr_size);
		return -1;
	}

	if (mem->is_pmem)
		mem->persist = pmem_persist;

	return 0;
}

void
common_pmem_unmap_file(struct common_mem *mem)
{
	if (mem->is_pmem) {
		(void) pmem_unmap(mem->mr_ptr, mem->mr_size);
		mem->mr_ptr = NULL;
		mem->is_pmem = 0;
	}
}
