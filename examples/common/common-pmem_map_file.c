// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * common-pmem_map_file.c -- a function to map PMem using libpmem
 *
 * Please see README.md for a detailed description of this example.
 */

#include <stdio.h>
#include "common-pmem_map_file.h"

int
client_pmem_map_file(char *path, struct example_mem *mem)
{
	mem->is_pmem = 0;

	/* map the file */
	mem->mr_ptr = pmem_map_file(path, 0 /* len */, 0 /* flags */,
			0 /* mode */, &mem->mr_size, &mem->is_pmem);
	if (mem->mr_ptr == NULL) {
		(void) fprintf(stderr,
			"pmem_map_file() for %s failed\n", path);
		return -1;
	}

	/* pmem is expected */
	if (!mem->is_pmem) {
		(void) fprintf(stderr, "%s is not an actual PMEM\n",
			path);
		(void) pmem_unmap(mem->mr_ptr, mem->mr_size);
		return -1;
	}

	if (mem->is_pmem)
		mem->persist = pmem_persist;

	return 0;
}

void
client_pmem_unmap_file(struct example_mem *mem)
{
	if (mem->is_pmem) {
		(void) pmem_unmap(mem->mr_ptr, mem->mr_size);
		mem->mr_ptr = NULL;
	}
}
