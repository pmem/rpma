// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Fujitsu */

/*
 * mocks-string.c -- string mocks
 */

#include "cmocka_headers.h"
#include "mocks-string.h"

/*
 * __wrap_strerror -- strerror() mock
 */
char *
__wrap_strerror(int errnum)
{
	check_expected(errnum);

	return mock_type(char *);
}
