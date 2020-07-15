// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * test_mocks.h -- definitions required by integration tests
 */

#ifndef EXAMPLES_TEST_MOCKS
#define EXAMPLES_TEST_MOCKS

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#define malloc(size) _test_malloc(size, __FILE__, __LINE__)
#define calloc(num, size) _test_calloc(num, size, __FILE__, __LINE__)
#define free(ptr) _test_free(ptr, __FILE__, __LINE__)

#endif /* EXAMPLES_TEST_MOCKS */
