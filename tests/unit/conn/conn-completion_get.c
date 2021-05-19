// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn-completion_get.c -- the rpma_conn_completion_get() unit tests
 *
 * API covered:
 * - rpma_conn_completion_get()
 */

#include <string.h>
#include <arpa/inet.h>

#include "conn-common.h"
#include "mocks-ibverbs.h"
#include "test-common.h"

/*
 * completion_get__conn_NULL - NULL conn is invalid
 */
static void
completion_get__conn_NULL(void **unused)
{
	/* run test */
	struct rpma_completion cmpl = {0};
	int ret = rpma_conn_completion_get(NULL, &cmpl);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * completion_get__cmpl_NULL - NULL cmpl is invalid
 */
static void
completion_get__cmpl_NULL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_completion_get(cstate->conn, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * completion_get__cq_get_completion_E_PROVIDER -
 * rpma_cq_get_completion() fails with RPMA_E_PROVIDER
 */
static void
completion_get__cq_get_completion_E_PROVIDER(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	will_return(rpma_cq_get_completion, RPMA_E_PROVIDER);

	/* run test */
	struct rpma_completion cmpl = {0};
	int ret = rpma_conn_completion_get(cstate->conn, &cmpl);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * completion_get__success - happy day scenario
 */
static void
completion_get__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	will_return(rpma_cq_get_completion, MOCK_OK);

	/* run test */
	struct rpma_completion cmpl = {0};
	int ret = rpma_conn_completion_get(cstate->conn, &cmpl);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(memcmp(&cmpl, MOCK_COMPLETION,
			sizeof(struct rpma_completion)), 0);
}

static const struct CMUnitTest tests_completion_get[] = {
	/* rpma_conn_completion_get() unit tests */
	cmocka_unit_test(completion_get__conn_NULL),
	cmocka_unit_test_setup_teardown(
		completion_get__cmpl_NULL,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(
		completion_get__cq_get_completion_E_PROVIDER,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(
		completion_get__success,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_completion_get,
			NULL, NULL);
}
