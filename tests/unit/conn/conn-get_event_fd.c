/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn-test-get_event_fd.c -- the connection get_event_fd unit tests
 *
 * API covered:
 * - rpma_conn_get_event_fd()
 */

#include "conn-common.h"

/*
 * get_event_fd__conn_NULL -- conn NULL is invalid
 */
static void
get_event_fd__conn_NULL(void **unused)
{
	/* run test */
	int fd = 0;
	int ret = rpma_conn_get_event_fd(NULL, &fd);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
	assert_int_equal(fd, 0);
}

/*
 * get_event_fd__fd_NULL -- fd NULL is invalid
 */
static void
get_event_fd__fd_NULL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_get_event_fd(cstate->conn, NULL);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
}

/*
 * get_event_fd__conn_fd_NULL -- conn and fd NULL are invalid
 */
static void
get_event_fd__conn_fd_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_get_event_fd(NULL, NULL);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
}

/*
 * get_event_fd__success -- happy day scenario
 */
static void
get_event_fd__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* run test */
	int fd = 0;
	int ret = rpma_conn_get_event_fd(cstate->conn, &fd);

	/* verify the results */
	assert_ptr_equal(ret, 0);
	assert_int_equal(fd, MOCK_FD);
}

/*
 * group_setup_get_event_fd -- prepare resources for all tests in the group
 */
static int
group_setup_get_event_fd(void)
{
	Evch.fd = MOCK_FD;
	return 0;
}

const struct CMUnitTest tests_get_event_fd[] = {
	/* rpma_conn_get_event_fd() unit tests */
	cmocka_unit_test(get_event_fd__conn_NULL),
	cmocka_unit_test_setup_teardown(
		get_event_fd__fd_NULL,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(get_event_fd__conn_fd_NULL),
	cmocka_unit_test_setup_teardown(
		get_event_fd__success,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	/* prepare resources for all tests in the group */
	group_setup_get_event_fd();

	return cmocka_run_group_tests(tests_get_event_fd, NULL, NULL);
}
