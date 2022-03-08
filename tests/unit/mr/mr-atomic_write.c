// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * mr-atomic_write.c -- rpma_mr_atomic_write() unit tests
 */

#include <infiniband/verbs.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "cmocka_headers.h"
#include "mr.h"
#include "librpma.h"

#include "mocks-ibverbs.h"
#include "mr-common.h"
#include "test-common.h"

static const char Mock_src[8];

/*
 * atomic_write__COMPL_ON_ERROR_failed_E_PROVIDER -
 * rpma_mr_atomic_write failed with RPMA_E_PROVIDER
 * when send_flags == IBV_SEND_INLINE | IBV_SEND_FENCE
 * for RPMA_F_COMPLETION_ON_ERROR
 */
static void
atomic_write__COMPL_ON_ERROR_failed_E_PROVIDER(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	struct ibv_post_send_mock_args args;
	args.qp = MOCK_QP;
	args.opcode = IBV_WR_RDMA_WRITE;
	/* RPMA_F_COMPLETION_ON_ERROR */
	args.send_flags = IBV_SEND_INLINE | IBV_SEND_FENCE;
	args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	args.remote_addr = MOCK_RADDR + MOCK_DST_OFFSET;
	args.rkey = MOCK_RKEY;
	args.ret = MOCK_ERRNO;
	will_return(ibv_post_send_mock, &args);

	/* run test */
	int ret = rpma_mr_atomic_write(MOCK_QP, mrs->remote, MOCK_DST_OFFSET,
			Mock_src, RPMA_F_COMPLETION_ON_ERROR, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * atomic_write__COMPL_ON_SUCCESS_failed_E_PROVIDER -
 * rpma_mr_atomic_write failed with RPMA_E_PROVIDER
 * when send_flags == IBV_SEND_INLINE | IBV_SEND_FENCE
 *				| IBV_SEND_SIGNALED
 * for RPMA_F_COMPLETION_ALWAYS
 */
static void
atomic_write__COMPL_ON_SUCCESS_failed_E_PROVIDER(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	struct ibv_post_send_mock_args args;
	args.qp = MOCK_QP;
	args.opcode = IBV_WR_RDMA_WRITE;
	/* RPMA_F_COMPLETION_ALWAYS */
	args.send_flags = IBV_SEND_INLINE | IBV_SEND_FENCE
				| IBV_SEND_SIGNALED;
	args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	args.remote_addr = MOCK_RADDR + MOCK_DST_OFFSET;
	args.rkey = MOCK_RKEY;
	args.ret = MOCK_ERRNO;
	will_return(ibv_post_send_mock, &args);

	/* run test */
	int ret = rpma_mr_atomic_write(MOCK_QP, mrs->remote, MOCK_DST_OFFSET,
			Mock_src, RPMA_F_COMPLETION_ALWAYS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * atomic_write__COMPLETION_ALWAYS_success - happy day scenario
 */
static void
atomic_write__COMPLETION_ALWAYS_success(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mock */
	struct ibv_post_send_mock_args args;
	args.qp = MOCK_QP;
	args.opcode = IBV_WR_RDMA_WRITE;
	args.send_flags = IBV_SEND_INLINE | IBV_SEND_FENCE
				| IBV_SEND_SIGNALED;
	args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	args.remote_addr = MOCK_RADDR + MOCK_DST_OFFSET;
	args.rkey = MOCK_RKEY;
	args.ret = MOCK_OK;
	will_return(ibv_post_send_mock, &args);

	/* run test */
	int ret = rpma_mr_atomic_write(MOCK_QP, mrs->remote, MOCK_DST_OFFSET,
			Mock_src, RPMA_F_COMPLETION_ALWAYS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * atomic_write__COMPLETION_ON_ERROR_success
 */
static void
atomic_write__COMPLETION_ON_ERROR_success(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mock */
	struct ibv_post_send_mock_args args;
	args.qp = MOCK_QP;
	args.opcode = IBV_WR_RDMA_WRITE;
	args.send_flags = IBV_SEND_INLINE | IBV_SEND_FENCE;
	args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	args.remote_addr = MOCK_RADDR + MOCK_DST_OFFSET;
	args.rkey = MOCK_RKEY;
	args.ret = MOCK_OK;
	will_return(ibv_post_send_mock, &args);

	/* run test */
	int ret = rpma_mr_atomic_write(MOCK_QP, mrs->remote, MOCK_DST_OFFSET,
			Mock_src, RPMA_F_COMPLETION_ON_ERROR, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * group_setup_mr_atomic_write -- prepare resources for all tests in the group
 */
static int
group_setup_mr_atomic_write(void **unused)
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

static const struct CMUnitTest tests_mr__atomic_write[] = {
	/* rpma_mr_atomic_write() unit tests */
	cmocka_unit_test_setup_teardown(
			atomic_write__COMPL_ON_ERROR_failed_E_PROVIDER,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(
			atomic_write__COMPL_ON_SUCCESS_failed_E_PROVIDER,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(
			atomic_write__COMPLETION_ALWAYS_success,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(
			atomic_write__COMPLETION_ON_ERROR_success,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),

	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_mr__atomic_write,
			group_setup_mr_atomic_write, NULL);
}
