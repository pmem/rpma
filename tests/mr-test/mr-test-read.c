/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mr-test.c -- the memory region unit tests
 */

#include <infiniband/verbs.h>
#include <stdlib.h>

#include "cmocka_headers.h"
#include "mr.h"
#include "librpma.h"
#include "rpma_err.h"

#include "mr-test-common.h"

#define IBV_SEND_FLAGS ((unsigned int)(\
	IBV_SEND_FENCE |\
	IBV_SEND_SIGNALED |\
	IBV_SEND_SOLICITED |\
	IBV_SEND_INLINE |\
	IBV_SEND_IP_CSUM))

#ifndef IBV_WR_DRIVER1
/*
 * Define IBV_WR_DRIVER1 for older versions of libibverbs,
 * which do not have it defined yet.
 */
#define IBV_WR_DRIVER1 (IBV_WR_TSO + 1)
#endif

static const LargestIntegralType Ibv_wr_opcodes[] = {
	IBV_WR_RDMA_WRITE,
	IBV_WR_RDMA_WRITE_WITH_IMM,
	IBV_WR_SEND,
	IBV_WR_SEND_WITH_IMM,
	IBV_WR_RDMA_READ,
	IBV_WR_ATOMIC_CMP_AND_SWP,
	IBV_WR_ATOMIC_FETCH_AND_ADD,
	IBV_WR_LOCAL_INV,
	IBV_WR_BIND_MW,
	IBV_WR_SEND_WITH_INV,
	IBV_WR_TSO,
	IBV_WR_DRIVER1,
};

static const LargestIntegralType Num_ibv_wr_opcodes = \
	sizeof(Ibv_wr_opcodes) / sizeof(Ibv_wr_opcodes[0]);

/* mocks */

#define MOCK_DST_OFFSET		(size_t)0xC413
#define MOCK_SRC_OFFSET		(size_t)0xC414
#define MOCK_LEN		(size_t)0xC415
#define MOCK_FLAGS		(int)0xC416
#define MOCK_OP_CONTEXT		(void *)0xC417

static struct ibv_qp Ibv_qp;
static struct ibv_context Ibv_context;

/*
 * ibv_post_send_mock -- mock of ibv_post_send()
 */
int
ibv_post_send_mock(struct ibv_qp *qp, struct ibv_send_wr *wr,
			struct ibv_send_wr **bad_wr)
{
	assert_non_null(qp);
	assert_non_null(wr);
	assert_non_null(bad_wr);
	assert_in_set(wr->opcode, Ibv_wr_opcodes, Num_ibv_wr_opcodes);
	assert_int_equal(wr->send_flags & ~IBV_SEND_FLAGS, 0);

	return mock_type(int);
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

	/* configure mocks */

	/* create a local memory region structure */
	ret = setup__mr_reg_success((void **)&mrs.dst);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(mrs.dst);

	/* create a remote memory region structure */
	ret = setup__mr_remote((void **)&mrs.src);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(mrs.src);

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

	/* configure mocks */

	/* create a local memory region structure */
	ret = teardown__mr_dereg_success((void **)&mrs->dst);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(mrs->dst);

	/* create a remote memory region structure */
	ret = teardown__mr_remote((void **)&mrs->src);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(mrs->src);

	*mrs_ptr = NULL;

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
	will_return(ibv_post_send_mock, MOCK_ERRNO);

	/* run test */
	int ret = rpma_mr_read(&Ibv_qp, mrs->dst, MOCK_DST_OFFSET,
				mrs->src, MOCK_SRC_OFFSET,
				MOCK_LEN, MOCK_FLAGS, MOCK_OP_CONTEXT);

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
	will_return(ibv_post_send_mock, MOCK_OK);

	/* run test */
	int ret = rpma_mr_read(&Ibv_qp, mrs->dst, MOCK_DST_OFFSET,
				mrs->src, MOCK_SRC_OFFSET,
				MOCK_LEN, MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

int
main(int argc, char *argv[])
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

	const struct CMUnitTest tests[] = {
		/* rpma_mr_read() unit tests */
		cmocka_unit_test_setup_teardown(test_read__failed_E_PROVIDER,
				setup__mr_local_and_remote,
				teardown__mr_local_and_remote),
		cmocka_unit_test_setup_teardown(test_read__success,
				setup__mr_local_and_remote,
				teardown__mr_local_and_remote),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
