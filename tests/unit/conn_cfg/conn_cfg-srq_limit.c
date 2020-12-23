// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn_cfg-srq_limit.c -- the rpma_conn_cfg_set/get_srq_limit() unit tests
 *
 * APIs covered:
 * - rpma_conn_cfg_set_srq_limit()
 * - rpma_conn_cfg_get_srq_limit()
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
	int ret = rpma_conn_cfg_set_srq_limit(NULL, MOCK_SRQ_LIMIT);

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
	uint32_t srq_limit;
	int ret = rpma_conn_cfg_get_srq_limit(NULL, &srq_limit);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get__srq_limit_NULL -- NULL srq_limit is invalid
 */
static void
get__srq_limit_NULL(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_get_srq_limit(cstate->cfg, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * srq_limit__lifecycle -- happy day scenario
 */
static void
srq_limit__lifecycle(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_set_srq_limit(cstate->cfg, MOCK_SRQ_LIMIT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	uint32_t srq_limit;
	ret = rpma_conn_cfg_get_srq_limit(cstate->cfg, &srq_limit);
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(srq_limit, MOCK_SRQ_LIMIT);
}


static const struct CMUnitTest test_srq_limit[] = {
	/* rpma_conn_cfg_set_srq_limit() unit tests */
	cmocka_unit_test(set__cfg_NULL),

	/* rpma_conn_cfg_get_srq_limit() unit tests */
	cmocka_unit_test(get__cfg_NULL),
	cmocka_unit_test_setup_teardown(get__srq_limit_NULL,
		setup__conn_cfg, teardown__conn_cfg),

	/* rpma_conn_cfg_set/get_srq_limit() lifecycle */
	cmocka_unit_test_setup_teardown(srq_limit__lifecycle,
		setup__conn_cfg, teardown__conn_cfg),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_srq_limit, NULL, NULL);
}
