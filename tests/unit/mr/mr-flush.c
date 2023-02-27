// SPDX-License-Identifier: BSD-3-Clause
/* Copyright (c) 2023 Fujitsu Limited */

/*
 * mr-flush.c -- rpma_mr_flush() unit tests
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

static struct ibv_wr_flush_mock_args flush_args;

/*
 * configure_flush -- configure common mock for rpma_mr_flush()
 */
static void
configure_mr_flush(enum rpma_flush_type type, int flags, int ret)
{
	/* configure mock */
	expect_value(ibv_qp_to_qp_ex, qp, MOCK_QP);
	will_return(ibv_qp_to_qp_ex, MOCK_QPX);
	expect_value(ibv_wr_start_mock, qp, MOCK_QPX);
	flush_args.qp = MOCK_QPX;
	flush_args.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	flush_args.wr_flags = (flags == RPMA_F_COMPLETION_ALWAYS) ? IBV_SEND_SIGNALED : 0;
	flush_args.rkey = MOCK_RKEY;
	flush_args.remote_addr = MOCK_RADDR + MOCK_DST_OFFSET;
	flush_args.len = MOCK_LEN;
	switch (type) {
	case RPMA_FLUSH_TYPE_VISIBILITY:
		flush_args.type = IBV_FLUSH_GLOBAL;
		break;
	case RPMA_FLUSH_TYPE_PERSISTENT:
		flush_args.type = IBV_FLUSH_PERSISTENT;
	}
	flush_args.level = IBV_FLUSH_RANGE;
	will_return(ibv_wr_flush_mock, &flush_args);
	expect_value(ibv_wr_complete_mock, qp, MOCK_QPX);
	will_return(ibv_wr_complete_mock, ret);
}

/*
 * flush__COMPL_ON_ERROR_failed_E_PROVIDER - rpma_mr_flush failed with RPMA_E_PROVIDER
 * when RPMA_FLUSH_TYPE_VISIBILITY and RPMA_F_COMPLETION_ON_ERROR are specified
 */
static void
flush__COMPL_ON_ERROR_failed_E_PROVIDER(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	configure_mr_flush(RPMA_FLUSH_TYPE_VISIBILITY, RPMA_F_COMPLETION_ON_ERROR, MOCK_ERRNO);

	/* run test */
	int ret = rpma_mr_flush(MOCK_QP, mrs->remote, MOCK_DST_OFFSET, MOCK_LEN,
			RPMA_FLUSH_TYPE_VISIBILITY, RPMA_F_COMPLETION_ON_ERROR, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * flush__COMPL_ON_SUCCESS_failed_E_PROVIDER - rpma_mr_flush failed with RPMA_E_PROVIDER
 * when RPMA_FLUSH_TYPE_PERSISTENT and RPMA_F_COMPLETION_ALWAYS are specified
 */
static void
flush__COMPL_ON_SUCCESS_failed_E_PROVIDER(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	configure_mr_flush(RPMA_FLUSH_TYPE_PERSISTENT, RPMA_F_COMPLETION_ALWAYS, MOCK_ERRNO);

	/* run test */
	int ret = rpma_mr_flush(MOCK_QP, mrs->remote, MOCK_DST_OFFSET, MOCK_LEN,
			RPMA_FLUSH_TYPE_PERSISTENT, RPMA_F_COMPLETION_ALWAYS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * flush__COMPLETION_ALWAYS_success - rpma_mr_flush succeeded when
 * RPMA_FLUSH_TYPE_VISIBILITY and RPMA_F_COMPLETION_ALWAYS are specified
 */
static void
flush__COMPLETION_ALWAYS_success(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mock */
	configure_mr_flush(RPMA_FLUSH_TYPE_VISIBILITY, RPMA_F_COMPLETION_ALWAYS, MOCK_OK);

	/* run test */
	int ret = rpma_mr_flush(MOCK_QP, mrs->remote, MOCK_DST_OFFSET, MOCK_LEN,
			RPMA_FLUSH_TYPE_VISIBILITY, RPMA_F_COMPLETION_ALWAYS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * flush__COMPLETION_ON_ERROR_success - rpma_mr_flush succeeded when
 * when RPMA_FLUSH_TYPE_PERSISTENT and RPMA_F_COMPLETION_ON_ERROR are specified
 */
static void
flush__COMPLETION_ON_ERROR_success(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mock */
	configure_mr_flush(RPMA_FLUSH_TYPE_PERSISTENT, RPMA_F_COMPLETION_ON_ERROR, MOCK_OK);

	/* run test */
	int ret = rpma_mr_flush(MOCK_QP, mrs->remote, MOCK_DST_OFFSET, MOCK_LEN,
			RPMA_FLUSH_TYPE_PERSISTENT, RPMA_F_COMPLETION_ON_ERROR, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * group_setup_mr_flush -- prepare resources for all tests in the group
 */
static int
group_setup_mr_flush(void **unused)
{
	/* configure global mocks */
	/*
	 * ibv_wr_start(), ibv_wr_flush() and ibv_wr_complete() are defined
	 * as static inline functions in the included header <infiniband/verbs.h>,
	 * so we cannot define them again. They are defined as:
	 * {
	 *	return qp->wr_start(qp);
	 * }
	 * {
	 *	return qp->wr_flush(qp, rkey, remote_addr, len, type, level);
	 * }
	 * {
	 *	return qp->wr_complete(qp);
	 * }
	 * so we can set these three function pointers to our mock functions.
	 */
	Ibv_qp_ex.wr_start = ibv_wr_start_mock;
	Ibv_qp_ex.wr_flush = ibv_wr_flush_mock;
	Ibv_qp_ex.wr_complete = ibv_wr_complete_mock;

	return 0;
}

static const struct CMUnitTest tests_mr__flush[] = {
	/* rpma_mr_flush() unit tests */
	cmocka_unit_test_setup_teardown(
			flush__COMPL_ON_ERROR_failed_E_PROVIDER,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(
			flush__COMPL_ON_SUCCESS_failed_E_PROVIDER,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(
			flush__COMPLETION_ALWAYS_success,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(
			flush__COMPLETION_ON_ERROR_success,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),

	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_mr__flush,
			group_setup_mr_flush, NULL);
}
