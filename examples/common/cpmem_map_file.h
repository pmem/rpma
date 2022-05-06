/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * client_pmem_map_file.h -- a header file for both client_pmem_map_file.c
 * and client_pmem2_map_file.c
 *
 * Please see README.md for a detailed description of this example.
 */

#ifndef EXAMPLES_COMMON_PMEM_MAP_FILE
#define EXAMPLES_COMMON_PMEM_MAP_FILE

#include "common-pmem.h"

typedef void (*persist_fn)(const void *ptr, size_t size);

struct example_mem {
	/* memory buffer */
	char *mr_ptr;
	/* size of the mapped persistent memory */
	size_t mr_size;
	size_t data_offset;
	int is_pmem;
	persist_fn persist;

#ifdef USE_LIBPMEM2
	/* libpmem2 structure used for mapping PMem */
	struct pmem2_map *map;
#endif
};

int client_pmem_map_file(char *path, int argc, struct example_mem *mem);

void client_pmem_unmap_file(struct example_mem *mem);
#endif /* EXAMPLES_COMMON_PMEM_MAP_FILE */
