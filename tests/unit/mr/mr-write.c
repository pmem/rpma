// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * mr-write.c -- rpma_mr_write() unit tests
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

/*
 * write__failed_E_NOSUPP - rpma_mr_write failed with RPMA_E_NOSUPP
 */
static void
write__failed_E_NOSUPP(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* run test */
	int ret = rpma_mr_write(MOCK_QP, mrs->remote, MOCK_DST_OFFSET,
			mrs->local, MOCK_SRC_OFFSET, MOCK_LEN,
			RPMA_F_COMPLETION_ALWAYS, MOCK_UNKNOWN_OP,
			0, MOCK_OP_CONTEXT, MOCK_NOFENCE);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOSUPP);
}

/*
 * write__COMPL_ALWAYS_failed_E_PROVIDER - rpma_mr_write failed
 * with RPMA_E_PROVIDER when send_flags == RPMA_F_COMPLETION_ON_SUCCESS
 */
static void
write__COMPL_ALWAYS_failed_E_PROVIDER(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	struct ibv_post_send_mock_args args;
	args.qp = MOCK_QP;
	args.opcode = IBV_WR_RDMA_WRITE;
	args.send_flags = IBV_SEND_SIGNALED; /* for RPMA_F_COMPLETION_ALWAYS */
	args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	args.remote_addr = MOCK_RADDR + MOCK_DST_OFFSET;
	args.rkey = MOCK_RKEY;
	args.ret = MOCK_ERRNO;
	will_return(ibv_post_send_mock, &args);

	/* run test */
	int ret = rpma_mr_write(MOCK_QP, mrs->remote, MOCK_DST_OFFSET,
			mrs->local, MOCK_SRC_OFFSET, MOCK_LEN,
			RPMA_F_COMPLETION_ALWAYS, IBV_WR_RDMA_WRITE,
			0, MOCK_OP_CONTEXT, MOCK_NOFENCE);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * write__COMPL_ALWAYS_FENCE_failed_E_PROVIDER - rpma_mr_write failed
 * with RPMA_E_PROVIDER when send_flags == RPMA_F_COMPLETION_ON_SUCCESS
 * and fence == true
 */
static void
write__COMPL_ALWAYS_FENCE_failed_E_PROVIDER(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	struct ibv_post_send_mock_args args;
	args.qp = MOCK_QP;
	args.opcode = IBV_WR_RDMA_WRITE;
	/* for RPMA_F_COMPLETION_ALWAYS and fence is true */
	args.send_flags = IBV_SEND_SIGNALED | IBV_SEND_FENCE;
	args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	args.remote_addr = MOCK_RADDR + MOCK_DST_OFFSET;
	args.rkey = MOCK_RKEY;
	args.ret = MOCK_ERRNO;
	will_return(ibv_post_send_mock, &args);

	/* run test */
	int ret = rpma_mr_write(MOCK_QP, mrs->remote, MOCK_DST_OFFSET,
			mrs->local, MOCK_SRC_OFFSET, MOCK_LEN,
			RPMA_F_COMPLETION_ALWAYS, IBV_WR_RDMA_WRITE,
			0, MOCK_OP_CONTEXT, MOCK_FENCE);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * write__COMPL_ON_ERROR_failed_E_PROVIDER - rpma_mr_write failed
 * with RPMA_E_PROVIDER when send_flags == 0 for RPMA_F_COMPLETION_ON_ERROR
 */
static void
write__COMPL_ON_ERROR_failed_E_PROVIDER(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	struct ibv_post_send_mock_args args;
	args.qp = MOCK_QP;
	args.opcode = IBV_WR_RDMA_WRITE;
	args.send_flags = 0; /* for RPMA_F_COMPLETION_ON_ERROR */
	args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	args.remote_addr = MOCK_RADDR + MOCK_DST_OFFSET;
	args.rkey = MOCK_RKEY;
	args.ret = MOCK_ERRNO;
	will_return(ibv_post_send_mock, &args);

	/* run test */
	int ret = rpma_mr_write(MOCK_QP, mrs->remote, MOCK_DST_OFFSET,
			mrs->local, MOCK_SRC_OFFSET, MOCK_LEN,
			RPMA_F_COMPLETION_ON_ERROR, IBV_WR_RDMA_WRITE,
			0, MOCK_OP_CONTEXT, MOCK_NOFENCE);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * write__COMPL_ON_ERROR_failed_E_PROVIDER - rpma_mr_write failed
 * with RPMA_E_PROVIDER when send_flags == 0 for RPMA_F_COMPLETION_ON_ERROR
 * and fence == true
 */
static void
write__COMPL_ON_ERROR_FENCE_failed_E_PROVIDER(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	struct ibv_post_send_mock_args args;
	args.qp = MOCK_QP;
	args.opcode = IBV_WR_RDMA_WRITE;
	args.send_flags = IBV_SEND_FENCE; /* for fence is true */
	args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	args.remote_addr = MOCK_RADDR + MOCK_DST_OFFSET;
	args.rkey = MOCK_RKEY;
	args.ret = MOCK_ERRNO;
	will_return(ibv_post_send_mock, &args);

	/* run test */
	int ret = rpma_mr_write(MOCK_QP, mrs->remote, MOCK_DST_OFFSET,
			mrs->local, MOCK_SRC_OFFSET, MOCK_LEN,
			RPMA_F_COMPLETION_ON_ERROR, IBV_WR_RDMA_WRITE,
			0, MOCK_OP_CONTEXT, MOCK_FENCE);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * write__success - happy day scenario
 */
static void
write__success(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	enum ibv_wr_opcode opcodes[] = {
		IBV_WR_RDMA_WRITE,
		IBV_WR_RDMA_WRITE_WITH_IMM
	};
	uint32_t imms[] = {
		0,
		MOCK_IMM_DATA
	};

	int n_values = sizeof(opcodes) / sizeof(opcodes[0]);

	for (int i = 0; i < n_values; i++) {
		/* configure mocks */
		struct ibv_post_send_mock_args args;
		args.qp = MOCK_QP;
		args.opcode = opcodes[i];
		/* for RPMA_F_COMPLETION_ALWAYS */
		args.send_flags = IBV_SEND_SIGNALED;
		args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
		args.remote_addr = MOCK_RADDR + MOCK_DST_OFFSET;
		args.rkey = MOCK_RKEY;
		if (opcodes[i] == IBV_WR_RDMA_WRITE_WITH_IMM)
			args.imm_data = htonl(MOCK_IMM_DATA);
		args.ret = MOCK_OK;
		will_return(ibv_post_send_mock, &args);

		/* run test */
		int ret = rpma_mr_write(MOCK_QP, mrs->remote, MOCK_DST_OFFSET,
				mrs->local, MOCK_SRC_OFFSET, MOCK_LEN,
				RPMA_F_COMPLETION_ALWAYS, opcodes[i],
				imms[i], MOCK_OP_CONTEXT, MOCK_NOFENCE);

		/* verify the results */
		assert_int_equal(ret, MOCK_OK);
	}
}

/*
 * write__FENCE_success - happy day scenario
 */
static void
write__FENCE_success(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	struct ibv_post_send_mock_args args;
	args.qp = MOCK_QP;
	args.opcode = IBV_WR_RDMA_WRITE;
	/* for RPMA_F_COMPLETION_ALWAYS and fence is true */
	args.send_flags = IBV_SEND_SIGNALED | IBV_SEND_FENCE;
	args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	args.remote_addr = MOCK_RADDR + MOCK_DST_OFFSET;
	args.rkey = MOCK_RKEY;
	args.ret = MOCK_OK;
	will_return(ibv_post_send_mock, &args);

	/* run test */
	int ret = rpma_mr_write(MOCK_QP, mrs->remote, MOCK_DST_OFFSET,
			mrs->local, MOCK_SRC_OFFSET, MOCK_LEN,
			RPMA_F_COMPLETION_ALWAYS, IBV_WR_RDMA_WRITE,
			0, MOCK_OP_CONTEXT, MOCK_FENCE);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * write_0B_message__success - happy day scenario
 */
static void
write_0B_message__success(void **mrs_ptr)
{
	enum ibv_wr_opcode opcodes[] = {
		IBV_WR_RDMA_WRITE,
		IBV_WR_RDMA_WRITE_WITH_IMM
	};
	uint32_t imms[] = {
		0,
		MOCK_IMM_DATA
	};

	int n_values = sizeof(opcodes) / sizeof(opcodes[0]);

	for (int i = 0; i < n_values; i++) {
		/* configure mocks */
		struct ibv_post_send_mock_args args;
		args.qp = MOCK_QP;
		args.opcode = opcodes[i];
		/* for RPMA_F_COMPLETION_ALWAYS */
		args.send_flags = IBV_SEND_SIGNALED;
		args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
		args.remote_addr = 0;
		args.rkey = 0;
		if (opcodes[i] == IBV_WR_RDMA_WRITE_WITH_IMM)
			args.imm_data = htonl(MOCK_IMM_DATA);
		args.ret = MOCK_OK;
		will_return(ibv_post_send_mock, &args);

		/* run test */
		int ret = rpma_mr_write(MOCK_QP, NULL, 0, NULL, 0, 0,
				RPMA_F_COMPLETION_ALWAYS, opcodes[i],
				imms[i], MOCK_OP_CONTEXT, MOCK_NOFENCE);

		/* verify the results */
		assert_int_equal(ret, MOCK_OK);
	}
}

/*
 * group_setup_mr_write -- prepare resources for all tests in the group
 */
static int
group_setup_mr_write(void **unused)
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

static const struct CMUnitTest tests_mr_write[] = {
	/* rpma_mr_write() unit tests */
	cmocka_unit_test_setup_teardown(write__failed_E_NOSUPP,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(
			write__COMPL_ALWAYS_failed_E_PROVIDER,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(
			write__COMPL_ALWAYS_FENCE_failed_E_PROVIDER,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(
			write__COMPL_ON_ERROR_failed_E_PROVIDER,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(
			write__COMPL_ON_ERROR_FENCE_failed_E_PROVIDER,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(write__success,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(write__FENCE_success,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(write_0B_message__success,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_mr_write,
			group_setup_mr_write, NULL);
}
