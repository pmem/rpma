/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * utils-conn_event_2str.c -- a unit test for rpma_utils_conn_event_2str()
 */

#include "cmocka_headers.h"
#include "librpma.h"

/*
 * event_2str_test_CONN_UNDEFINED - sanity test for
 * rpma_utils_conn_event_2str()
 */
static void
event_2str_test_CONN_UNDEFINED(void **unused)
{
	assert_string_equal(rpma_utils_conn_event_2str(RPMA_CONN_UNDEFINED),
	"Undefined connection event");
}

/*
 * event_2str_test_CONN_ESTABLISHED - sanity test for
 * rpma_utils_conn_event_2str()
 */
static void
event_2str_test_CONN_ESTABLISHED(void **unused)
{
	assert_string_equal(rpma_utils_conn_event_2str(RPMA_CONN_ESTABLISHED),
	"Connection established");
}

/*
 * event_2str_test_CONN_CLOSED - sanity test for
 * rpma_utils_conn_event_2str()
 */
static void
event_2str_test_CONN_CLOSED(void **unused)
{
	assert_string_equal(rpma_utils_conn_event_2str(RPMA_CONN_CLOSED),
	"Connection closed");
}

/*
 * event_2str_test_CONN_LOST - sanity test for
 * rpma_utils_conn_event_2str()
 */
static void
event_2str_test_CONN_LOST(void **unused)
{
	assert_string_equal(rpma_utils_conn_event_2str(RPMA_CONN_LOST),
	"Connection lost");
}

/*
 * event_2str_test_CONN_UNKOWN - sanity test for
 * rpma_utils_conn_event_2str()
 */
static void
event_2str_test_CONN_UNKNOWN(void **unused)
{
	assert_string_equal(rpma_utils_conn_event_2str(RPMA_E_UNKNOWN),
	"Unknown connection event");
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_utils_conn_event_2str() unit tests */
		cmocka_unit_test(event_2str_test_CONN_UNDEFINED),
		cmocka_unit_test(event_2str_test_CONN_ESTABLISHED),
		cmocka_unit_test(event_2str_test_CONN_CLOSED),
		cmocka_unit_test(event_2str_test_CONN_LOST),
		cmocka_unit_test(event_2str_test_CONN_UNKNOWN),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
