// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * conn-atomic_write.c -- the rpma_atomic_write() unit tests
 *
 * APIs covered:
 * - rpma_atomic_write()
 */

#include "conn-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"

static const char Mock_src[8];

/*
 * atomic_write__conn_NULL -- NULL conn is invalid
 */
static void
atomic_write__conn_NULL(void **unused)
{
	/* run test */
	int ret = rpma_atomic_write(NULL, MOCK_RPMA_MR_REMOTE,
			MOCK_OFFSET_ALIGNED, Mock_src,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * atomic_write__dst_NULL -- NULL dst is invalid
 */
static void
atomic_write__dst_NULL(void **unused)
{
	/* run test */
	int ret = rpma_atomic_write(MOCK_CONN, NULL,
			MOCK_OFFSET_ALIGNED, Mock_src,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * atomic_write__src_NULL -- NULL src is invalid
 */
static void
atomic_write__src_NULL(void **unused)
{
	/* run test */
	int ret = rpma_atomic_write(MOCK_CONN, MOCK_RPMA_MR_REMOTE,
			MOCK_OFFSET_ALIGNED, NULL,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * atomic_write__flags_0 -- flags == 0 is invalid
 */
static void
atomic_write__flags_0(void **unused)
{
	/* run test */
	int ret = rpma_atomic_write(MOCK_CONN, MOCK_RPMA_MR_REMOTE,
			MOCK_OFFSET_ALIGNED, Mock_src,
			0, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * atomic_write__dst_offset_unaligned -- the unaligned dst_offset is invalid
 */
static void
atomic_write__dst_offset_unaligned(void **unused)
{
	/* run test */
	int ret = rpma_atomic_write(MOCK_CONN, MOCK_RPMA_MR_REMOTE,
			MOCK_REMOTE_OFFSET, Mock_src,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * atomic_write__conn_dst_src_NULL_flags_0_dst_offset_unaligned -- NULL conn,
 * dst, src, flags == 0 and an unaligned dst_offset are invalid
 */
static void
atomic_write__conn_dst_src_NULL_flags_0_dst_offset_unaligned(void **unused)
{
	/* run test */
	int ret = rpma_atomic_write(NULL, NULL,
			MOCK_REMOTE_OFFSET, NULL,
			0, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * atomic_write__success -- happy day scenario
 */
static void
atomic_write__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	expect_value(rpma_mr_atomic_write, qp, MOCK_QP);
	expect_value(rpma_mr_atomic_write, dst, MOCK_RPMA_MR_REMOTE);
	expect_value(rpma_mr_atomic_write, dst_offset, MOCK_OFFSET_ALIGNED);
	expect_value(rpma_mr_atomic_write, src, Mock_src);
	expect_value(rpma_mr_atomic_write, flags, MOCK_FLAGS);
	expect_value(rpma_mr_atomic_write, op_context, MOCK_OP_CONTEXT);
	will_return(rpma_mr_atomic_write, MOCK_OK);

	/* run test */
	int ret = rpma_atomic_write(cstate->conn, MOCK_RPMA_MR_REMOTE,
			MOCK_OFFSET_ALIGNED, Mock_src,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * group_setup_atomic_write -- prepare resources for all tests in the group
 */
int
group_setup_atomic_write(void **unused)
{
	/* set value of QP in mock of CM ID */
	Cm_id.qp = MOCK_QP;

	return 0;
}

static const struct CMUnitTest tests_atomic_write[] = {
	/* rpma_atomic_write() unit tests */
	cmocka_unit_test(atomic_write__conn_NULL),
	cmocka_unit_test(atomic_write__dst_NULL),
	cmocka_unit_test(atomic_write__src_NULL),
	cmocka_unit_test(atomic_write__flags_0),
	cmocka_unit_test(atomic_write__dst_offset_unaligned),
	cmocka_unit_test(
		atomic_write__conn_dst_src_NULL_flags_0_dst_offset_unaligned),
	cmocka_unit_test_setup_teardown(atomic_write__success,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_atomic_write,
			group_setup_atomic_write, NULL);
}
