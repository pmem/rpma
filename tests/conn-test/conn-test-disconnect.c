/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn-test-disconnect.c -- the connection disconnect unit tests
 *
 * API covered:
 * - rpma_conn_disconnect()
 */

#include "conn-test-common.h"

/*
 * disconnect_test_conn_NULL - NULL conn is invalid
 */
static void
disconnect_test_conn_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_disconnect(NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * disconnect_test_rdma_disconnect_EINVAL -
 * rdma_disconnect() fails with EINVAL
 */
static void
disconnect_test_rdma_disconnect_EINVAL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_disconnect, id, MOCK_CM_ID);
	will_return(rdma_disconnect, EINVAL);

	/* run test */
	int ret = rpma_conn_disconnect(cstate->conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(errno, EINVAL);
}

/*
 * disconnect_test_success - happy day scenario
 */
static void
disconnect_test_success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_disconnect, id, MOCK_CM_ID);
	will_return(rdma_disconnect, MOCK_OK);

	/* run test */
	int ret = rpma_conn_disconnect(cstate->conn);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

const struct CMUnitTest tests_disconnect[] = {
	/* rpma_conn_disconnect() unit tests */
	cmocka_unit_test(disconnect_test_conn_NULL),
	cmocka_unit_test_setup_teardown(
		disconnect_test_rdma_disconnect_EINVAL,
		conn_setup, conn_teardown),
	cmocka_unit_test_setup_teardown(
		disconnect_test_success,
		conn_setup, conn_teardown),
	cmocka_unit_test(NULL)
};
