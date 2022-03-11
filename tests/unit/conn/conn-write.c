// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * conn-write.c -- the rpma_write() unit tests
 *
 * APIs covered:
 * - rpma_write()
 */

#include "conn-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"

/*
 * write__conn_NULL -- NULL conn is invalid
 */
static void
write__conn_NULL(void **unused)
{
	/* run test */
	int ret = rpma_write(NULL, MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
				MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
				MOCK_LEN, MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write__dst_NULL -- NULL dst is invalid
 */
static void
write__dst_NULL(void **unused)
{
	/* run test */
	int ret = rpma_write(MOCK_CONN, NULL, MOCK_REMOTE_OFFSET,
				MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
				MOCK_LEN, MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write__src_NULL_dst_not_NULL -- NULL src and not NULL dst are invalid
 */
static void
write__src_NULL_dst_not_NULL(void **unused)
{
	/* run test */
	int ret = rpma_write(MOCK_CONN, MOCK_RPMA_MR_REMOTE, 0, NULL, 0, 0,
				MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write__src_NULL_dst_offset_not_NULL -- NULL src and
 * dst_offset != 0 are invalid
 */
static void
write__src_NULL_dst_offset_not_NULL(void **unused)
{
	/* run test */
	int ret = rpma_write(MOCK_CONN, NULL, MOCK_REMOTE_OFFSET, NULL, 0, 0,
				MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write__src_NULL_src_offset_not_NULL -- NULL src and
 * src_offset != 0 are invalid
 */
static void
write__src_NULL_src_offset_not_NULL(void **unused)
{
	/* run test */
	int ret = rpma_write(MOCK_CONN, NULL, 0, NULL, MOCK_LOCAL_OFFSET,
				0, MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write__src_NULL_len_not_NULL -- NULL src and len != 0 are invalid
 */
static void
write__src_NULL_len_not_NULL(void **unused)
{
	/* run test */
	int ret = rpma_write(MOCK_CONN, NULL, 0, NULL, 0, MOCK_LEN,
				MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write__src_NULL_dst_offsets_len_not_NULL -- NULL src is invalid
 * and not NULL dst, dst_offset, src_offset or len
 */
static void
write__src_NULL_dst_offsets_len_not_NULL(void **unused)
{
	/* run test */
	int ret = rpma_write(MOCK_CONN, MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
				NULL, MOCK_LOCAL_OFFSET,
				MOCK_LEN, MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write__flags_0 -- flags == 0 is invalid
 */
static void
write__flags_0(void **unused)
{
	/* run test */
	int ret = rpma_write(MOCK_CONN, MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
				MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
				MOCK_LEN, 0, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write__conn_dst_NULL_flags_0 -- NULL conn, dst
 * and flags == 0 are invalid
 */
static void
write__conn_dst_NULL_flags_0(void **unused)
{
	/* run test */
	int ret = rpma_write(NULL, NULL, MOCK_REMOTE_OFFSET,
				MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
				MOCK_LEN, 0, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write__success -- happy day scenario
 */
static void
write__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	expect_value(rpma_mr_write, qp, MOCK_QP);
	expect_value(rpma_mr_write, dst, MOCK_RPMA_MR_REMOTE);
	expect_value(rpma_mr_write, dst_offset, MOCK_REMOTE_OFFSET);
	expect_value(rpma_mr_write, src, MOCK_RPMA_MR_LOCAL);
	expect_value(rpma_mr_write, src_offset, MOCK_LOCAL_OFFSET);
	expect_value(rpma_mr_write, len, MOCK_LEN);
	expect_value(rpma_mr_write, flags, MOCK_FLAGS);
	expect_value(rpma_mr_write, operation, IBV_WR_RDMA_WRITE);
	expect_value(rpma_mr_write, imm, 0);
	expect_value(rpma_mr_write, op_context, MOCK_OP_CONTEXT);
	will_return(rpma_mr_write, MOCK_OK);

	/* run test */
	int ret = rpma_write(cstate->conn,
				MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
				MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
				MOCK_LEN, MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * group_setup_write -- prepare resources for all tests in the group
 */
static int
group_setup_write(void **unused)
{
	/* set value of QP in mock of CM ID */
	Cm_id.qp = MOCK_QP;

	return 0;
}

static const struct CMUnitTest tests_write[] = {
	/* rpma_read() unit tests */
	cmocka_unit_test(write__conn_NULL),
	cmocka_unit_test(write__dst_NULL),
	cmocka_unit_test(write__src_NULL_dst_not_NULL),
	cmocka_unit_test(write__src_NULL_dst_offset_not_NULL),
	cmocka_unit_test(write__src_NULL_src_offset_not_NULL),
	cmocka_unit_test(write__src_NULL_len_not_NULL),
	cmocka_unit_test(write__src_NULL_dst_offsets_len_not_NULL),
	cmocka_unit_test(write__flags_0),
	cmocka_unit_test(write__conn_dst_NULL_flags_0),
	cmocka_unit_test_setup_teardown(write__success,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_write, group_setup_write, NULL);
}
