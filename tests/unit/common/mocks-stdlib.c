// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * mocks-stdlib.c -- stdlib mocks
 */

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/mman.h>

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
 * __wrap_mmap -- mmap() mock
 */
void *
__wrap_mmap(void *__addr, size_t __len, int __prot,
		int __flags, int __fd, off_t __offset)
{
	void *err = mock_type(void *);
	if (err == MAP_FAILED)
		return MAP_FAILED;

	struct mmap_args *args = mock_type(struct mmap_args *);

	void *memptr = __real__test_malloc(__len);

	/* save the address of the allocated memory to verify it later */
	args->ptr = memptr;

	return memptr;
}

/*
 * __wrap_munmap -- munmap() mock
 */
int
__wrap_munmap(void *__addr, size_t __len)
{
	(void) __len; /* unsused */

	test_free(__addr);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}
