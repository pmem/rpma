// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Fujitsu */

/*
 * cq-get__fd.c -- the rpma_cq_get_fd() unit tests
 *
 * API covered:
 * - rpma_cq_get_fd()
 */

#include "librpma.h"
#include "cmocka_headers.h"
#include "mocks-ibverbs.h"
#include "cq-common.h"

/*
 * get_fd__cq_NULL -- cq NULL is invalid
 */
static void
get_fd__cq_NULL(void **unused)
{
	/* run test */
	int fd = 0;
	int ret = rpma_cq_get_fd(NULL, &fd);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
	assert_int_equal(fd, 0);
}

/*
 * get_fd__fd_NULL -- cq and fd NULL are invalid
 */
static void
get_fd__fd_NULL(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;

	/* run test */
	int ret = rpma_cq_get_fd(cq, NULL);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
}

/*
 * get_fd__cq_fd_NULL -- fd NULL is invalid
 */
static void
get_fd__cq_fd_NULL(void **unused)
{
	/* run test */
	int ret = rpma_cq_get_fd(NULL, NULL);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
}

/*
 * get_fd__success -- happy day scenario
 */
static void
get_fd__success(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;

	/* run test */
	int fd = 0;
	int ret = rpma_cq_get_fd(cq, &fd);

	/* verify the results */
	assert_ptr_equal(ret, 0);
	assert_int_equal(fd, MOCK_COMPLETION_FD);
}

/*
 * group_setup_get_fd -- prepare resources for all tests in the group
 */
static int
group_setup_get_fd(void **unused)
{
	/* set the req_notify_cq callback in mock of IBV CQ */
	MOCK_VERBS->ops.req_notify_cq = ibv_req_notify_cq_mock;
	Ibv_cq.context = MOCK_VERBS;

	Ibv_comp_channel.fd = MOCK_COMPLETION_FD;

	return 0;
}

static const struct CMUnitTest tests_get_fd[] = {
	/* rpma_cq_get_fd() unit tests */
	cmocka_unit_test(get_fd__cq_NULL),
	cmocka_unit_test_setup_teardown(
		get_fd__fd_NULL, setup__cq_create, teardown__cq_destroy),
	cmocka_unit_test(get_fd__cq_fd_NULL),
	cmocka_unit_test_setup_teardown(
		get_fd__success, setup__cq_create, teardown__cq_destroy),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_get_fd,
			group_setup_get_fd, NULL);
}
