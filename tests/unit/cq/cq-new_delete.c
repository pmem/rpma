// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Fujitsu */

/*
 * cq-new_delete.c -- the rpma_cq_new/delete() unit tests
 *
 * API covered:
 * - rpma_cq_new()
 * - rpma_cq_delete()
 */

#include <rdma/rdma_cma.h>
#include <stdlib.h>

#include "cmocka_headers.h"
#include "mocks-ibverbs.h"
#include "mocks-rpma-conn_cfg.h"
#include "cq-common.h"

/*
 * new__create_comp_channel_EAGAIN -- ibv_create_comp_channel()
 * fails with EAGAIN
 */
static void
new__create_comp_channel_EAGAIN(void **unused)
{
	struct rpma_cq *cq = NULL;

	/* configure mocks */
	will_return(ibv_create_comp_channel, NULL);
	will_return(ibv_create_comp_channel, EAGAIN);

	/* run test */
	int ret = rpma_cq_new(MOCK_VERBS, MOCK_CQ_SIZE_DEFAULT, &cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * new__create_cq_EAGAIN -- ibv_create_cq() fails with EAGAIN
 */
static void
new__create_cq_EAGAIN(void **unused)
{
	struct rpma_cq *cq = NULL;

	/* configure mocks */
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	expect_value(ibv_create_cq, cqe, MOCK_CQ_SIZE_DEFAULT);
	will_return(ibv_create_cq, NULL);
	will_return(ibv_create_cq, EAGAIN);
	will_return(ibv_destroy_comp_channel, MOCK_OK);

	/* run test */
	int ret = rpma_cq_new(MOCK_VERBS, MOCK_CQ_SIZE_DEFAULT, &cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * new__req_notify_cq_fail -- ibv_req_notify_cq_fail() fails with EAGAIN
 */
static void
new__req_notify_cq_fail(void **unused)
{
	struct rpma_cq *cq = NULL;

	/* configure mocks */
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	expect_value(ibv_create_cq, cqe, MOCK_CQ_SIZE_DEFAULT);
	will_return(ibv_create_cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_ERRNO);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);

	/* run test */
	int ret = rpma_cq_new(MOCK_VERBS, MOCK_CQ_SIZE_DEFAULT, &cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * new__malloc_ENOMEM -- malloc() fails with ENOMEM
 */
static void
new__malloc_ENOMEM(void **unused)
{
	struct rpma_cq *cq = NULL;

	/* configure mocks */
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	expect_value(ibv_create_cq, cqe, MOCK_CQ_SIZE_DEFAULT);
	will_return(ibv_create_cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_OK);
	will_return(__wrap__test_malloc, ENOMEM);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);

	/* run test */
	int ret = rpma_cq_new(MOCK_VERBS, MOCK_CQ_SIZE_DEFAULT, &cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NOMEM);
}

/*
 * test_lifecycle - happy day scenario
 */
static void
test_lifecycle(void **unused)
{
	/*
	 * the thing is done by setup__cq_new() and teardown__cq_delete()
	 */
}

/*
 * delete__destroy_cq_EAGAIN -- ibv_destroy_cq() fails with EAGAIN
 */
static void
delete__destroy_cq_EAGAIN(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;

	/* configure mocks */
	will_return(ibv_destroy_cq, EAGAIN);
	will_return(ibv_destroy_comp_channel, MOCK_OK);

	/* run test */
	int ret = rpma_cq_delete(&cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * delete__destroy_comp_channel_EAGAIN -- ibv_destroy_comp_channel()
 * fails with EAGAIN
 */
static void
delete__destroy_comp_channel_EAGAIN(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;

	/* configure mocks */
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, EAGAIN);

	/* run test */
	int ret = rpma_cq_delete(&cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * group_setup_new_delete -- prepare resources for all tests in the group
 */
static int
group_setup_new_delete(void **unused)
{
	/* set the req_notify_cq callback in mock of IBV CQ */
	MOCK_VERBS->ops.req_notify_cq = ibv_req_notify_cq_mock;
	Ibv_cq.context = MOCK_VERBS;

	return 0;
}

static const struct CMUnitTest tests_new_delete[] = {
	/* rpma_cq_new() unit tests */
	cmocka_unit_test(new__create_comp_channel_EAGAIN),
	cmocka_unit_test(new__create_cq_EAGAIN),
	cmocka_unit_test(new__req_notify_cq_fail),
	cmocka_unit_test(new__malloc_ENOMEM),

	/* rpma_cq_new()/delete() lifecycle */
	cmocka_unit_test_setup_teardown(test_lifecycle,
		setup__cq_new, teardown__cq_delete),

	/* rpma_cq_delete() unit tests */
	cmocka_unit_test_setup_teardown(delete__destroy_cq_EAGAIN,
		setup__cq_new, NULL),
	cmocka_unit_test_setup_teardown(delete__destroy_comp_channel_EAGAIN,
		setup__cq_new, NULL),

	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_new_delete,
		group_setup_new_delete, NULL);
}
