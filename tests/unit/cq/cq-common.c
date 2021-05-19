// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Fujitsu */

/*
 * cq-common.c -- the rpma_cq unit tests common functions
 */

#include <rdma/rdma_cma.h>
#include <stdlib.h>

#include "cmocka_headers.h"
#include "mocks-ibverbs.h"
#include "mocks-rpma-conn_cfg.h"
#include "cq-common.h"

/*
 * setup__cq_new -- prepare a valid cq object
 */
int
setup__cq_new(void **cq_ptr)
{
	struct rpma_cq *cq = NULL;

	/* configure mocks */
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	expect_value(ibv_create_cq, cqe, MOCK_CQ_SIZE_DEFAULT);
	will_return(ibv_create_cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_OK);

	/* run test */
	int ret = rpma_cq_new(MOCK_VERBS, MOCK_CQ_SIZE_DEFAULT, &cq);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);

	*cq_ptr = cq;

	return 0;
}

/*
 * teardown__cq_delete -- destroy the cq object
 */
int
teardown__cq_delete(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;

	/* configure mocks */
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);

	/* run test */
	int ret = rpma_cq_delete(&cq);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cq);

	return 0;
}

/*
 * group_setup_common_cq -- prepare common resources
 * for all tests in the group
 */
int
group_setup_common_cq(void **unused)
{
	/* set the req_notify_cq callback in mock of IBV CQ */
	MOCK_VERBS->ops.req_notify_cq = ibv_req_notify_cq_mock;
	Ibv_cq.context = MOCK_VERBS;

	return 0;
}
