// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn-completion_wait.c -- the rpma_conn_completion_wait() unit tests
 *
 * API covered:
 * - rpma_conn_completion_wait()
 */

#include "conn-common.h"
#include "mocks-ibverbs.h"

/*
 * completion_wait__conn_NULL - NULL conn is invalid
 */
static void
completion_wait__conn_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_completion_wait(NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * completion_wait__cq_wait_E_PROVIDER - rpma_cq_wait()
 * fails with RPMA_E_PROVIDER
 */
static void
completion_wait__cq_wait_E_PROVIDER(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	will_return(rpma_cq_wait, RPMA_E_PROVIDER);

	/* run test */
	int ret = rpma_conn_completion_wait(cstate->conn);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * completion_wait__success - happy day scenario
 */
static void
completion_wait__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	will_return(rpma_cq_wait, MOCK_OK);

	/* run test */
	int ret = rpma_conn_completion_wait(cstate->conn);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
}

static const struct CMUnitTest tests_completion_wait[] = {
	/* rpma_conn_completion_wait() unit tests */
	cmocka_unit_test(completion_wait__conn_NULL),
	cmocka_unit_test_setup_teardown(
		completion_wait__cq_wait_E_PROVIDER,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(
		completion_wait__success,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_completion_wait,
			NULL, NULL);
}
