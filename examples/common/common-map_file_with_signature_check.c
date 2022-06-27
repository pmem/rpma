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

/* size is used only on server side when no initiation with write_hello_str is required */
int
common_pmem_map_file_with_signature_check(char *path, size_t size, struct common_mem *mem)
{
	size_t min_size = size;

	if (min_size == 0)
		min_size = sizeof(struct hello_t);

	if (common_pmem_map_file(path, SIGNATURE_LEN + min_size, mem))
		return -1;

	/*
	 * At the beginning of the persistent memory, a signature is stored which marks its content
	 * as valid. So the total space is assumed to be at least SIGNATURE_LEN + the size expected
	 * by the user.
	 */
	if (mem->mr_size < SIGNATURE_LEN + min_size) {
		(void) fprintf(stderr, "%s has too small size (%zu < %zu)\n",
				path, mem->mr_size, SIGNATURE_LEN + min_size);
		return -1;
	}
	mem->data_offset = SIGNATURE_LEN;

	/*
	 * If the signature is not in place the persistent content has
	 * to be initialized and persisted.
	 */
	if (strncmp(mem->mr_ptr, SIGNATURE_STR, SIGNATURE_LEN) != 0) {
		if (size == 0) {
			/* write an initial value and persist it */
			write_hello_str((struct hello_t *)(mem->mr_ptr + mem->data_offset), en);
			mem->persist(mem->mr_ptr + mem->data_offset, sizeof(struct hello_t));
		} else {
			/* write an initial empty string and persist it */
			(mem->mr_ptr + mem->data_offset)[0] = '\0';
			mem->persist(mem->mr_ptr + mem->data_offset, 1);
		}
		/* write the signature to mark the content as valid */
		memcpy(mem->mr_ptr, SIGNATURE_STR, SIGNATURE_LEN);
		mem->persist(mem->mr_ptr, SIGNATURE_LEN);
	}

	return 0;
}
