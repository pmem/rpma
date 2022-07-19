// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Fujitsu */

/*
 * srq-get_rcq.c -- the rpma_srq_get_rcq() unit tests
 *
 * APIs covered:
 * - rpma_srq_get_rcq()
 */

#include "srq-common.h"

/*
 * get_rcq__srq_NULL -- srq NULL is invalid
 */
static void
get_rcq__srq_NULL(void **unused)
{
	struct rpma_cq *rcq = NULL;

	/* run test */
	int ret = rpma_srq_get_rcq(NULL, &rcq);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_rcq__rcq_ptr_NULL -- rcq_ptr NULL is invalid
 */
static void
get_rcq__rcq_ptr_NULL(void **cstate_ptr)
{
	struct srq_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_srq_get_rcq(cstate->srq, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_rcq__success -- happy day scenario
 */
static void
get_rcq__success(void **cstate_ptr)
{
	struct srq_test_state *cstate = *cstate_ptr;

	struct rpma_cq *rcq = NULL;

	/* run test */
	int ret = rpma_srq_get_rcq(cstate->srq, &rcq);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_ptr_equal(rcq, MOCK_GET_SRQ_RCQ(cstate));
}

static const struct CMUnitTest tests_get_rcq[] = {
	/* rpma_srq_get_rcq() unit tests */
	cmocka_unit_test(get_rcq__srq_NULL),
	cmocka_unit_test_prestate_setup_teardown(
		get_rcq__rcq_ptr_NULL, setup__srq_new, teardown__srq_delete,
		&Srq_new_srq_cfg_default),
	SRQ_NEW_TEST_SETUP_TEARDOWN_WITHOUT_CFG(
		get_rcq__success, setup__srq_new, teardown__srq_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_get_rcq, NULL, NULL);
}
