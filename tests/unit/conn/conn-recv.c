// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * conn-recv.c -- the rpma_recv() unit tests
 *
 * APIs covered:
 * - rpma_recv()
 */

#include "conn-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"

/*
 * recv__conn_NULL - NULL conn is invalid
 */
static void
recv__conn_NULL(void **unused)
{
	/* run test */
	int ret = rpma_recv(NULL, MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
			MOCK_LEN, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * recv__dst_NULL_offset_not_NULL - NULL dst
 * and not NULL offset is invalid
 */
static void
recv__dst_NULL_offset_not_NULL(void **unused)
{
	/* run test */
	int ret = rpma_recv(MOCK_CONN, NULL, MOCK_LOCAL_OFFSET,
			0, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * recv__dst_NULL_len_not_NULL - NULL dst
 * and not NULL len is invalid
 */
static void
recv__dst_NULL_len_not_NULL(void **unused)
{
	/* run test */
	int ret = rpma_recv(MOCK_CONN, NULL, 0,
			MOCK_LEN, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * recv__dst_NULL_offset_len_not_NULL - NULL dst
 * and not NULL offset or len are invalid
 */
static void
recv__dst_NULL_offset_len_not_NULL(void **unused)
{
	/* run test */
	int ret = rpma_recv(MOCK_CONN, NULL, MOCK_LOCAL_OFFSET,
			MOCK_LEN, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * recv__success - happy day scenario
 */
static void
recv__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	expect_value(rpma_mr_recv, qp, MOCK_QP);
	expect_value(rpma_mr_recv, dst, MOCK_RPMA_MR_LOCAL);
	expect_value(rpma_mr_recv, offset, MOCK_LOCAL_OFFSET);
	expect_value(rpma_mr_recv, len, MOCK_LEN);
	expect_value(rpma_mr_recv, op_context, MOCK_OP_CONTEXT);
	will_return(rpma_mr_recv, MOCK_OK);

	/* run test */
	int ret = rpma_recv(cstate->conn, MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
			MOCK_LEN, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * group_setup_recv -- prepare resources for all tests in the group
 */
static int
group_setup_recv(void **unused)
{
	/* set value of QP in mock of CM ID */
	Cm_id.qp = MOCK_QP;

	return 0;
}

static const struct CMUnitTest tests_recv[] = {
	/* rpma_recv() unit tests */
	cmocka_unit_test(recv__conn_NULL),
	cmocka_unit_test(recv__dst_NULL_offset_not_NULL),
	cmocka_unit_test(recv__dst_NULL_len_not_NULL),
	cmocka_unit_test(recv__dst_NULL_offset_len_not_NULL),
	cmocka_unit_test_setup_teardown(recv__success,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_recv, group_setup_recv, NULL);
}
