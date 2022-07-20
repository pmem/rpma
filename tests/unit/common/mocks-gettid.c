// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * mocks-gettid.c -- gettid mock
 */

#include <unistd.h>

#include "cmocka_headers.h"
#include "mocks-gettid.h"

/*
 * gettid() shall be mocked only during tests otherwise ctest with TESTS_COVERAGE=1
 * does not work properly
 */
bool enabled__wrap_gettid = false;

__pid_t __real_gettid(void);

/*
 * __wrap_gettid -- mock of gettid()
 */
__pid_t
__wrap_gettid()
{
	if (!enabled__wrap_gettid)
		return __real_gettid();

	int ret = mock_type(int);
	return ret;
}
