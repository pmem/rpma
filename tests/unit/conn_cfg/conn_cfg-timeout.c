// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn_cfg-timeout.c -- the rpma_conn_cfg_set/get_timeout() unit tests
 *
 * APIs covered:
 * - rpma_conn_cfg_set_timeout()
 * - rpma_conn_cfg_get_timeout()
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
	int ret = rpma_conn_cfg_set_timeout(NULL, MOCK_TIMEOUT_MS);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * set__timeout_negative -- timeout_ms < 0 is invalid
 */
static void
set__timeout_negative(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* cache the before value */
	int before;
	int ret = rpma_conn_cfg_get_timeout(cstate->cfg, &before);
	assert_int_equal(ret, MOCK_OK);

	/* run test */
	ret = rpma_conn_cfg_set_timeout(cstate->cfg, -1);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	int after;
	ret = rpma_conn_cfg_get_timeout(cstate->cfg, &after);
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(before, after);
}

/*
 * get__cfg_NULL -- NULL cfg is invalid
 */
static void
get__cfg_NULL(void **unused)
{
	/* run test */
	int timeout_ms;
	int ret = rpma_conn_cfg_get_timeout(NULL, &timeout_ms);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get__timeout_NULL -- NULL timeout_ms is invalid
 */
static void
get__timeout_NULL(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_get_timeout(cstate->cfg, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * timeout__lifecycle -- happy day scenario
 */
static void
timeout__lifecycle(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_set_timeout(cstate->cfg, MOCK_TIMEOUT_MS);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	int timeout_ms;
	ret = rpma_conn_cfg_get_timeout(cstate->cfg, &timeout_ms);
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(timeout_ms, MOCK_TIMEOUT_MS);
}


static const struct CMUnitTest test_timeout[] = {
	/* rpma_conn_cfg_set_timeout() unit tests */
	cmocka_unit_test(set__cfg_NULL),
	cmocka_unit_test_setup_teardown(set__timeout_negative,
		setup__conn_cfg, teardown__conn_cfg),

	/* rpma_conn_cfg_get_timeout() unit tests */
	cmocka_unit_test(get__cfg_NULL),
	cmocka_unit_test_setup_teardown(get__timeout_NULL,
		setup__conn_cfg, teardown__conn_cfg),

	/* rpma_conn_cfg_set/get_timeout() lifecycle */
	cmocka_unit_test_setup_teardown(timeout__lifecycle,
		setup__conn_cfg, teardown__conn_cfg),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_timeout, NULL, NULL);
}
