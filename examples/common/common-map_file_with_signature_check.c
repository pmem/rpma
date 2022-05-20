// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * common-client_pmem_signature_check.c -- a function to check pmem signature
 *
 * Please see README.md for a detailed description of this example.
 */

#include "common-conn.h"
#include "common-map_file_with_signature_check.h"
#include "common-pmem_map_file.h"

/* size is used only on server side when no initiation with write_hello_str is required */
int
pmem_signature_check(char *path, struct hello_t *hello, struct example_mem *mem, size_t size)
{
	size_t min_size = size;

	if (min_size == 0)
		min_size = sizeof(struct hello_t);

	if (client_pmem_map_file(path, mem, SIGNATURE_LEN+min_size))
		return -1;

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
		return -1;
	}
	mem->data_offset = SIGNATURE_LEN;

	/*
	 * The space under the offset is intended for storing the hello
	 * structure. So the total space is assumed to be at least
	 * 1 KiB + offset of the string contents.
	 */
	if (mem->mr_size - mem->data_offset < sizeof(struct hello_t)) {
		fprintf(stderr, "%s too small (%zu < %zu)\n", path,
			mem->mr_size, sizeof(struct hello_t));
		return -1;
	}

	hello = (struct hello_t *)
			((uintptr_t)mem->mr_ptr + mem->data_offset);

	/*
	 * If the signature is not in place the persistent content has
	 * to be initialized and persisted.
	 */
	if (strncmp(mem->mr_ptr, SIGNATURE_STR, SIGNATURE_LEN) != 0) {
		if (size == 0) {
			/* write an initial value and persist it */
			write_hello_str(hello, en);
			mem->persist(hello, sizeof(struct hello_t));
		} else {
			/* write an initial empty string and persist it */
			((char *)mem->mr_ptr + mem->data_offset)[0] = '\0';
			mem->persist(mem->mr_ptr, 1);
		}
		/* write the signature to mark the content as valid */
		memcpy(mem->mr_ptr, SIGNATURE_STR, SIGNATURE_LEN);
		mem->persist(mem->mr_ptr, SIGNATURE_LEN);
	}

	return 0;
}
