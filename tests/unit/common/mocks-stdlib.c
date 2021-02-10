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
#include "test-common.h"

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
	void *ret = mock_type(void *);
	if (ret != (void *)MOCK_OK)
		return MAP_FAILED;

	struct mmap_args *args = mock_type(struct mmap_args *);

	void *memptr = __real__test_malloc(__len);

	/*
	 * Save the address and length of the allocated memory
	 * in order to verify it later.
	 */
	args->addr = memptr;
	args->len = __len;

	return memptr;
}

/*
 * __wrap_munmap -- munmap() mock
 */
int
__wrap_munmap(void *__addr, size_t __len)
{
	struct mmap_args *args = mock_type(struct mmap_args *);
	assert_ptr_equal(__addr, args->addr);
	assert_int_equal(__len, args->len);

	test_free(__addr);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}
