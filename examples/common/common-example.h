/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * common-conn.h -- a common structure used for
 * mapping PMem for examples
 */

struct example_mem {
	/* memory buffer */
	char *mr_ptr;
	/* size of the mapped persistent memory */
	size_t *mr_size;
	size_t *data_offset;
	int *is_pmem;
	struct hello_t *hello;

#ifdef USE_LIBPMEM2
	/* libpmem2 structure used for mapping PMem */
	struct pmem2_map *map;
	pmem2_persist_fn persist;
#endif
};
