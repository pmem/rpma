// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

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
 * setup__flush_new - prepare a valid rpma_flush object
 */
int
setup__flush_new(void **fstate_ptr)
{
	static struct flush_test_state fstate = {0};

	/* configure mocks */
	will_return_always(__wrap__test_malloc, MOCK_OK);
	will_return(__wrap_sysconf, MOCK_OK);
	will_return(__wrap_mmap, MOCK_OK);
	will_return(__wrap_mmap, &fstate.allocated_raw);
	expect_value(rpma_mr_reg, peer, MOCK_PEER);
	expect_value(rpma_mr_reg, size, 8);
	expect_value(rpma_mr_reg, usage, RPMA_MR_USAGE_READ_DST);
	will_return(rpma_mr_reg, &fstate.allocated_raw.addr);
	will_return(rpma_mr_reg, MOCK_RPMA_MR_LOCAL);

	/* run test */
	int ret = rpma_flush_new(MOCK_PEER, &fstate.flush);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_non_null(fstate.flush);

	*fstate_ptr = &fstate;
	return 0;
}

/*
 * teardown__flush_delete - delete the rpma_flush object
 */
int
teardown__flush_delete(void **fstate_ptr)
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
