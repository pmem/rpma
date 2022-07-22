// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * mocks-glibc.c -- glibc mocks
 */

#include <unistd.h>
#include <sys/syscall.h>

#include "cmocka_headers.h"
#include "mocks-glibc.h"

/*
 * syscall() shall be mocked only during tests otherwise ctest with TESTS_COVERAGE=1
 * does not work properly
 */
bool enabled__wrap_syscall = false;

long __real_syscall(long number);

/*
 * __wrap_syscall -- mock of syscall()
 */
long
__wrap_syscall(long number)
{
	if (!enabled__wrap_syscall || number != SYS_gettid)
		return __real_syscall(number);

	return mock_type(long);
}
