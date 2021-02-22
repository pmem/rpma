// SPDX-License-Identifier: BSD-3-Clause
/* Copyright (c) 2021 Fujitsu */

/*
 * conn-write_atomic_cmp_swp.c -- the rpma_write_atomic_cmp_swp() unit tests
 *
 * APIs covered:
 * - rpma_write_atomic_cmp_swp()
 */

#include "conn-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"

/*
 * write_atomic_cmp_swp__conn_NULL -- NULL conn is invalid
 */
static void
write_atomic_cmp_swp__conn_NULL(void **unused)
{
	/* run test */
	int ret = rpma_write_atomic_cmp_swp(NULL,
			MOCK_RPMA_MR_REMOTE, MOCK_OFFSET_ALIGNED,
			MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
			MOCK_FLAGS, MOCK_COMPARE, MOCK_SWAP, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write_atomic_cmp_swp__swap_dst_NULL -- NULL swap_dst is invalid
 */
static void
write_atomic_cmp_swp__swap_dst_NULL(void **unused)
{
	/* run test */
	int ret = rpma_write_atomic_cmp_swp(MOCK_CONN,
			NULL, MOCK_OFFSET_ALIGNED,
			MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
			MOCK_FLAGS, MOCK_COMPARE, MOCK_SWAP, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write_atomic_cmp_swp__read_dst_NULL -- NULL read_dst is invalid
 */
static void
write_atomic_cmp_swp__read_dst_NULL(void **unused)
{
	/* run test */
	int ret = rpma_write_atomic_cmp_swp(MOCK_CONN,
			MOCK_RPMA_MR_REMOTE, MOCK_OFFSET_ALIGNED,
			NULL, MOCK_LOCAL_OFFSET,
			MOCK_FLAGS, MOCK_COMPARE, MOCK_SWAP, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write_atomic_cmp_swp__flags_0 -- flags == 0 is invalid
 */
static void
write_atomic_cmp_swp__flags_0(void **unused)
{
	/* run test */
	int ret = rpma_write_atomic_cmp_swp(MOCK_CONN,
			MOCK_RPMA_MR_REMOTE, MOCK_OFFSET_ALIGNED,
			MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
			0, MOCK_COMPARE, MOCK_SWAP, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write_atomic_cmp_swp__swap_dst_offset_unaligned -- the unaligned
 * swap_dst_offset is invalid
 */
static void
write_atomic_cmp_swp__swap_dst_offset_unaligned(void **unused)
{
	/* run test */
	int ret = rpma_write_atomic_cmp_swp(MOCK_CONN,
			MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
			MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
			MOCK_FLAGS, MOCK_COMPARE, MOCK_SWAP, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write_atomic_cmp_swp__all_invalid_arguments -- NULL conn, swap_dst,
 * read_dst, flags == 0 and an unaligned swap_dst_offset are invalid
 */
static void
write_atomic_cmp_swp__all_invalid_arguments(void **unused)
{
	/* run test */
	int ret = rpma_write_atomic_cmp_swp(NULL,
			NULL, MOCK_REMOTE_OFFSET,
			NULL, MOCK_LOCAL_OFFSET,
			0, MOCK_COMPARE, MOCK_SWAP, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * write_atomic_cmp_swp__success -- happy day scenario
 */
static void
write_atomic_cmp_swp__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	expect_value(rpma_mr_atomic_cmp_swp, qp, MOCK_QP);
	expect_value(rpma_mr_atomic_cmp_swp, swap_dst, MOCK_RPMA_MR_REMOTE);
	expect_value(rpma_mr_atomic_cmp_swp, swap_dst_offset,
		MOCK_OFFSET_ALIGNED);
	expect_value(rpma_mr_atomic_cmp_swp, read_dst, MOCK_RPMA_MR_LOCAL);
	expect_value(rpma_mr_atomic_cmp_swp, read_dst_offset,
		MOCK_LOCAL_OFFSET);
	expect_value(rpma_mr_atomic_cmp_swp, flags, MOCK_FLAGS);
	expect_value(rpma_mr_atomic_cmp_swp, compare, MOCK_COMPARE);
	expect_value(rpma_mr_atomic_cmp_swp, swap, MOCK_SWAP);
	expect_value(rpma_mr_atomic_cmp_swp, op_context, MOCK_OP_CONTEXT);
	will_return(rpma_mr_atomic_cmp_swp, MOCK_OK);

	/* run test */
	int ret = rpma_write_atomic_cmp_swp(cstate->conn,
			MOCK_RPMA_MR_REMOTE, MOCK_OFFSET_ALIGNED,
			MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
			MOCK_FLAGS, MOCK_COMPARE, MOCK_SWAP, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * group_setup_write_atomic_cmp_swp -- prepare resources for all tests
 * in the group
 */
int
group_setup_write_atomic_cmp_swp(void **unused)
{
	/* set value of QP in mock of CM ID */
	Cm_id.qp = MOCK_QP;

	return 0;
}

static const struct CMUnitTest tests_write_atomic_cmp_swp[] = {
	/* rpma_write_atomic_cmp_swp() unit tests */
	cmocka_unit_test(write_atomic_cmp_swp__conn_NULL),
	cmocka_unit_test(write_atomic_cmp_swp__swap_dst_NULL),
	cmocka_unit_test(write_atomic_cmp_swp__read_dst_NULL),
	cmocka_unit_test(write_atomic_cmp_swp__flags_0),
	cmocka_unit_test(write_atomic_cmp_swp__swap_dst_offset_unaligned),
	cmocka_unit_test(write_atomic_cmp_swp__all_invalid_arguments),
	cmocka_unit_test_setup_teardown(write_atomic_cmp_swp__success,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_write_atomic_cmp_swp,
			group_setup_write_atomic_cmp_swp, NULL);
}
