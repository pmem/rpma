/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * client_pmem2_map_file.c -- a function to map PMem using libpmem2
 *
 * Please see README.md for a detailed description of this example.
 */

#include <fcntl.h>
#include <librpma.h>
#include <libpmem2.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "common-conn.h"
#include "common-example.h"
#include "hello.h"

static int client_pmem_map_file(char *path, int argc,
			struct example_mem *mem)
{
	int fd;
	struct pmem2_config *cfg = NULL;
	struct pmem2_map *map = NULL;
	struct pmem2_source *src = NULL;
	pmem2_persist_fn persist;

	if (argc >= 4) {
		if ((fd = open(path, O_RDWR)) < 0) {
			(void) fprintf(stderr, "cannot open file\n");
			return -1;
		}

		if (pmem2_source_from_fd(&src, fd) != 0) {
			(void) fprintf(stderr,
				"pmem2_source_from_fd() failed\n");
			goto err_close;
		}

		if (pmem2_config_new(&cfg) != 0) {
			(void) fprintf(stderr,
				"pmem2_config_new() failed\n");
			goto err_source_delete;
		}

		if (pmem2_config_set_required_store_granularity(cfg,
				PMEM2_GRANULARITY_CACHE_LINE) != 0) {
			(void) fprintf(stderr,
				"pmem2_config_set_required_store_granularity()failed: %s\n",
				pmem2_errormsg());
			goto err_config_delete;
		}

		if (pmem2_map_new(&map, cfg, src) != 0) {
			(void) fprintf(stderr,
				"pmem2_map_new(%s) failed: %s\n",
				path, pmem2_errormsg());
			goto err_config_delete;
		}
		mem->map = map;
		*mem->mr_size = pmem2_map_get_size(map);
		mem->mr_ptr = pmem2_map_get_address(map);

		/*
		 * At the beginning of the persistent memory, a signature is
		 * stored which marks its content as valid. So the length
		 * of the mapped memory has to be at least of the length of
		 * the signature to convey any meaningful content and be usable
		 * as a persistent store.
		 */
		if (mem->mr_size < SIGNATURE_LEN) {
			(void) fprintf(stderr, "%s too small (%zu < %zu)\n",
					path, mem->mr_size, SIGNATURE_LEN);
			goto err_map_delete;
		}
		*mem->data_offset = SIGNATURE_LEN;

		/*
		 * The space under the offset is intended for storing the hello
		 * structure. So the total space is assumed to be at least
		 * 1 KiB + offset of the string contents.
		 */
		if (
		*mem->mr_size - *mem->data_offset < sizeof(struct hello_t)) {
			fprintf(stderr, "%s too small (%zu < %zu)\n", path,
				*mem->mr_size, sizeof(struct hello_t));
			goto err_map_delete;
		}

		mem->hello = (struct hello_t *)
				((uintptr_t)mem->mr_ptr + *mem->data_offset);

		/* Get libpmem2 persist function from pmem2_map */
		persist = pmem2_get_persist_fn(map);
		mem->persist = persist;

		/*
		 * If the signature is not in place the persistent content has
		 * to be initialized and persisted.
		 */
		if (strncmp(mem->mr_ptr, SIGNATURE_STR, SIGNATURE_LEN) != 0) {
			/* write an initial value and persist it */
			write_hello_str(mem->hello, en);
			persist(mem->hello, sizeof(struct hello_t));
			/* write the signature to mark the content as valid */
			memcpy(mem->mr_ptr, SIGNATURE_STR, SIGNATURE_LEN);
			persist(mem->mr_ptr, SIGNATURE_LEN);
		}

		return 0;

err_map_delete:
		pmem2_map_delete(&map);
err_config_delete:
		pmem2_config_delete(&cfg);
err_source_delete:
		pmem2_source_delete(&src);
err_close:
		close(fd);

		return -1;
}
