/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mocks-stdlib.c -- stdlib mocks
 */

#include <errno.h>
#include <stddef.h>

#include "cmocka_headers.h"
#include "mocks-stdlib.h"

void *__real__test_malloc(size_t size);

/*
 * __wrap__test_malloc -- malloc() mock
 */
void *
__wrap__test_malloc(size_t size)
{
	errno = mock_type(int);

	if (errno)
		return NULL;

	return __real__test_malloc(size);
}

/*
 * __wrap_posix_memalign -- posix_memalign() mock
 */
int
__wrap_posix_memalign(void **memptr, size_t alignment, size_t size)
{
	int err = mock_type(int);
	if (err)
		return err;

	struct posix_memalign_args *args =
		mock_type(struct posix_memalign_args *);

	*memptr = __real__test_malloc(size);

	/* save the address of the allocated memory to verify it later */
	args->ptr = *memptr;

	return 0;
}
