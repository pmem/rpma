/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn-test-read.c -- the rpma_read() unit tests
 *
 * APIs covered:
 * - rpma_read()
 */

#include "conn-test-common.h"

/*
 * test_read__conn_NULL - NULL conn is invalid
 */
static void
test_read__conn_NULL(void **unused)
{
	/* run test */
	int ret = rpma_read(NULL, MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
				MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
				MOCK_LEN, MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_read__dst_NULL - NULL dst is invalid
 */
static void
test_read__dst_NULL(void **unused)
{
	/* run test */
	int ret = rpma_read(MOCK_CONN, NULL, MOCK_LOCAL_OFFSET,
				MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
				MOCK_LEN, MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_read__src_NULL - NULL src is invalid
 */
static void
test_read__src_NULL(void **unused)
{
	/* run test */
	int ret = rpma_read(MOCK_CONN, MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
				NULL, MOCK_REMOTE_OFFSET,
				MOCK_LEN, MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_read__flags_0 - flags == 0 is invalid
 */
static void
test_read__flags_0(void **unused)
{
	/* run test */
	int ret = rpma_read(MOCK_CONN, MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
				MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
				MOCK_LEN, 0, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_read__conn_dst_src_NULL_flags_0 - NULL conn, dst, src
 * and flags == 0 are invalid
 */
static void
test_read__conn_dst_src_NULL_flags_0(void **unused)
{
	/* run test */
	int ret = rpma_read(NULL, NULL, MOCK_LOCAL_OFFSET,
				NULL, MOCK_REMOTE_OFFSET,
				MOCK_LEN, 0, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_read__success - happy day scenario
 */
static void
test_read__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	expect_value(rpma_mr_read, qp, MOCK_QP);
	expect_value(rpma_mr_read, dst, MOCK_RPMA_MR_LOCAL);
	expect_value(rpma_mr_read, dst_offset, MOCK_LOCAL_OFFSET);
	expect_value(rpma_mr_read, src, MOCK_RPMA_MR_REMOTE);
	expect_value(rpma_mr_read, src_offset, MOCK_REMOTE_OFFSET);
	expect_value(rpma_mr_read, len, MOCK_LEN);
	expect_value(rpma_mr_read, flags, MOCK_FLAGS);
	expect_value(rpma_mr_read, op_context, MOCK_OP_CONTEXT);
	will_return(rpma_mr_read, MOCK_OK);

	/* run test */
	int ret = rpma_read(cstate->conn, MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
				MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
				MOCK_LEN, MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * group_setup_read -- prepare resources for all tests in the group
 */
int
group_setup_read(void **unused)
{
	/* set value of QP in mock of CM ID */
	Cm_id.qp = MOCK_QP;

	return 0;
}

const struct CMUnitTest tests_read[] = {
	/* rpma_read() unit tests */
	cmocka_unit_test(test_read__conn_NULL),
	cmocka_unit_test(test_read__dst_NULL),
	cmocka_unit_test(test_read__src_NULL),
	cmocka_unit_test(test_read__flags_0),
	cmocka_unit_test(test_read__conn_dst_src_NULL_flags_0),
	cmocka_unit_test_setup_teardown(test_read__success,
		conn_setup, conn_teardown),
	cmocka_unit_test(NULL)
};
