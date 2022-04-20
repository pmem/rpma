// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * conn-get_compl_fd.c -- the rpma_conn_get_compl_fd() unit tests
 *
 * APIs covered:
 * - rpma_conn_get_compl_fd()
 */

#include "conn-common.h"
#include "conn_cfg.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"
#include "test-common.h"
#include <infiniband/verbs.h>

/*
 * get_compl_fd__conn_NULL -- conn NULL is invalid
 */
static void
get_compl_fd__conn_NULL(void **unused)
{
	/* run test */
	int fd = 0;
	int ret = rpma_conn_get_compl_fd(NULL, &fd);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
	assert_int_equal(fd, 0);
}

/*
 * get_compl_fd__fd_ptr_NULL -- fd_ptr NULL is invalid
 */
static void
get_compl_fd__fd_ptr_NULL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_get_compl_fd(cstate->conn, NULL);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
}

/*
 * get_compl_fd__conn_fd_NULL -- conn and fd NULL are invalid
 */
static void
get_compl_fd__conn_fd_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_get_compl_fd(NULL, NULL);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
}

/*
 * get_compl_fd__E_NOT_SHARED_CHNL -- rpma_conn_get_compl_fd()
 * fails with RPMA_E_NOT_SHARED_CHNL
 */
static void
get_compl_fd__E_NOT_SHARED_CHNL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* run test */
	int fd = 0;
	int ret = rpma_conn_get_compl_fd(cstate->conn, &fd);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_NOT_SHARED_CHNL);
	assert_int_equal(fd, 0);
}

/*
 * get_compl_fd__success -- happy day scenario
 */
static void
get_compl_fd__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* run test */
	int fd = 0;
	int ret = rpma_conn_get_compl_fd(cstate->conn, &fd);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(fd, MOCK_COMPLETION_FD);
}

/*
 * group_setup_get_compl_fd -- prepare resources for all tests in the group
 */
static int
group_setup_get_compl_fd(void **unused)
{
	Ibv_comp_channel.fd = MOCK_COMPLETION_FD;
	return 0;
}

static const struct CMUnitTest tests_get_compl_fd[] = {
	/* rpma_conn_get_compl_fd() unit tests */
	cmocka_unit_test(get_compl_fd__conn_NULL),
	cmocka_unit_test_setup_teardown(
		get_compl_fd__fd_ptr_NULL,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(get_compl_fd__conn_fd_NULL),
	cmocka_unit_test_prestate_setup_teardown(
		get_compl_fd__E_NOT_SHARED_CHNL, setup__conn_new,
		teardown__conn_delete, &Conn_no_rcq_no_channel),
	cmocka_unit_test_prestate_setup_teardown(
		get_compl_fd__success, setup__conn_new,
		teardown__conn_delete, &Conn_with_rcq_with_channel),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_get_compl_fd,
			group_setup_get_compl_fd, NULL);
}
