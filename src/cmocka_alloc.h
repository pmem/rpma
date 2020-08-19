/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

#ifndef COMMON_CMOCKA_ALLOC
#define COMMON_CMOCKA_ALLOC

#include <stddef.h>

extern void *_test_malloc(const size_t size, const char *file, const int line);
extern void *_test_calloc(const size_t number_of_elements, const size_t size,
		const char *file, const int line);
extern void _test_free(void *const ptr, const char *file, const int line);

#define malloc(size) _test_malloc(size, __FILE__, __LINE__)
#define calloc(num, size) _test_calloc(num, size, __FILE__, __LINE__)
#define free(ptr) _test_free(ptr, __FILE__, __LINE__)

#endif /* COMMON_CMOCKA_ALLOC */
