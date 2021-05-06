// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Fujitsu */

/*
 * cq-get_ibv_cq.c -- the rpma_cq_get_ibv_cq() unit tests
 *
 * API covered:
 * - rpma_cq_get_ibv_cq()
 */

#include "librpma.h"
#include "cmocka_headers.h"
#include "mocks-ibverbs.h"
#include "cq-common.h"

/*
 * get_ibv_cq__success -- happy day scenario
 */
static void
get_ibv_cq__success(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;
	struct ibv_cq *ibv_cq = NULL;

	/* run test */
	int ret = rpma_cq_get_ibv_cq(cq, &ibv_cq);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_int_equal(ibv_cq, MOCK_IBV_CQ);
}

static int
group_setup_get_ibv_cq(void **unused)
{
	/* set the req_notify_cq callback in mock of IBV CQ */
	MOCK_VERBS->ops.req_notify_cq = ibv_req_notify_cq_mock;
	Ibv_cq.context = MOCK_VERBS;

	return 0;
}

static const struct CMUnitTest tests_get_ibv_cq[] = {
	/* rpma_cq_get_fd() unit tests */
	cmocka_unit_test_setup_teardown(
		get_ibv_cq__success, setup__cq_new, teardown__cq_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_get_ibv_cq,
		group_setup_get_ibv_cq, NULL);
}
