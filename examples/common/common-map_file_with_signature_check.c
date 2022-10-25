// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * common-map_file_with_signature_check.c -- a function to check pmem signature
 */

#include <stdio.h>
#include "common-conn.h"
#include "common-hello.h"
#include "common-map_file_with_signature_check.h"
#include "common-pmem_map_file.h"

void
init_hello(struct common_mem *mem)
{
	/* write the initial hello string and persist it */
	write_hello_str((struct hello_t *)(mem->mr_ptr + mem->data_offset), en);
	mem->persist(mem->mr_ptr + mem->data_offset, HELLO_T_SIZE);
}

int
common_pmem_map_file_with_signature_check(char *path, size_t size, struct common_mem *mem,
						init_func_t init_pmem)
{
	if (path == NULL || size == 0 || mem == NULL)
		return -1;

	/*
	 * At the beginning of the persistent memory, a signature is stored which marks its content
	 * as valid. So the total space is assumed to be at least SIGNATURE_LEN + the size expected
	 * by the user.
	 */
	size += SIGNATURE_LEN;
	mem->data_offset = SIGNATURE_LEN;

	if (common_pmem_map_file(path, size, mem))
		return -1;

	if (mem->mr_size < size) {
		(void) fprintf(stderr, "%s has too small size (%zu < %zu)\n",
				path, mem->mr_size, size);
		return -1;
	}

	/*
	 * If the signature is not in place the persistent content has
	 * to be initialized and persisted.
	 */
	if (strncmp(mem->mr_ptr, SIGNATURE_STR, SIGNATURE_LEN) != 0) {
		if (init_pmem) {
			(*init_pmem)(mem);
		} else {
			/* write the initial empty string and persist it */
			(mem->mr_ptr + mem->data_offset)[0] = '\0';
			mem->persist(mem->mr_ptr + mem->data_offset, 1);
		}
		/* write the signature to mark the content as valid */
		memcpy(mem->mr_ptr, SIGNATURE_STR, SIGNATURE_LEN);
		mem->persist(mem->mr_ptr, SIGNATURE_LEN);
	}

	return 0;
}
