// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Fujitsu */

/*
 * srq-common.c -- the rpma_srq unit tests common functions
 */

#include "srq-common.h"

struct srq_test_state Srq_new_srq_cfg_default = {
	.get_args.cfg = MOCK_SRQ_CFG_DEFAULT,
	.get_args.rq_size = MOCK_SRQ_SIZE_DEFAULT,
	.get_args.rcq_size = MOCK_SRQ_RCQ_SIZE_DEFAULT,
};

struct srq_test_state Srq_new_srq_cfg_custom = {
	.get_args.cfg = MOCK_SRQ_CFG_CUSTOM,
	.get_args.rq_size = MOCK_SRQ_SIZE_CUSTOM,
	.get_args.rcq_size = MOCK_SRQ_RCQ_SIZE_CUSTOM,
};

/*
 * setup__srq_new -- prepare a valid srq object
 */
int
setup__srq_new(void **cstate_ptr)
{
	struct srq_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	will_return(rpma_peer_create_srq, &cstate->get_args);
	will_return(rpma_peer_create_srq, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_OK);

	/* run test */
	int ret = rpma_srq_new(MOCK_PEER, MOCK_GET_SRQ_CFG(cstate), &cstate->srq);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);

	*cstate_ptr = cstate;

	return 0;
}

/*
 * teardown__srq_delete -- destroy the srq object
 */
int
teardown__srq_delete(void **cstate_ptr)
{
	struct srq_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_SRQ_RCQ(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(ibv_destroy_srq, srq, MOCK_IBV_SRQ);
	will_return(ibv_destroy_srq, MOCK_OK);

	/* run test */
	int ret = rpma_srq_delete(&cstate->srq);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cstate->srq);

	return 0;
}
