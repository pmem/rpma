// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */
/* Copyright (c) 2023 Fujitsu Limited */

/*
 * flush-common.c -- common part of unit tests of the flush module
 */

#include "cmocka_headers.h"
#include "flush.h"
#include "flush-common.h"
#include "mocks-stdlib.h"
#include "mocks-unistd.h"
#include "test-common.h"

/*
 * setup__apm_flush_new - prepare a valid rpma_flush object
 */
int
setup__apm_flush_new(void **fstate_ptr)
{
	static struct flush_test_state fstate = {0};

	/* configure mocks */
	will_return_always(__wrap__test_malloc, MOCK_OK);
#ifdef NATIVE_FLUSH_SUPPORTED
	expect_value(ibv_qp_to_qp_ex, qp, MOCK_QP);
	will_return(ibv_qp_to_qp_ex, NULL);
#endif
	will_return(__wrap_sysconf, MOCK_OK);
	will_return(__wrap_mmap, MOCK_OK);
	will_return(__wrap_mmap, &fstate.allocated_raw);
	expect_value(rpma_mr_reg, peer, MOCK_PEER);
	expect_value(rpma_mr_reg, size, 8);
	expect_value(rpma_mr_reg, usage, RPMA_MR_USAGE_READ_DST);
	will_return(rpma_mr_reg, &fstate.allocated_raw.addr);
	will_return(rpma_mr_reg, MOCK_RPMA_MR_LOCAL);

	/* run test */
	int ret = rpma_flush_new(MOCK_PEER, MOCK_QP, &fstate.flush);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_non_null(fstate.flush);

	*fstate_ptr = &fstate;
	return 0;
}

/*
 * teardown__apm_flush_delete - delete the rpma_flush object
 */
int
teardown__apm_flush_delete(void **fstate_ptr)
{
	struct flush_test_state *fstate = *fstate_ptr;

	/* configure mock */
	expect_value(rpma_mr_dereg, *mr_ptr, MOCK_RPMA_MR_LOCAL);
	will_return(rpma_mr_dereg, MOCK_OK);
	will_return(__wrap_munmap, &fstate->allocated_raw);
	will_return(__wrap_munmap, MOCK_OK);

	/* delete the object */
	int ret = rpma_flush_delete(&fstate->flush);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(fstate->flush);
	return 0;
}

#ifdef NATIVE_FLUSH_SUPPORTED
/*
 * setup__native_flush_new - prepare a valid rpma_flush object
 */
int
setup__native_flush_new(void **fstate_ptr)
{
	static struct flush_test_state fstate = {0};

	/* configure mocks */
	will_return_always(__wrap__test_malloc, MOCK_OK);
	expect_value(ibv_qp_to_qp_ex, qp, MOCK_QP);
	will_return(ibv_qp_to_qp_ex, MOCK_QPX);

	/* run test */
	int ret = rpma_flush_new(MOCK_PEER, MOCK_QP, &fstate.flush);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_non_null(fstate.flush);

	*fstate_ptr = &fstate;
	return 0;
}

/*
 * teardown__native_flush_delete - delete the rpma_flush object
 */
int
teardown__native_flush_delete(void **fstate_ptr)
{
	struct flush_test_state *fstate = *fstate_ptr;

	/* delete the object */
	int ret = rpma_flush_delete(&fstate->flush);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(fstate->flush);
	return 0;
}
#endif

/*
 * group_setup_flush_common -- prepare resources for all tests in the group
 */
int
group_setup_flush_common(void **unused)
{
#ifdef NATIVE_FLUSH_SUPPORTED
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
	Ibv_qp_ex.wr_flush = ibv_wr_flush;
	Ibv_qp_ex.wr_complete = ibv_wr_complete_mock;
#endif

	return 0;
}
