/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * common-pmem_map_file.h -- a header file for both common-pmem_map_file.c
 * and common-pmem2_map_file.c
 */

#ifndef COMMON_PMEM_MAP_FILE_H
#define COMMON_PMEM_MAP_FILE_H

#include "common-pmem.h"

typedef void (*persist_fn)(const void *ptr, size_t size);

struct common_mem {
	/* memory buffer */
	char *mr_ptr;
	/* size of the mapped persistent memory */
	size_t mr_size;
	/* offset of the beginning of the used persistent memory */
	size_t offset;
	/* offset of user data after the pmem signature */
	size_t data_offset;
	int is_pmem;
	persist_fn persist;

#ifdef USE_LIBPMEM2
	/* libpmem2 structure used for mapping PMem */
	struct pmem2_map *map;
#endif
};

int common_pmem_map_file(char *path, size_t min_size, struct common_mem *mem);

void common_pmem_unmap_file(struct common_mem *mem);

#endif /* COMMON_PMEM_MAP_FILE_H */
