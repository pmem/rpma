/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * common-conn.h -- a common structure used for
 * mapping PMem for examples
 */

#include <stddef.h>
#include "hello.h"

typedef void (*persist_fn)(const void *ptr, size_t size);

struct example_mem {
	/* memory buffer */
	char *mr_ptr;
	/* size of the mapped persistent memory */
	size_t mr_size;
	size_t data_offset;
	int *is_pmem;
	persist_fn persist;

#ifdef USE_LIBPMEM2
	/* libpmem2 structure used for mapping PMem */
	struct pmem2_map *map;
#endif
};

static inline void
write_hello_str(struct hello_t *hello, enum lang_t lang)
{
	hello->lang = lang;
	strncpy(hello->str, hello_str[hello->lang], KILOBYTE - 1);
	hello->str[KILOBYTE - 1] = '\0';
}

static void
translate(struct hello_t *hello)
{
	printf("translating...\n");
	enum lang_t lang = (enum lang_t)((hello->lang + 1) % LANG_NUM);
	write_hello_str(hello, lang);
}
