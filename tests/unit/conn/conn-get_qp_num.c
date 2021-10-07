// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Fujitsu */

/*
 * conn-get_qp_num.c -- the connection get_qp_num unit tests
 *
 * API covered:
 * - rpma_conn_get_qp_num()
 */

#include "conn-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"

/*
 * get_qp_num__conn_NULL -- conn NULL is invalid
 */
static void
get_qp_num__conn_NULL(void **unused)
{
	/* run test */
	uint32_t qp_num = 0;
	int ret = rpma_conn_get_qp_num(NULL, &qp_num);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
	assert_int_equal(qp_num, 0);
}

/*
 * get_qp_num__qp_num_NULL -- qp_num NULL is invalid
 */
static void
get_qp_num__qp_num_NULL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_get_qp_num(cstate->conn, NULL);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
}

/*
 * get_qp_num__conn_qp_num_NULL -- conn and qp_num NULL are invalid
 */
static void
get_qp_num__conn_qp_num_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_get_qp_num(NULL, NULL);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
}

/*
 * get_qp_num__success -- happy day scenario
 */
static void
get_qp_num__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* run test */
	uint32_t qp_num = 0;
	int ret = rpma_conn_get_qp_num(cstate->conn, &qp_num);

	/* verify the results */
	assert_ptr_equal(ret, MOCK_OK);
	assert_int_equal(qp_num, MOCK_QP_NUM);
}

/*
 * get_qp_num__success_after_disconnect - get the connection's qp_num
 * successfully after rpma_conn_disconnect().
 */
static void
get_qp_num__success_after_disconnect(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_disconnect, id, MOCK_CM_ID);
	will_return(rdma_disconnect, MOCK_OK);

	/* run test */
	int ret = rpma_conn_disconnect(cstate->conn);
	assert_int_equal(ret, MOCK_OK);
	uint32_t qp_num = 0;
	ret = rpma_conn_get_qp_num(cstate->conn, &qp_num);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(qp_num, MOCK_QP_NUM);
}

/*
 * group_setup_get_qp_num -- prepare resources for all tests in the group
 */
static int
group_setup_get_qp_num(void **unused)
{
	Ibv_qp.qp_num = MOCK_QP_NUM;
	Cm_id.qp = MOCK_QP;
	return 0;
}

static const struct CMUnitTest tests_get_qp_num[] = {
	/* rpma_conn_get_qp_num() unit tests */
	cmocka_unit_test(get_qp_num__conn_NULL),
	cmocka_unit_test_setup_teardown(
		get_qp_num__qp_num_NULL,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(get_qp_num__conn_qp_num_NULL),
	cmocka_unit_test_setup_teardown(
		get_qp_num__success,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(
		get_qp_num__success_after_disconnect,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_get_qp_num,
			group_setup_get_qp_num, NULL);
}
