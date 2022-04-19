// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn-get_cq_rcq.c -- the rpma_conn_get_cq/rcq() unit tests
 *
 * APIs covered:
 * - rpma_conn_get_cq()
 * - rpma_conn_get_rcq()
 */

#include "conn-common.h"
#include "mocks-ibverbs.h"

/*
 * get_cq__conn_NULL -- conn NULL is invalid
 */
static void
get_cq__conn_NULL(void **unused)
{
	struct rpma_cq *cq = NULL;

	/* run test */
	int ret = rpma_conn_get_cq(NULL, &cq);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_cq__cq_ptr_NULL -- cq_ptr NULL is invalid
 */
static void
get_cq__cq_ptr_NULL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_get_cq(cstate->conn, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_cq__success -- happy day scenario
 */
static void
get_cq__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;
	struct rpma_cq *cq = NULL;

	/* run test */
	int ret = rpma_conn_get_cq(cstate->conn, &cq);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_ptr_equal(cq, MOCK_RPMA_CQ);
}

/*
 * get_rcq__conn_NULL -- conn NULL is invalid
 */
static void
get_rcq__conn_NULL(void **unused)
{
	struct rpma_cq *rcq = NULL;

	/* run test */
	int ret = rpma_conn_get_rcq(NULL, &rcq);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_rcq__rcq_ptr_NULL -- rcq_ptr NULL is invalid
 */
static void
get_rcq__rcq_ptr_NULL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_get_rcq(cstate->conn, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_rcq__success -- happy day scenario
 */
static void
get_rcq__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	struct rpma_cq *rcq = NULL;

	/* run test */
	int ret = rpma_conn_get_rcq(cstate->conn, &rcq);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_ptr_equal(rcq, cstate->rcq);
}

static const struct CMUnitTest tests_get_cq_rcq[] = {
	/* rpma_conn_get_cq() unit tests */
	cmocka_unit_test(get_cq__conn_NULL),
	cmocka_unit_test_setup_teardown(
		get_cq__cq_ptr_NULL, setup__conn_new, teardown__conn_delete),
	CONN_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_RCQ_CHANNEL(
		get_cq__success, setup__conn_new, teardown__conn_delete),

	/* rpma_conn_get_rcq() unit tests */
	cmocka_unit_test(get_rcq__conn_NULL),
	cmocka_unit_test_setup_teardown(
		get_rcq__rcq_ptr_NULL, setup__conn_new, teardown__conn_delete),
	CONN_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_RCQ_CHANNEL(
		get_rcq__success, setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_get_cq_rcq, NULL, NULL);
}
