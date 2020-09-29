// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mr-test-recv.c -- rpma_mr_recv() unit tests
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
 * recv__failed_E_PROVIDER - rpma_mr_recv failed with RPMA_E_PROVIDER
 */
static void
recv__failed_E_PROVIDER(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	struct ibv_post_recv_mock_args args;
	args.qp = MOCK_QP;
	args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	args.ret = MOCK_ERRNO;
	will_return(ibv_post_recv_mock, &args);

	/* run test */
	int ret = rpma_mr_recv(MOCK_QP, mrs->local, MOCK_SRC_OFFSET,
				MOCK_LEN, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * recv__success - happy day scenario
 */
static void
recv__success(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	struct ibv_post_recv_mock_args args;
	args.qp = MOCK_QP;
	args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	args.ret = MOCK_OK;
	will_return(ibv_post_recv_mock, &args);

	/* run test */
	int ret = rpma_mr_recv(MOCK_QP, mrs->local, MOCK_SRC_OFFSET,
				MOCK_LEN, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * group_setup_mr_recv -- prepare resources for all tests in the group
 */
static int
group_setup_mr_recv(void **unused)
{
	/* configure global mocks */

	/*
	 * ibv_post_recv() is defined as a static inline function
	 * in the included header <infiniband/verbs.h>,
	 * so we cannot define it again. It is defined as:
	 * {
	 *     return qp->context->ops.post_recv(qp, wr, bad_wr);
	 * }
	 * so we can set the 'qp->context->ops.post_recv' function pointer
	 * to our mock function.
	 */
	MOCK_VERBS->ops.post_recv = ibv_post_recv_mock;
	Ibv_qp.context = MOCK_VERBS;

	return 0;
}

static const struct CMUnitTest tests_mr_recv[] = {
	/* rpma_mr_recv() unit tests */
	cmocka_unit_test_setup_teardown(recv__failed_E_PROVIDER,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(recv__success,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_mr_recv,
			group_setup_mr_recv, NULL);
}
