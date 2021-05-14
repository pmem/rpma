// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn-get_completion_fd.c -- the connection get_completion_fd unit tests
 *
 * API covered:
 * - rpma_conn_get_completion_fd()
 */

#include "conn-common.h"
#include "mocks-ibverbs.h"

/*
 * get_completion_fd__conn_NULL -- conn NULL is invalid
 */
static void
get_completion_fd__conn_NULL(void **unused)
{
	/* run test */
	int fd = 0;
	int ret = rpma_conn_get_completion_fd(NULL, &fd);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_int_equal(fd, 0);
}

/*
 * get_completion_fd__fd_NULL -- fd NULL is invalid
 */
static void
get_completion_fd__fd_NULL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_get_completion_fd(cstate->conn, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_completion_fd__success -- happy day scenario
 */
static void
get_completion_fd__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	will_return(rpma_cq_get_fd, MOCK_COMPLETION_FD);

	/* run test */
	int fd = 0;
	int ret = rpma_conn_get_completion_fd(cstate->conn, &fd);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_int_equal(fd, MOCK_COMPLETION_FD);
}

static const struct CMUnitTest tests_get_completion_fd[] = {
	/* rpma_conn_get_completion_fd() unit tests */
	cmocka_unit_test(get_completion_fd__conn_NULL),
	cmocka_unit_test_setup_teardown(
		get_completion_fd__fd_NULL,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(
		get_completion_fd__success,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_get_completion_fd,
			NULL, NULL);
}
