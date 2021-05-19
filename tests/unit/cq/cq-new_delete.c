// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Fujitsu */

/*
 * cq-new_delete.c -- the rpma_cq_new/delete() unit tests
 *
 * APIs covered:
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
 * new__create_comp_channel_ERRNO -- ibv_create_comp_channel()
 * fails with MOCK_ERRNO
 */
static void
new__create_comp_channel_ERRNO(void **unused)
{
	struct rpma_cq *cq = NULL;

	/* configure mocks */
	will_return(ibv_create_comp_channel, NULL);
	will_return(ibv_create_comp_channel, MOCK_ERRNO);

	/* run test */
	int ret = rpma_cq_new(MOCK_VERBS, MOCK_CQ_SIZE_DEFAULT, &cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * new__create_cq_ERRNO -- ibv_create_cq() fails with MOCK_ERRNO
 */
static void
new__create_cq_ERRNO(void **unused)
{
	struct rpma_cq *cq = NULL;

	/* configure mocks */
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	expect_value(ibv_create_cq, cqe, MOCK_CQ_SIZE_DEFAULT);
	will_return(ibv_create_cq, NULL);
	will_return(ibv_create_cq, MOCK_ERRNO);
	will_return(ibv_destroy_comp_channel, MOCK_OK);

	/* run test */
	int ret = rpma_cq_new(MOCK_VERBS, MOCK_CQ_SIZE_DEFAULT, &cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * new__create_cq_ERRNO_subsequent_EIO -- ibv_create_cq() fails
 * with MOCK_ERRNO whereas subsequent ibv_destroy_comp_channel()
 * fails with EIO
 */
static void
new__create_cq_ERRNO_subsequent_EIO(void **unused)
{
	struct rpma_cq *cq = NULL;

	/* configure mocks */
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	expect_value(ibv_create_cq, cqe, MOCK_CQ_SIZE_DEFAULT);
	will_return(ibv_create_cq, NULL);
	will_return(ibv_create_cq, MOCK_ERRNO);
	will_return(ibv_destroy_comp_channel, EIO);

	/* run test */
	int ret = rpma_cq_new(MOCK_VERBS, MOCK_CQ_SIZE_DEFAULT, &cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * new__req_notify_cq_ERRNO -- ibv_req_notify_cq() fails with MOCK_ERRNO
 */
static void
new__req_notify_cq_ERRNO(void **unused)
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
 * new__req_notify_cq_ERRNO_subsequent_EIO -- ibv_req_notify_cq()
 * fails with MOCK_ERRNO whereas subsequent ibv_destroy_cq() and
 * ibv_destroy_comp_channel() fail with EIO
 */
static void
new__req_notify_cq_ERRNO_subsequent_EIO(void **unused)
{
	struct rpma_cq *cq = NULL;

	/* configure mocks */
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	expect_value(ibv_create_cq, cqe, MOCK_CQ_SIZE_DEFAULT);
	will_return(ibv_create_cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_ERRNO);
	will_return(ibv_destroy_cq, EIO);
	will_return(ibv_destroy_comp_channel, EIO);

	/* run test */
	int ret = rpma_cq_new(MOCK_VERBS, MOCK_CQ_SIZE_DEFAULT, &cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * new__malloc_ERRNO -- malloc() fails with MOCK_ERRNO
 */
static void
new__malloc_ERRNO(void **unused)
{
	struct rpma_cq *cq = NULL;

	/* configure mocks */
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	expect_value(ibv_create_cq, cqe, MOCK_CQ_SIZE_DEFAULT);
	will_return(ibv_create_cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_ERRNO);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);

	/* run test */
	int ret = rpma_cq_new(MOCK_VERBS, MOCK_CQ_SIZE_DEFAULT, &cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NOMEM);
}

/*
 * new__malloc_ERRNO_subsequent_EIO -- malloc() fails with MOCK_ERRNO whereas
 * subsequent ibv_destroy_cq() and ibv_destroy_comp_channel() fail with EIO
 */
static void
new__malloc_ERRNO_subsequent_EIO(void **unused)
{
	struct rpma_cq *cq = NULL;

	/* configure mocks */
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	expect_value(ibv_create_cq, cqe, MOCK_CQ_SIZE_DEFAULT);
	will_return(ibv_create_cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_ERRNO);
	will_return(ibv_destroy_cq, EIO);
	will_return(ibv_destroy_comp_channel, EIO);

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
 * delete__destroy_cq_ERRNO -- ibv_destroy_cq() fails with MOCK_ERRNO
 */
static void
delete__destroy_cq_ERRNO(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;

	/* configure mocks */
	will_return(ibv_destroy_cq, MOCK_ERRNO);
	will_return(ibv_destroy_comp_channel, MOCK_OK);

	/* run test */
	int ret = rpma_cq_delete(&cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * delete__destroy_cq_ERRNO_subsequent_EIO -- ibv_destroy_cq()
 * fails with MOCK_ERRNO whereas subsequent ibv_destroy_comp_channel()
 * fails with EIO
 */
static void
delete__destroy_cq_ERRNO_subsequent_EIO(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;

	/* configure mocks */
	will_return(ibv_destroy_cq, MOCK_ERRNO);
	will_return(ibv_destroy_comp_channel, EIO);

	/* run test */
	int ret = rpma_cq_delete(&cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * delete__destroy_comp_channel_ERRNO -- ibv_destroy_comp_channel()
 * fails with MOCK_ERRNO
 */
static void
delete__destroy_comp_channel_ERRNO(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;

	/* configure mocks */
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_ERRNO);

	/* run test */
	int ret = rpma_cq_delete(&cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

static const struct CMUnitTest tests_new_delete[] = {
	/* rpma_cq_new() unit tests */
	cmocka_unit_test(new__create_comp_channel_ERRNO),
	cmocka_unit_test(new__create_cq_ERRNO),
	cmocka_unit_test(new__create_cq_ERRNO_subsequent_EIO),
	cmocka_unit_test(new__req_notify_cq_ERRNO),
	cmocka_unit_test(new__req_notify_cq_ERRNO_subsequent_EIO),
	cmocka_unit_test(new__malloc_ERRNO),
	cmocka_unit_test(new__malloc_ERRNO_subsequent_EIO),

	/* rpma_cq_new()/delete() lifecycle */
	cmocka_unit_test_setup_teardown(test_lifecycle,
		setup__cq_new, teardown__cq_delete),

	/* rpma_cq_delete() unit tests */
	cmocka_unit_test_setup_teardown(delete__destroy_cq_ERRNO,
		setup__cq_new, NULL),
	cmocka_unit_test_setup_teardown(
		delete__destroy_cq_ERRNO_subsequent_EIO,
		setup__cq_new, NULL),
	cmocka_unit_test_setup_teardown(delete__destroy_comp_channel_ERRNO,
		setup__cq_new, NULL),

	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_new_delete,
		group_setup_common_cq, NULL);
}
