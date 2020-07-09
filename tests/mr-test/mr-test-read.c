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

/* mocks */

static struct ibv_qp Ibv_qp;
static struct ibv_context Ibv_context;

#define MOCK_DST_OFFSET		(size_t)0xC413
#define MOCK_SRC_OFFSET		(size_t)0xC414
#define MOCK_LEN		(size_t)0xC415
#define MOCK_OP_CONTEXT		(void *)0xC417
#define MOCK_QP			(struct ibv_qp *)&Ibv_qp

struct ibv_post_send_mock_args {
	struct ibv_qp *qp;
	enum ibv_wr_opcode opcode;
	unsigned send_flags;
	uint64_t wr_id;
	int ret;
};

/*
 * ibv_post_send_mock -- mock of ibv_post_send()
 */
int
ibv_post_send_mock(struct ibv_qp *qp, struct ibv_send_wr *wr,
			struct ibv_send_wr **bad_wr)
{
	struct ibv_post_send_mock_args *args =
		mock_type(struct ibv_post_send_mock_args *);

	assert_non_null(qp);
	assert_non_null(wr);
	assert_non_null(bad_wr);

	assert_int_equal(qp, args->qp);
	assert_int_equal(wr->opcode, args->opcode);
	assert_int_equal(wr->send_flags, args->send_flags);
	assert_int_equal(wr->wr_id, args->wr_id);

	return args->ret;
}

/* setups & teardowns */

struct mrs {
	struct rpma_mr_local *dst;
	struct rpma_mr_remote *src;
};

/*
 * setup__mr_local_and_remote -- create a local and a remote
 * memory region structures
 */
int
setup__mr_local_and_remote(void **mrs_ptr)
{
	static struct mrs mrs = {0};
	int ret;

	struct prestate prestate = {MOCK_USAGE, MOCK_ACCESS, NULL};
	struct prestate *pprestate = &prestate;

	/* create a local memory region structure */
	ret = setup__reg_success((void **)&pprestate);
	mrs.dst = prestate.mr;

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);

	/* create a remote memory region structure */
	ret = setup__mr_remote((void **)&mrs.src);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);

	*mrs_ptr = &mrs;

	return 0;
}

/*
 * teardown__mr_local_and_remote -- delete a local and a remote
 * memory region structures
 */
int
teardown__mr_local_and_remote(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;
	int ret;

	struct prestate prestate = {0};
	struct prestate *pprestate = &prestate;
	prestate.mr = mrs->dst;

	/* create a local memory region structure */
	ret = teardown__dereg_success((void **)&pprestate);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);

	/* create a remote memory region structure */
	ret = teardown__mr_remote((void **)&mrs->src);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);

	return 0;
}

/*
 * test_read__failed_E_PROVIDER - rpma_mr_read failed with RPMA_E_PROVIDER
 */
static void
test_read__failed_E_PROVIDER(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	struct ibv_post_send_mock_args args;
	args.qp = MOCK_QP;
	args.opcode = IBV_WR_RDMA_READ;
	args.send_flags = 0; /* for RPMA_F_COMPLETION_ON_ERROR */
	args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	args.ret = MOCK_ERRNO;
	will_return(ibv_post_send_mock, &args);

	/* run test */
	int ret = rpma_mr_read(MOCK_QP, mrs->dst, MOCK_DST_OFFSET,
				mrs->src, MOCK_SRC_OFFSET,
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
	struct ibv_post_send_mock_args args;
	args.qp = MOCK_QP;
	args.opcode = IBV_WR_RDMA_READ;
	args.send_flags = IBV_SEND_SIGNALED; /* for RPMA_F_COMPLETION_ALWAYS */
	args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	args.ret = MOCK_OK;
	will_return(ibv_post_send_mock, &args);

	/* run test */
	int ret = rpma_mr_read(MOCK_QP, mrs->dst, MOCK_DST_OFFSET,
				mrs->src, MOCK_SRC_OFFSET,
				MOCK_LEN, RPMA_F_COMPLETION_ALWAYS,
				MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * group_setup_read -- prepare resources for all tests in the group
 */
int
group_setup_read(void **unused)
{
	/* configure global mocks */

	/*
	 * ibv_post_send() is defined as a static inline function
	 * in the included header <infiniband/verbs.h>,
	 * so we cannot define it again. It is defined as:
	 * {
	 *     return qp->context->ops.post_recv(qp, wr, bad_wr);
	 * }
	 * so we can set the 'qp->context->ops.post_recv' function pointer
	 * to our mock function.
	 */
	Ibv_context.ops.post_send = ibv_post_send_mock;
	Ibv_qp.context = &Ibv_context;

	return 0;
}

const struct CMUnitTest tests_read[] = {
	/* rpma_mr_read() unit tests */
	cmocka_unit_test_setup_teardown(test_read__failed_E_PROVIDER,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(test_read__success,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_EOF
};
