/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * error-test.c -- unit tests for error-handling rpma-err module
 */

#include "cmocka_headers.h"
#include "librpma.h"
#include "rpma_err.h"

#define EMPTY_STRING	""

/*
 * err_get_msg_test - sanity test for rpma_err_get_msg()
 */
static void
err_get_msg_test(void **unused)
{
	assert_string_equal(rpma_err_get_msg(), EMPTY_STRING);
}

/*
 * err_get_provider_error_test - sanity test for rpma_err_get_provider_error()
 */
static void
err_get_provider_error_test(void **unused)
{
	assert_int_equal(rpma_err_get_provider_error(), Rpma_provider_error);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(err_get_msg_test),
		cmocka_unit_test(err_get_provider_error_test),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
