// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * common-map_file_with_signature_check.c -- a function to check pmem signature
 */

#include <stdio.h>
#include "common-conn.h"
#include "common-map_file_with_signature_check.h"
#include "common-pmem_map_file.h"

int
common_pmem_map_file_with_signature_check(char *path, size_t size, struct common_mem *mem,
						common_init_func_t init_pmem)
{
	if (path == NULL || size == 0 || mem == NULL)
		return -1;

	/*
	 * The beginning of the used persistent memory starts at mem->offset.
	 * At the beginning of the used persistent memory, the signature is stored
	 * which marks its content as valid. So the total space is assumed to be at least:
	 * mem->offset + SIGNATURE_LEN + the size expected by the user.
	 */
	mem->data_offset = mem->offset + SIGNATURE_LEN;
	size += mem->data_offset;

	if (common_pmem_map_file(path, size, mem))
		return -1;

	if (mem->mr_size < size) {
		(void) fprintf(stderr, "%s has too small size (%zu < %zu)\n",
				path, mem->mr_size, size);
		return -1;
	}

	/* beginning of the used persistent memory */
	char *pmem = mem->mr_ptr + mem->offset;

	/*
	 * If the signature is not in place the persistent content has
	 * to be initialized and persisted.
	 */
	if (strncmp(pmem, SIGNATURE_STR, SIGNATURE_LEN) != 0) {
		char *pmem_data = pmem + mem->data_offset;
		if (init_pmem) {
			/* write the initial hello string and persist it */
			ssize_t size_to_persist = (*init_pmem)(pmem_data, size - SIGNATURE_LEN);
			if (size_to_persist < 0) {
				(void) fprintf(stderr, "Initialization of PMem failed.\n");
				return -1;
			}
			mem->persist(pmem_data, (size_t)size_to_persist);
		} else {
			/* write the initial empty string and persist it */
			pmem_data[0] = '\0';
			mem->persist(pmem_data, 1);
		}
		/* write the signature to mark the content as valid */
		memcpy(pmem, SIGNATURE_STR, SIGNATURE_LEN);
		mem->persist(pmem, SIGNATURE_LEN);
	}

	return 0;
}
