// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn_cfg-rq_size.c -- the rpma_conn_cfg_set/get_use_srq() unit tests
 *
 * APIs covered:
 * - rpma_conn_cfg_set_use_srq()
 * - rpma_conn_cfg_get_use_srq()
 */

#include "conn_cfg-common.h"
#include "test-common.h"

/*
 * set__cfg_NULL -- NULL cfg is invalid
 */
static void
set__cfg_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_cfg_set_use_srq(NULL, MOCK_USE_SRQ);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get__cfg_NULL -- NULL cfg is invalid
 */
static void
get__cfg_NULL(void **unused)
{
	/* run test */
	bool use_srq;
	int ret = rpma_conn_cfg_get_use_srq(NULL, &use_srq);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get__use_srq_NULL -- NULL rq_size is invalid
 */
static void
get__use_srq_NULL(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_get_use_srq(cstate->cfg, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * use_srq__lifecycle -- happy day scenario
 */
static void
use_srq__lifecycle(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_set_use_srq(cstate->cfg, MOCK_USE_SRQ);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	bool use_srq;
	ret = rpma_conn_cfg_get_use_srq(cstate->cfg, &use_srq);
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(use_srq, MOCK_USE_SRQ);
}


static const struct CMUnitTest test_use_srq[] = {
	/* rpma_conn_cfg_set_use_srq() unit tests */
	cmocka_unit_test(set__cfg_NULL),

	/* rpma_conn_cfg_get_use_srq() unit tests */
	cmocka_unit_test(get__cfg_NULL),
	cmocka_unit_test_setup_teardown(get__use_srq_NULL,
		setup__conn_cfg, teardown__conn_cfg),

	/* rpma_conn_cfg_set/get_use_srq() lifecycle */
	cmocka_unit_test_setup_teardown(use_srq__lifecycle,
		setup__conn_cfg, teardown__conn_cfg),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_use_srq, NULL, NULL);
}
