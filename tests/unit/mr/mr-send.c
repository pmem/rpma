// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mr-test-send.c -- rpma_mr_send() unit tests
 */

#include <infiniband/verbs.h>
#include <stdlib.h>

#include "cmocka_headers.h"
#include "mr.h"
#include "librpma.h"

#include "mocks-ibverbs.h"
#include "mr-common.h"
#include "test-common.h"

/*
 * send__failed_E_PROVIDER - rpma_mr_send failed with RPMA_E_PROVIDER
 */
static void
send__failed_E_PROVIDER(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	struct ibv_post_send_mock_args args;
	args.qp = MOCK_QP;
	args.opcode = IBV_WR_SEND;
	args.send_flags = 0; /* for RPMA_F_COMPLETION_ON_ERROR */
	args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	args.ret = MOCK_ERRNO;
	will_return(ibv_post_send_mock, &args);

	/* run test */
	int ret = rpma_mr_send(MOCK_QP, mrs->local, MOCK_SRC_OFFSET,
				MOCK_LEN, RPMA_F_COMPLETION_ON_ERROR,
				MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * send__success - happy day scenario
 */
static void
send__success(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	struct ibv_post_send_mock_args args;
	args.qp = MOCK_QP;
	args.opcode = IBV_WR_SEND;
	args.send_flags = IBV_SEND_SIGNALED; /* for RPMA_F_COMPLETION_ALWAYS */
	args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	args.ret = MOCK_OK;
	will_return(ibv_post_send_mock, &args);

	/* run test */
	int ret = rpma_mr_send(MOCK_QP, mrs->local, MOCK_SRC_OFFSET,
				MOCK_LEN, RPMA_F_COMPLETION_ALWAYS,
				MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * group_setup_mr_send -- prepare resources for all tests in the group
 */
static int
group_setup_mr_send(void **unused)
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

static const struct CMUnitTest tests_mr_send[] = {
	/* rpma_mr_send() unit tests */
	cmocka_unit_test_setup_teardown(send__failed_E_PROVIDER,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(send__success,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_mr_send,
			group_setup_mr_send, NULL);
}
