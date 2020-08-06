/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * ep-get_fd.c -- the endpoint unit tests of the following API:
 *
 * - rpma_ep_get_fd
 */

#include "librpma.h"
#include "ep-common.h"
#include "cmocka_headers.h"

/*
 * ep_get_fd__ep_NULL -- ep NULL is invalid
 */
static void
ep_get_fd__ep_NULL(void **unused)
{
	/* run test */
	int fd = 0;
	int ret = rpma_ep_get_fd(NULL, &fd);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
	assert_int_equal(fd, 0);
}

/*
 * ep_get_fd__fd_NULL - fd NULL is invalid
 */
static void
ep_get_fd__fd_NULL(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	/* run test */
	int ret = rpma_ep_get_fd(estate->ep, NULL);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
}

/*
 * ep_get_fd__ep_fd_NULL -- ep and fd NULL are invalid
 */
static void
ep_get_fd__ep_fd_NULL(void **unused)
{
	/* run test */
	int ret = rpma_ep_get_fd(NULL, NULL);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
}

/*
 * ep_get_fd__success - happy day scenario
 */
static void
ep_get_fd__success(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	/* run test */
	int fd = 0;
	int ret = rpma_ep_get_fd(estate->ep, &fd);

	/* verify the results */
	assert_ptr_equal(ret, MOCK_OK);
	assert_ptr_equal(fd, MOCK_FD);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_ep_get_fd() unit tests */
		cmocka_unit_test(ep_get_fd__ep_NULL),
		cmocka_unit_test_setup_teardown(
			ep_get_fd__fd_NULL,
			setup__ep_listen, teardown__ep_shutdown),
		cmocka_unit_test(ep_get_fd__ep_fd_NULL),
		cmocka_unit_test_setup_teardown(
			ep_get_fd__success,
			setup__ep_listen, teardown__ep_shutdown),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
