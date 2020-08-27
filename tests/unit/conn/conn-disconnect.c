// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn-test-disconnect.c -- the connection disconnect unit tests
 *
 * API covered:
 * - rpma_conn_disconnect()
 */

#include "mocks-rdma_cm.h"
#include "conn-common.h"

/*
 * disconnect__conn_NULL - NULL conn is invalid
 */
static void
disconnect__conn_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_disconnect(NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * disconnect__rdma_disconnect_EINVAL -
 * rdma_disconnect() fails with EINVAL
 */
static void
disconnect__rdma_disconnect_EINVAL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_disconnect, id, MOCK_CM_ID);
	will_return(rdma_disconnect, EINVAL);

	/* run test */
	int ret = rpma_conn_disconnect(cstate->conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EINVAL);
}

/*
 * disconnect__success - happy day scenario
 */
static void
disconnect__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_disconnect, id, MOCK_CM_ID);
	will_return(rdma_disconnect, MOCK_OK);

	/* run test */
	int ret = rpma_conn_disconnect(cstate->conn);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

const static struct CMUnitTest tests_disconnect[] = {
	/* rpma_conn_disconnect() unit tests */
	cmocka_unit_test(disconnect__conn_NULL),
	cmocka_unit_test_setup_teardown(
		disconnect__rdma_disconnect_EINVAL,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(
		disconnect__success,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_disconnect, NULL, NULL);
}
