/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * common-pmem.h -- a common structure used for mapping PMem for examples
 */

#include <stddef.h>

#ifdef USE_LIBPMEM2
#include <libpmem2.h>
#elif USE_LIBPMEM
#include <libpmem.h>
#endif

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
