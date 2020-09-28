// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * error.c -- unit tests for error-handling rpma-err module
 *
 * APIs covered:
 * - rpma_err_get_provider_error()
 * - rpma_err_2str()
 */

#include "cmocka_headers.h"
#include "librpma.h"
#include "rpma_err.h"

/*
 * err_get_provider_error - sanity test for rpma_err_get_provider_error()
 */
static void
err_get_provider_error(void **unused)
{
	Rpma_provider_error = 1234; /* any random value */
	assert_int_equal(rpma_err_get_provider_error(), Rpma_provider_error);
	Rpma_provider_error = 5678; /* any random value */
	assert_int_equal(rpma_err_get_provider_error(), Rpma_provider_error);
}

/*
 * err_2str__E_NOSUPP - sanity test for rpma_err_2str()
 */
static void
err_2str__E_NOSUPP(void **unused)
{
	assert_string_equal(rpma_err_2str(RPMA_E_NOSUPP), "Not supported");
}

/*
 * err_2str__E_PROVIDER - sanity test for rpma_err_2str()
 */
static void
err_2str__E_PROVIDER(void **unused)
{
	assert_string_equal(rpma_err_2str(RPMA_E_PROVIDER),
				"Provider error occurred");
}

/*
 * err_2str__E_NOMEM - sanity test for rpma_err_2str()
 */
static void
err_2str__E_NOMEM(void **unused)
{
	assert_string_equal(rpma_err_2str(RPMA_E_NOMEM), "Out of memory");
}

/*
 * err_2str__E_INVAL - sanity test for rpma_err_2str()
 */
static void
err_2str__E_INVAL(void **unused)
{
	assert_string_equal(rpma_err_2str(RPMA_E_INVAL), "Invalid argument");
}

/*
 * err_2str__E_NO_COMPLETION - sanity test for rpma_err_2str()
 */
static void
err_2str__E_NO_COMPLETION(void **unused)
{
	assert_string_equal(rpma_err_2str(RPMA_E_NO_COMPLETION),
				"No next completion available");
}

/*
 * err_2str__E_NO_NEXT - sanity test for rpma_err_2str()
 */
static void
err_2str__E_NO_NEXT(void **unused)
{
	assert_string_equal(rpma_err_2str(RPMA_E_NO_EVENT),
				"No next event available");
}

/*
 * err_2str__E_UNKOWN - sanity test for rpma_err_2str()
 */
static void
err_2str__E_UNKNOWN(void **unused)
{
	assert_string_equal(rpma_err_2str(RPMA_E_UNKNOWN), "Unknown error");
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(err_get_provider_error),
		cmocka_unit_test(err_2str__E_NOSUPP),
		cmocka_unit_test(err_2str__E_PROVIDER),
		cmocka_unit_test(err_2str__E_NOMEM),
		cmocka_unit_test(err_2str__E_INVAL),
		cmocka_unit_test(err_2str__E_NO_COMPLETION),
		cmocka_unit_test(err_2str__E_NO_NEXT),
		cmocka_unit_test(err_2str__E_UNKNOWN),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
