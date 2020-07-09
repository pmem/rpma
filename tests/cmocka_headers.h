/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * cmocka_headers.h -- include all headers required by cmocka
 */

#ifndef LIBRPMA_CMOCKA_H
#define LIBRPMA_CMOCKA_H 1

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#define cmocka_EOF { NULL, NULL, NULL, NULL, NULL }

static inline size_t
cmocks_tests_num(const struct CMUnitTest * const tests)
{
	size_t i = 0;
	while (tests[i].test_func != NULL)
		++i;

	return i - 1;
}

#endif /* LIBRPMA_CMOCKA_H */
