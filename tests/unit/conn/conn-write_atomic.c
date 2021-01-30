// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn-write_atomic.c -- the rpma_write_atomic() unit tests
 *
 * APIs covered:
 * - rpma_write_atomic()
 */

#include "conn-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"

/*
 * write_atomic__conn_NULL -- NULL conn is invalid
 */
static void
write_atomic__conn_NULL(void **unused)
{
	/* run test */
	int ret = rpma_write_atomic(NULL, MOCK_RPMA_MR_REMOTE,
			MOCK_OFFSET_ALIGNED, MOCK_RPMA_MR_LOCAL,
			MOCK_LOCAL_OFFSET, MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write_atomic__dst_NULL -- NULL dst is invalid
 */
static void
write_atomic__dst_NULL(void **unused)
{
	/* run test */
	int ret = rpma_write_atomic(MOCK_CONN, NULL, MOCK_OFFSET_ALIGNED,
			MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write_atomic__src_NULL -- NULL src is invalid
 */
static void
write_atomic__src_NULL(void **unused)
{
	/* run test */
	int ret = rpma_write_atomic(MOCK_CONN, MOCK_RPMA_MR_REMOTE,
			MOCK_OFFSET_ALIGNED, NULL, MOCK_LOCAL_OFFSET,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write_atomic__flags_0 -- flags == 0 is invalid
 */
static void
write_atomic__flags_0(void **unused)
{
	/* run test */
	int ret = rpma_write_atomic(MOCK_CONN, MOCK_RPMA_MR_REMOTE,
			MOCK_OFFSET_ALIGNED, MOCK_RPMA_MR_LOCAL,
			MOCK_LOCAL_OFFSET, 0, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write_atomic__dst_offset_unaligned -- the unaligned dst_offset is invalid
 */
static void
write_atomic__dst_offset_unaligned(void **unused)
{
	/* run test */
	int ret = rpma_write_atomic(MOCK_CONN, MOCK_RPMA_MR_REMOTE,
			MOCK_REMOTE_OFFSET, MOCK_RPMA_MR_LOCAL,
			MOCK_LOCAL_OFFSET, MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write_atomic__conn_dst_src_NULL_flags_0_dst_offset_unaligned -- NULL conn,
 * dst, src, flags == 0 and an unaligned dst_offset are invalid
 */
static void
write_atomic__conn_dst_src_NULL_flags_0_dst_offset_unaligned(void **unused)
{
	/* run test */
	int ret = rpma_write_atomic(NULL, NULL, MOCK_REMOTE_OFFSET,
			NULL, MOCK_LOCAL_OFFSET,
			0, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write_atomic__success -- happy day scenario
 */
static void
write_atomic__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	expect_value(rpma_mr_write, qp, MOCK_QP);
	expect_value(rpma_mr_write, dst, MOCK_RPMA_MR_REMOTE);
	expect_value(rpma_mr_write, dst_offset, MOCK_OFFSET_ALIGNED);
	expect_value(rpma_mr_write, src, MOCK_RPMA_MR_LOCAL);
	expect_value(rpma_mr_write, src_offset, MOCK_LOCAL_OFFSET);
	expect_value(rpma_mr_write, len, RPMA_ATOMIC_WRITE_ALIGNMENT);
	expect_value(rpma_mr_write, flags, MOCK_FLAGS);
	expect_value(rpma_mr_write, operation, IBV_WR_RDMA_WRITE);
	expect_value(rpma_mr_write, imm, 0);
	expect_value(rpma_mr_write, op_context, MOCK_OP_CONTEXT);
	expect_value(rpma_mr_write, fence, MOCK_FENCE);
	will_return(rpma_mr_write, MOCK_OK);

	/* run test */
	int ret = rpma_write_atomic(cstate->conn,
			MOCK_RPMA_MR_REMOTE, MOCK_OFFSET_ALIGNED,
			MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * group_setup_write_atomic -- prepare resources for all tests in the group
 */
int
group_setup_write_atomic(void **unused)
{
	/* set value of QP in mock of CM ID */
	Cm_id.qp = MOCK_QP;

	return 0;
}

static const struct CMUnitTest tests_write_atomic[] = {
	/* rpma_write_atomic() unit tests */
	cmocka_unit_test(write_atomic__conn_NULL),
	cmocka_unit_test(write_atomic__dst_NULL),
	cmocka_unit_test(write_atomic__src_NULL),
	cmocka_unit_test(write_atomic__flags_0),
	cmocka_unit_test(write_atomic__dst_offset_unaligned),
	cmocka_unit_test(
		write_atomic__conn_dst_src_NULL_flags_0_dst_offset_unaligned),
	cmocka_unit_test_setup_teardown(write_atomic__success,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_write_atomic,
			group_setup_write_atomic, NULL);
}
