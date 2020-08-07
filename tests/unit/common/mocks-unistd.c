/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mocks-unistd.c -- unistd mocks
 */

#include <errno.h>
#include <unistd.h>

#include "cmocka_headers.h"
#include "mocks-unistd.h"

/*
 * __wrap_sysconf -- sysconf() mock
 */
long
__wrap_sysconf(int name)
{
	assert_int_equal(name, _SC_PAGESIZE);
	int err = mock_type(int);
	if (err) {
		errno = err;
		return -1;
	}
	return PAGESIZE;
}
