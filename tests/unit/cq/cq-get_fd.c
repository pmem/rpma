// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Fujitsu */

/*
 * cq-get_fd.c -- the rpma_cq_get_fd() unit tests
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
}

/*
 * get_fd__fd_NULL -- fd NULL is invalid
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
	assert_int_equal(ret, 0);
	assert_int_equal(fd, MOCK_COMPLETION_FD);
}

/*
 * group_setup_get_fd -- prepare resources for all tests in the group
 */
static int
group_setup_get_fd(void **unused)
{
	Ibv_comp_channel.fd = MOCK_COMPLETION_FD;

	return group_setup_common_cq(NULL);
}

static const struct CMUnitTest tests_get_fd[] = {
	/* rpma_cq_get_fd() unit tests */
	cmocka_unit_test(get_fd__cq_NULL),
	cmocka_unit_test_setup_teardown(
		get_fd__fd_NULL, setup__cq_new, teardown__cq_delete),
	cmocka_unit_test_setup_teardown(
		get_fd__success, setup__cq_new, teardown__cq_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_get_fd,
			group_setup_get_fd, NULL);
}
