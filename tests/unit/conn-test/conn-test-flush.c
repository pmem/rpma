/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn-test-flush.c -- the rpma_flush() unit tests
 *
 * APIs covered:
 * - rpma_flush()
 */

#include "conn-test-common.h"

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
test_flush__success(void **unused)
{
	struct conn_test_state *cstate;
	int ret = conn_setup((void **)&cstate);
	assert_int_equal(ret, 0);
	assert_non_null(cstate->conn);

	/* run test */
	ret = rpma_flush(cstate->conn, MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
			MOCK_LEN, RPMA_FLUSH_TYPE_VISIBILITY,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);

	ret = conn_teardown((void **)&cstate);
	assert_int_equal(ret, 0);
}

const struct CMUnitTest tests_flush[] = {
	/* rpma_read() unit tests */
	cmocka_unit_test(test_flush__conn_NULL),
	cmocka_unit_test(test_flush__dst_NULL),
	cmocka_unit_test(test_flush__flags_0),
	cmocka_unit_test(test_flush__conn_dst_NULL_flags_0),
	cmocka_unit_test(test_flush__success),
	cmocka_unit_test(NULL)
};
