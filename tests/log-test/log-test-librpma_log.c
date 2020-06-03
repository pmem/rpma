/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * log-test-librpma_log_flags.c -- to enable asserts tests in unit tests
 */

#ifdef UNIT_TESTING
extern void mock_assert(const int result, const char *const expression, \
		const char *const file, const int line);
#undef assert
#define assert(expression) \
	mock_assert((int)(expression), #expression, __FILE__, __LINE__);
#endif
#include "../common/librpma_log.c"
