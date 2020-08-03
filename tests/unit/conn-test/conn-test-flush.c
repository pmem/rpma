// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn-test-flush.c -- the rpma_flush() unit tests
 *
 * APIs covered:
 * - rpma_flush()
 */

#include "conn-test-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rpma-flush.h"

/*
 * test_flush__conn_NULL - NULL conn is invalid
 */
static void
test_flush__conn_NULL(void **unused)
{
	/* run test */
	int ret = rpma_flush(NULL, MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
			MOCK_LEN, RPMA_FLUSH_TYPE_VISIBILITY,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_flush__dst_NULL - NULL dst is invalid
 */
static void
test_flush__dst_NULL(void **unused)
{
	/* run test */
	int ret = rpma_flush(MOCK_CONN, NULL, MOCK_REMOTE_OFFSET,
			MOCK_LEN, RPMA_FLUSH_TYPE_VISIBILITY,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_flush__flags_0 - flags == 0 is invalid
 */
static void
test_flush__flags_0(void **unused)
{
	/* run test */
	int ret = rpma_flush(MOCK_CONN, MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
			MOCK_LEN, RPMA_FLUSH_TYPE_VISIBILITY,
			0, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_flush__conn_dst_NULL_flags_0 - NULL conn, dst
 * and flags == 0 are invalid
 */
static void
test_flush__conn_dst_NULL_flags_0(void **unused)
{
	/* run test */
	int ret = rpma_flush(NULL, NULL, MOCK_REMOTE_OFFSET,
			MOCK_LEN, RPMA_FLUSH_TYPE_VISIBILITY,
			0, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_flush__success - happy day scenario
 */
static void
test_flush__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	expect_value(rpma_flush_mock_do, qp, MOCK_QP);
	expect_value(rpma_flush_mock_do, flush, MOCK_FLUSH);
	expect_value(rpma_flush_mock_do, dst, MOCK_RPMA_MR_REMOTE);
	expect_value(rpma_flush_mock_do, dst_offset, MOCK_REMOTE_OFFSET);
	expect_value(rpma_flush_mock_do, len, MOCK_LEN);
	expect_value(rpma_flush_mock_do, flags, MOCK_FLAGS);
	expect_value(rpma_flush_mock_do, op_context, MOCK_OP_CONTEXT);

	/* run test */
	int ret = rpma_flush(cstate->conn, MOCK_RPMA_MR_REMOTE,
			MOCK_REMOTE_OFFSET, MOCK_LEN,
			RPMA_FLUSH_TYPE_VISIBILITY,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

const struct CMUnitTest tests_flush[] = {
	/* rpma_read() unit tests */
	cmocka_unit_test(test_flush__conn_NULL),
	cmocka_unit_test(test_flush__dst_NULL),
	cmocka_unit_test(test_flush__flags_0),
	cmocka_unit_test(test_flush__conn_dst_NULL_flags_0),
	cmocka_unit_test_setup_teardown(test_flush__success,
		conn_setup, conn_teardown),
	cmocka_unit_test(NULL)
};
