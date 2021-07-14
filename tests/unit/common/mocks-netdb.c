// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Fujitsu */

/*
 * mocks-netdb.c -- netdb mocks
 */

#include "cmocka_headers.h"
#include "mocks-netdb.h"

/*
 * __wrap_gai_strerror -- gai_strerror() mock
 */
const char *
__wrap_gai_strerror(int errcode)
{
	check_expected(errcode);

	return mock_type(const char *);
}
