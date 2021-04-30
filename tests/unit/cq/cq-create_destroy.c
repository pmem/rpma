// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Fujitsu */

/*
 * cq-create-destroy.c -- the rpma_cq_create/destroy() unit tests
 *
 * API covered:
 * - rpma_cq_create()
 * - rpma_cq_destroy()
 */

#include <rdma/rdma_cma.h>
#include <stdlib.h>

#include "cmocka_headers.h"
#include "mocks-ibverbs.h"
#include "mocks-rpma-conn_cfg.h"
#include "cq-common.h"

/*
 * create__id_NULL -- id NULL is invalid
 */
static void
create__id_NULL(void **unused)
{
	struct rpma_cq *cq = NULL;

	/* run test */
	int ret = rpma_cq_create(NULL, MOCK_CQ_SIZE_DEFAULT, &cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * create__cq_ptr_NULL -- cq_ptr NULL is invalid
 */
static void
create__cq_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_cq_create(MOCK_VERBS, MOCK_CQ_SIZE_DEFAULT, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * create__id_cq_ptr_NULL -- id and cq_ptr NULL are invalid
 */
static void
create__id_cq_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_cq_create(NULL, MOCK_CQ_SIZE_DEFAULT, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * create__create_comp_channel_EAGAIN -- ibv_create_comp_channel()
 * fails with EAGAIN
 */
static void
create__create_comp_channel_EAGAIN(void **unused)
{
	struct rpma_cq *cq = NULL;

	/* configure mocks */
	will_return(ibv_create_comp_channel, NULL);
	will_return(ibv_create_comp_channel, EAGAIN);

	/* run test */
	int ret = rpma_cq_create(MOCK_VERBS, MOCK_CQ_SIZE_DEFAULT, &cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * create__create_cq_EAGAIN -- ibv_create_cq() fails with EAGAIN
 */
static void
create__create_cq_EAGAIN(void **unused)
{
	struct rpma_cq *cq = NULL;

	/* configure mocks */
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	expect_value(ibv_create_cq, cqe, MOCK_CQ_SIZE_DEFAULT);
	will_return(ibv_create_cq, NULL);
	will_return(ibv_create_cq, EAGAIN);
	will_return(ibv_destroy_comp_channel, MOCK_OK);

	/* run test */
	int ret = rpma_cq_create(MOCK_VERBS, MOCK_CQ_SIZE_DEFAULT, &cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * create__req_notify_cq_fail -- ibv_req_notify_cq_fail() fails with EAGAIN
 */
static void
create__req_notify_cq_fail(void **unused)
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
	int ret = rpma_cq_create(MOCK_VERBS, MOCK_CQ_SIZE_DEFAULT, &cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * create__malloc_ENOMEM -- malloc() fails with ENOMEM
 */
static void
create__malloc_ENOMEM(void **unused)
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
	int ret = rpma_cq_create(MOCK_VERBS, MOCK_CQ_SIZE_DEFAULT, &cq);

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
	 * the thing is done by setup__cq() and teardown__cq()
	 */
}

/*
 * destroy__cq_ptr_NULL -- cq_ptr NULL is invalid
 */
static void
destroy__cq_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_cq_destroy(NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * destroy__cq_NULL -- cq NULL is valid
 */
static void
destroy__cq_NULL(void **unused)
{
	struct rpma_cq *cq = NULL;

	/* run test */
	int ret = rpma_cq_destroy(&cq);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * destroy__destroy_cq_EAGAIN -- ibv_destroy_cq() fails with EAGAIN
 */
static void
destroy__destroy_cq_EAGAIN(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;

	/* configure mocks */
	will_return(ibv_destroy_cq, EAGAIN);
	will_return(ibv_destroy_comp_channel, MOCK_OK);

	/* run test */
	int ret = rpma_cq_destroy(&cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * destroy__destroy_comp_channel_EAGAIN -- ibv_destroy_comp_channel()
 * fails with EAGAIN
 */
static void
destroy__destroy_comp_channel_EAGAIN(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;

	/* configure mocks */
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, EAGAIN);

	/* run test */
	int ret = rpma_cq_destroy(&cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * group_setup_create_destroy -- prepare resources for all tests in the group
 */
static int
group_setup_create_destroy(void **unused)
{
	/* set the req_notify_cq callback in mock of IBV CQ */
	MOCK_VERBS->ops.req_notify_cq = ibv_req_notify_cq_mock;
	Ibv_cq.context = MOCK_VERBS;

	return 0;
}

static const struct CMUnitTest tests_create_destroy[] = {
	/* rpma_cq_create() unit tests */
	cmocka_unit_test(create__id_NULL),
	cmocka_unit_test(create__cq_ptr_NULL),
	cmocka_unit_test(create__id_cq_ptr_NULL),
	cmocka_unit_test(create__create_comp_channel_EAGAIN),
	cmocka_unit_test(create__create_cq_EAGAIN),
	cmocka_unit_test(create__req_notify_cq_fail),
	cmocka_unit_test(create__malloc_ENOMEM),

	/* rpma_cq_create()/_destroy() lifecycle */
	cmocka_unit_test_setup_teardown(test_lifecycle,
		setup__cq_create, teardown__cq_destroy),

	/* rpma_cq_destroy() unit tests */
	cmocka_unit_test(destroy__cq_ptr_NULL),
	cmocka_unit_test(destroy__cq_NULL),
	cmocka_unit_test_setup_teardown(destroy__destroy_cq_EAGAIN,
		setup__cq_create, NULL),
	cmocka_unit_test_setup_teardown(destroy__destroy_comp_channel_EAGAIN,
		setup__cq_create, NULL),

	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_create_destroy,
		group_setup_create_destroy, NULL);
}
