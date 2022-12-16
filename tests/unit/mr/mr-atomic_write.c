// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */
/* Copyright (c) 2022, Fujitsu Limited */

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

#ifdef IBV_WR_ATOMIC_WRITE_SUPPORTED
static struct ibv_wr_atomic_write_mock_args atomic_write_args;
#endif
static struct ibv_post_send_mock_args args;

/*
 * configure_atomic_write -- configure common mock for rpma_mr_atomic_write()
 */
static void
configure_mr_atomic_write(int flags, int ret)
{
	/* configure mock */
#ifdef IBV_WR_ATOMIC_WRITE_SUPPORTED
	expect_value(ibv_qp_to_qp_ex, qp, MOCK_QP);
	will_return(ibv_qp_to_qp_ex, MOCK_QPX);
	expect_value(ibv_wr_start_mock, qp, MOCK_QPX);
	atomic_write_args.qp = MOCK_QPX;
	atomic_write_args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	atomic_write_args.wr_flags = (flags == RPMA_F_COMPLETION_ALWAYS) ? IBV_SEND_SIGNALED : 0;
	atomic_write_args.rkey = MOCK_RKEY;
	atomic_write_args.remote_addr = MOCK_RADDR + MOCK_DST_OFFSET;
	atomic_write_args.atomic_wr = Mock_src;
	will_return(ibv_wr_atomic_write_mock, &atomic_write_args);
	expect_value(ibv_wr_complete_mock, qp, MOCK_QPX);
	will_return(ibv_wr_complete_mock, ret);
#else
	args.qp = MOCK_QP;
	args.opcode = IBV_WR_RDMA_WRITE;
	args.send_flags = IBV_SEND_INLINE | IBV_SEND_FENCE;
	if (flags == RPMA_F_COMPLETION_ALWAYS)
		args.send_flags |= IBV_SEND_SIGNALED;
	args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	args.remote_addr = MOCK_RADDR + MOCK_DST_OFFSET;
	args.rkey = MOCK_RKEY;
	args.ret = ret;
	will_return(ibv_post_send_mock, &args);
#endif
}

#ifdef IBV_WR_ATOMIC_WRITE_SUPPORTED
/*
 * atomic_write__qpx_NULL_success - rpma_mr_atomic_write
 * succeeded when ibv_qp_to_qp_ex() returned NULL
 */
static void
atomic_write__qpx_NULL_success(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	expect_value(ibv_qp_to_qp_ex, qp, MOCK_QP);
	will_return(ibv_qp_to_qp_ex, NULL);
	args.qp = MOCK_QP;
	args.opcode = IBV_WR_RDMA_WRITE;
	args.send_flags = IBV_SEND_INLINE | IBV_SEND_FENCE | IBV_SEND_SIGNALED;
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
#endif

/*
 * atomic_write__COMPL_ON_ERROR_failed_E_PROVIDER -
 * rpma_mr_atomic_write failed with RPMA_E_PROVIDER
 * when RPMA_F_COMPLETION_ON_ERROR is specified
 */
static void
atomic_write__COMPL_ON_ERROR_failed_E_PROVIDER(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	configure_mr_atomic_write(RPMA_F_COMPLETION_ON_ERROR, MOCK_ERRNO);

	/* run test */
	int ret = rpma_mr_atomic_write(MOCK_QP, mrs->remote, MOCK_DST_OFFSET,
			Mock_src, RPMA_F_COMPLETION_ON_ERROR, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * atomic_write__COMPL_ON_SUCCESS_failed_E_PROVIDER -
 * rpma_mr_atomic_write failed with RPMA_E_PROVIDER
 * when RPMA_F_COMPLETION_ALWAYS is specified
 */
static void
atomic_write__COMPL_ON_SUCCESS_failed_E_PROVIDER(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	configure_mr_atomic_write(RPMA_F_COMPLETION_ALWAYS, MOCK_ERRNO);

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
	configure_mr_atomic_write(RPMA_F_COMPLETION_ALWAYS, MOCK_OK);

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
	configure_mr_atomic_write(RPMA_F_COMPLETION_ON_ERROR, MOCK_OK);

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
#ifdef IBV_WR_ATOMIC_WRITE_SUPPORTED
	/*
	 * ibv_wr_start(), ibv_wr_atomic_write() and ibv_wr_complete() are defined
	 * as static inline functions in the included header <infiniband/verbs.h>,
	 * so we cannot define them again. They are defined as:
	 * {
	 *     return qp->wr_start(qp);
	 * }
	 * {
	 *     return qp->wr_atomic_write(qp, rkey, remote_addr, atomic_wr);
	 * }
	 * {
	 *     return qp->wr_complete(qp);
	 * }
	 * so we can set these three function pointers to our mock functions.
	 */
	Ibv_qp_ex.wr_start = ibv_wr_start_mock;
	Ibv_qp_ex.wr_atomic_write = ibv_wr_atomic_write_mock;
	Ibv_qp_ex.wr_complete = ibv_wr_complete_mock;
#endif
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
#ifdef IBV_WR_ATOMIC_WRITE_SUPPORTED
	cmocka_unit_test_setup_teardown(
			atomic_write__qpx_NULL_success,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
#endif
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
