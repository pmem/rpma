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
 * conn_event_2str__CONN_UNDEFINED - sanity test for
 * rpma_utils_conn_event_2str()
 */
static void
conn_event_2str__CONN_UNDEFINED(void **unused)
{
	assert_string_equal(rpma_utils_conn_event_2str(RPMA_CONN_UNDEFINED),
	"Undefined connection event");
}

/*
 * conn_event_2str__CONN_ESTABLISHED - sanity test for
 * rpma_utils_conn_event_2str()
 */
static void
conn_event_2str__CONN_ESTABLISHED(void **unused)
{
	assert_string_equal(rpma_utils_conn_event_2str(RPMA_CONN_ESTABLISHED),
	"Connection established");
}

/*
 * conn_event_2str__CONN_CLOSED - sanity test for
 * rpma_utils_conn_event_2str()
 */
static void
conn_event_2str__CONN_CLOSED(void **unused)
{
	assert_string_equal(rpma_utils_conn_event_2str(RPMA_CONN_CLOSED),
	"Connection closed");
}

/*
 * conn_event_2str__CONN_LOST - sanity test for
 * rpma_utils_conn_event_2str()
 */
static void
conn_event_2str__CONN_LOST(void **unused)
{
	assert_string_equal(rpma_utils_conn_event_2str(RPMA_CONN_LOST),
	"Connection lost");
}

/*
 * conn_event_2str__CONN_UNKOWN - sanity test for
 * rpma_utils_conn_event_2str()
 */
static void
conn_event_2str__CONN_UNKNOWN(void **unused)
{
	assert_string_equal(rpma_utils_conn_event_2str(RPMA_E_UNKNOWN),
	"Unknown connection event");
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_utils_conn_event_2str() unit tests */
		cmocka_unit_test(conn_event_2str__CONN_UNDEFINED),
		cmocka_unit_test(conn_event_2str__CONN_ESTABLISHED),
		cmocka_unit_test(conn_event_2str__CONN_CLOSED),
		cmocka_unit_test(conn_event_2str__CONN_LOST),
		cmocka_unit_test(conn_event_2str__CONN_UNKNOWN),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
