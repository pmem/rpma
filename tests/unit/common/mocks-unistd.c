// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * mocks-unistd.c -- unistd mocks
 */

#include <errno.h>
#include <unistd.h>

#include "cmocka_headers.h"
#include "mocks-unistd.h"

static int unistd_mocks_enabled;

long __real_sysconf(int name);

/*
 * enable_unistd_mocks -- enable unistd mocks
 */
void
enable_unistd_mocks(void)
{
	unistd_mocks_enabled = 1;
}

/*
 * disable_unistd_mocks -- disable unistd mocks
 */
void
disable_unistd_mocks(void)
{
	unistd_mocks_enabled = 0;
}

/*
 * __wrap_sysconf -- sysconf() mock
 */
long
__wrap_sysconf(int name)
{
	if (unistd_mocks_enabled == 0)
		return __real_sysconf(name);

	assert_int_equal(name, _SC_PAGESIZE);
	int err = mock_type(int);
	if (err) {
		errno = err;
		return -1;
	}
	return PAGESIZE;
}
