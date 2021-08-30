// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Fujitsu */

/*
 * conn-get_uid.c -- the connection get_uid unit tests
 *
 * API covered:
 * - rpma_conn_get_uid()
 */

#include "conn-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"

/*
 * get_uid__conn_NULL -- conn NULL is invalid
 */
static void
get_uid__conn_NULL(void **unused)
{
	/* run test */
	uint32_t conn_uid = 0;
	int ret = rpma_conn_get_uid(NULL, &conn_uid);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
	assert_int_equal(conn_uid, 0);
}

/*
 * get_uid__conn_uid_NULL -- conn_uid NULL is invalid
 */
static void
get_uid__conn_uid_NULL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_get_uid(cstate->conn, NULL);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
}

/*
 * get_uid__conn_conn_uid_NULL -- conn and conn_uid NULL are invalid
 */
static void
get_uid__conn_conn_uid_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_get_uid(NULL, NULL);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
}

/*
 * get_uid__success -- happy day scenario
 */
static void
get_uid__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* run test */
	uint32_t conn_uid = 0;
	int ret = rpma_conn_get_uid(cstate->conn, &conn_uid);

	/* verify the results */
	assert_ptr_equal(ret, MOCK_OK);
	assert_int_equal(conn_uid, MOCK_CONN_UID);
}

/*
 * get_uid__success_after_disconnect - get the connection's unique ID
 * successfully after rpma_conn_disconnect().
 */
static void
get_uid__success_after_disconnect(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_disconnect, id, MOCK_CM_ID);
	will_return(rdma_disconnect, MOCK_OK);
	assert_int_equal(rpma_conn_disconnect(cstate->conn), MOCK_OK);

	/* run test */
	uint32_t conn_uid = 0;
	int ret = rpma_conn_get_uid(cstate->conn, &conn_uid);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(conn_uid, MOCK_CONN_UID);
}

/*
 * group_setup_get_uid -- prepare resources for all tests in the group
 */
static int
group_setup_get_uid(void **unused)
{
	Ibv_qp.qp_num = MOCK_CONN_UID;
	Cm_id.qp = MOCK_QP;
	return 0;
}

static const struct CMUnitTest tests_get_uid[] = {
	/* rpma_conn_get_uid() unit tests */
	cmocka_unit_test(get_uid__conn_NULL),
	cmocka_unit_test_setup_teardown(
		get_uid__conn_uid_NULL,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(get_uid__conn_conn_uid_NULL),
	cmocka_unit_test_setup_teardown(
		get_uid__success,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(
		get_uid__success_after_disconnect,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_get_uid,
			group_setup_get_uid, NULL);
}
