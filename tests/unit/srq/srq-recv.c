// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Fujitsu */

/*
 * srq-recv.c -- the rpma_srq_recv() unit tests
 *
 * APIs covered:
 * - rpma_srq_recv()
 */

#include "srq-common.h"

/*
 * recv__srq_NULL - NULL srq is invalid
 */
static void
recv__srq_NULL(void **unused)
{
	/* run test */
	int ret = rpma_srq_recv(NULL, MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
			MOCK_LEN, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * recv__dst_NULL_offset_not_NULL - NULL dst and not NULL offset is invalid
 */
static void
recv__dst_NULL_offset_not_NULL(void **unused)
{
	/* run test */
	int ret = rpma_srq_recv(MOCK_RPMA_SRQ, NULL, MOCK_LOCAL_OFFSET, 0, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * recv__dst_NULL_len_not_NULL - NULL dst and not NULL len is invalid
 */
static void
recv__dst_NULL_len_not_NULL(void **unused)
{
	/* run test */
	int ret = rpma_srq_recv(MOCK_RPMA_SRQ, NULL, 0, MOCK_LEN, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * recv__dst_NULL_offset_len_not_NULL - NULL dst and not NULL offset or len are invalid
 */
static void
recv__dst_NULL_offset_len_not_NULL(void **unused)
{
	/* run test */
	int ret = rpma_srq_recv(MOCK_RPMA_SRQ, NULL, MOCK_LOCAL_OFFSET,
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
	struct srq_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	expect_value(rpma_mr_srq_recv, srq, MOCK_IBV_SRQ);
	expect_value(rpma_mr_srq_recv, dst, MOCK_RPMA_MR_LOCAL);
	expect_value(rpma_mr_srq_recv, offset, MOCK_LOCAL_OFFSET);
	expect_value(rpma_mr_srq_recv, len, MOCK_LEN);
	expect_value(rpma_mr_srq_recv, op_context, MOCK_OP_CONTEXT);
	will_return(rpma_mr_srq_recv, MOCK_OK);

	/* run test */
	int ret = rpma_srq_recv(cstate->srq, MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
			MOCK_LEN, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

static const struct CMUnitTest tests_recv[] = {
	/* rpma_srq_recv() unit tests */
	cmocka_unit_test(recv__srq_NULL),
	cmocka_unit_test(recv__dst_NULL_offset_not_NULL),
	cmocka_unit_test(recv__dst_NULL_len_not_NULL),
	cmocka_unit_test(recv__dst_NULL_offset_len_not_NULL),
	cmocka_unit_test_prestate_setup_teardown(recv__success, setup__srq_new,
		teardown__srq_delete, &Srq_new_srq_cfg_default),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_recv, NULL, NULL);
}
