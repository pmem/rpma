// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

#ifndef COMMON_CMOCKA_ALLOC
#define COMMON_CMOCKA_ALLOC

#ifdef TEST_MOCK_ALLOC

#include <stddef.h>

extern void *_test_malloc(const size_t size, const char *file, const int line);
extern void *_test_calloc(const size_t number_of_elements, const size_t size,
		const char *file, const int line);
extern void _test_free(void *const ptr, const char *file, const int line);

#define Malloc(size) _test_malloc(size, __FILE__, __LINE__)
#define Calloc(num, size) _test_calloc(num, size, __FILE__, __LINE__)
#define Free(ptr) _test_free(ptr, __FILE__, __LINE__)

#else

#include <stdlib.h>

#define Malloc malloc
#define Calloc calloc
#define Free free

#endif

#endif /* COMMON_CMOCKA_ALLOC */
