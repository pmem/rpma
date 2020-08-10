/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mr-test-read.c -- rpma_mr_read() unit tests
 */

#include <infiniband/verbs.h>
#include <stdlib.h>

#include "cmocka_headers.h"
#include "mr.h"
#include "librpma.h"
#include "rpma_err.h"

#include "mr-test-common.h"
#include "mocks-ibverbs.h"

/*
 * test_read__failed_E_PROVIDER - rpma_mr_read failed with RPMA_E_PROVIDER
 */
static void
test_read__failed_E_PROVIDER(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	expect_value(ibv_post_send_mock, wr->opcode, IBV_WR_RDMA_READ);
	/* for RPMA_F_COMPLETION_ON_ERROR */
	expect_value(ibv_post_send_mock, wr->send_flags, 0);
	will_return(ibv_post_send_mock, MOCK_ERRNO);

	/* run test */
	int ret = rpma_mr_read(MOCK_QP, mrs->local, MOCK_DST_OFFSET,
				mrs->remote, MOCK_SRC_OFFSET,
				MOCK_LEN, RPMA_F_COMPLETION_ON_ERROR,
				MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), MOCK_ERRNO);
}

/*
 * test_read__success - happy day scenario
 */
static void
test_read__success(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	expect_value(ibv_post_send_mock, wr->opcode, IBV_WR_RDMA_READ);
	/* for RPMA_F_COMPLETION_ALWAYS */
	expect_value(ibv_post_send_mock, wr->send_flags, IBV_SEND_SIGNALED);
	will_return(ibv_post_send_mock, MOCK_OK);

	/* run test */
	int ret = rpma_mr_read(MOCK_QP, mrs->local, MOCK_DST_OFFSET,
				mrs->remote, MOCK_SRC_OFFSET,
				MOCK_LEN, RPMA_F_COMPLETION_ALWAYS,
				MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * group_setup_mr_read -- prepare resources for all tests in the group
 */
int
group_setup_mr_read(void **unused)
{
	/* configure global mocks */

	/*
	 * ibv_post_send() is defined as a static inline function
	 * in the included header <infiniband/verbs.h>,
	 * so we cannot define it again. It is defined as:
	 * {
	 *     return qp->context->ops.post_send(qp, wr, bad_wr);
	 * }
	 * so we can set the 'qp->context->ops.post_send' function pointer
	 * to our mock function.
	 */
	MOCK_VERBS->ops.post_send = ibv_post_send_mock;
	Ibv_qp.context = MOCK_VERBS;

	return 0;
}

const struct CMUnitTest tests_mr_read[] = {
	/* rpma_mr_read() unit tests */
	cmocka_unit_test_setup_teardown(test_read__failed_E_PROVIDER,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(test_read__success,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test(NULL)
};
