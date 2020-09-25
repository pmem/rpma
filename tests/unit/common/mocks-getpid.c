// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mocks-getpid.c -- getpid mock
 */

#include "cmocka_headers.h"
#include "mocks-getpid.h"

/*
 * __wrap_getpid -- getpid() mock
 */
__pid_t
__wrap_getpid()
{
	int ret = mock_type(int);
	return ret;
}
