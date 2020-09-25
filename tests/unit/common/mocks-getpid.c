// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mocks-getpid.c -- getpid mock
 */

#include <unistd.h>

#include "cmocka_headers.h"
#include "mocks-getpid.h"

/*
 * getpid() shall be mocked only during tests otherwise ctest with COVERAGE=1
 * does not work properly
 */
bool enabled__wrap_getpid = false;

__pid_t __real_getpid(void);

/*
 * __wrap_getpid -- mock of getpid()
 */
__pid_t
__wrap_getpid()
{
	if (!enabled__wrap_getpid)
		return __real_getpid();

	int ret = mock_type(int);
	return ret;
}
