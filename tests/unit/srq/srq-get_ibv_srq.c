// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Fujitsu */

/*
 * srq-get_ibv_srq.c -- the rpma_srq_get_ibv_srq() unit tests
 *
 * APIs covered:
 * - rpma_srq_get_ibv_srq()
 */

#include "srq-common.h"

/*
 * get_ibv_srq__success -- happy day scenario
 */
static void
get_ibv_srq__success(void **cstate_ptr)
{
	struct srq_test_state *cstate = *cstate_ptr;

	/* run test */
	struct ibv_srq *ibv_srq = rpma_srq_get_ibv_srq(cstate->srq);

	/* verify the results */
	assert_ptr_equal(ibv_srq, MOCK_IBV_SRQ);
}

static const struct CMUnitTest tests_get_ibv_srq[] = {
	/* rpma_srq_get_ibv_srq() unit tests */
	cmocka_unit_test_prestate_setup_teardown(
		get_ibv_srq__success, setup__srq_new, teardown__srq_delete,
		&Srq_new_srq_cfg_default),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_get_ibv_srq, NULL, NULL);
}
