// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * conn-read.c -- the rpma_read() unit tests
 *
 * APIs covered:
 * - rpma_read()
 */

#include "conn-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"

/*
 * read__conn_NULL - NULL conn is invalid
 */
static void
read__conn_NULL(void **unused)
{
	/* run test */
	int ret = rpma_read(NULL, MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
				MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
				MOCK_LEN, MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * read__dst_NULL - NULL dst is invalid
 */
static void
read__dst_NULL(void **unused)
{
	/* run test */
	int ret = rpma_read(MOCK_CONN, NULL, MOCK_LOCAL_OFFSET,
				MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
				MOCK_LEN, MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}


/*
 * read__src_NULL_dst_not_NULL -- NULL src and not NULL dst are invalid
 */
static void
read__src_NULL_dst_not_NULL(void **unused)
{
	/* run test */
	int ret = rpma_read(MOCK_CONN, MOCK_RPMA_MR_LOCAL, 0, NULL, 0, 0,
				MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * read__src_NULL_dst_offset_not_NULL -- NULL src and
 * dst_offset != 0 are invalid
 */
static void
read__src_NULL_dst_offset_not_NULL(void **unused)
{
	/* run test */
	int ret = rpma_read(MOCK_CONN, NULL, MOCK_LOCAL_OFFSET, NULL, 0, 0,
				MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * read__src_NULL_src_offset_not_NULL -- NULL src and
 * src_offset != 0 are invalid
 */
static void
read__src_NULL_src_offset_not_NULL(void **unused)
{
	/* run test */
	int ret = rpma_read(MOCK_CONN, NULL, 0, NULL, MOCK_REMOTE_OFFSET,
				0, MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * read__src_NULL_len_not_NULL -- NULL src and len != 0 are invalid
 */
static void
read__src_NULL_len_not_NULL(void **unused)
{
	/* run test */
	int ret = rpma_read(MOCK_CONN, NULL, 0, NULL, 0, MOCK_LEN,
				MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * read__src_NULL_dst_offsets_len_not_NULL - NULL src is invalid
 */
static void
read__src_NULL_dst_offsets_len_not_NULL(void **unused)
{
	/* run test */
	int ret = rpma_read(MOCK_CONN, MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
				NULL, MOCK_REMOTE_OFFSET,
				MOCK_LEN, MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * read__flags_0 - flags == 0 is invalid
 */
static void
read__flags_0(void **unused)
{
	/* run test */
	int ret = rpma_read(MOCK_CONN, MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
				MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
				MOCK_LEN, 0, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * read__conn_dst_NULL_flags_0 - NULL conn, dst
 * and flags == 0 are invalid
 */
static void
read__conn_dst_NULL_flags_0(void **unused)
{
	/* run test */
	int ret = rpma_read(NULL, NULL, MOCK_LOCAL_OFFSET,
				MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
				MOCK_LEN, 0, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * read__success - happy day scenario
 */
static void
read__success(void **cstate_ptr)
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
static int
group_setup_read(void **unused)
{
	/* set value of QP in mock of CM ID */
	Cm_id.qp = MOCK_QP;

	return 0;
}

static const struct CMUnitTest tests_read[] = {
	/* rpma_read() unit tests */
	cmocka_unit_test(read__conn_NULL),
	cmocka_unit_test(read__dst_NULL),
	cmocka_unit_test(read__src_NULL_dst_not_NULL),
	cmocka_unit_test(read__src_NULL_dst_offset_not_NULL),
	cmocka_unit_test(read__src_NULL_src_offset_not_NULL),
	cmocka_unit_test(read__src_NULL_len_not_NULL),
	cmocka_unit_test(read__src_NULL_dst_offsets_len_not_NULL),
	cmocka_unit_test(read__flags_0),
	cmocka_unit_test(read__conn_dst_NULL_flags_0),
	cmocka_unit_test_setup_teardown(read__success,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_read, group_setup_read, NULL);
}
