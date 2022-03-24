/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * client_pmem_map_file.c -- a function to map PMem using libpmem
 *
 * Please see README.md for a detailed description of this example.
 */

#include <libpmem.h>
#include <stdlib.h>
#include <stdio.h>

#include "common-conn.h"
#include "common-example.h"
#include "hello.h"

static int client_pmem_map_file(char *path, int argc,
			struct example_mem *mem)
{
	int is_pmem = 0;
	if (argc >= 4) {

		/* map the file */
		mem->mr_ptr = pmem_map_file(path, 0 /* len */, 0 /* flags */,
				0 /* mode */, mem->mr_size, &is_pmem);
		if (mem->mr_ptr == NULL) {
			(void) fprintf(stderr,
				"pmem_map_file() for %s failed\n", path);
			return -1;
		}

		/* pmem is expected */
		if (!is_pmem) {
			(void) fprintf(stderr, "%s is not an actual PMEM\n",
				path);
			(void) pmem_unmap(mem->mr_ptr, *mem->mr_size);
			return -1;
		}

		*mem->is_pmem = is_pmem;

		/*
		 * At the beginning of the persistent memory, a signature is
		 * stored which marks its content as valid. So the length
		 * of the mapped memory has to be at least of the length of
		 * the signature to convey any meaningful content and be usable
		 * as a persistent store.
		 */
		if (*mem->mr_size < SIGNATURE_LEN) {
			(void) fprintf(stderr, "%s too small (%zu < %zu)\n",
					path, *mem->mr_size, SIGNATURE_LEN);
			(void) pmem_unmap(mem->mr_ptr, *mem->mr_size);
			return -1;
		}
		*mem->data_offset = SIGNATURE_LEN;

		/*
		 * The space under the offset is intended for storing the hello
		 * structure. So the total space is assumed to be at least
		 * 1 KiB + offset of the string contents.
		 */
		if (
		*mem->mr_size - *mem->data_offset < sizeof(struct hello_t)) {
			fprintf(stderr, "%s too small (%zu < %zu)\n",
				path, *mem->mr_size, sizeof(struct hello_t));
			(void) pmem_unmap(mem->mr_ptr, *mem->mr_size);
			return -1;
		}

		mem->hello = (struct hello_t *)
				((uintptr_t)mem->mr_ptr + *mem->data_offset);

		/*
		 * If the signature is not in place the persistent content has
		 * to be initialized and persisted.
		 */
		if (strncmp(mem->mr_ptr, SIGNATURE_STR, SIGNATURE_LEN) != 0) {
			/* write an initial value and persist it */
			write_hello_str(mem->hello, en);
			pmem_persist(mem->hello, sizeof(struct hello_t));
			/* write the signature to mark the content as valid */
			memcpy(mem->mr_ptr, SIGNATURE_STR, SIGNATURE_LEN);
			pmem_persist(mem->mr_ptr, SIGNATURE_LEN);
		}
}
