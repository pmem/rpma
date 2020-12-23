// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn_cfg-max_wr.c -- the rpma_conn_cfg_set/get_max_wr() unit tests
 *
 * APIs covered:
 * - rpma_conn_cfg_set_max_wr()
 * - rpma_conn_cfg_get_max_wr()
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
	int ret = rpma_conn_cfg_set_max_wr(NULL, MOCK_MAX_WR);

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
	uint32_t max_wr;
	int ret = rpma_conn_cfg_get_max_wr(NULL, &max_wr);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get__max_wr_NULL -- NULL max_wr is invalid
 */
static void
get__max_wr_NULL(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_get_max_wr(cstate->cfg, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * max_wr__lifecycle -- happy day scenario
 */
static void
max_wr__lifecycle(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_set_max_wr(cstate->cfg, MOCK_MAX_WR);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	uint32_t max_wr;
	ret = rpma_conn_cfg_get_max_wr(cstate->cfg, &max_wr);
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(max_wr, MOCK_MAX_WR);
}


static const struct CMUnitTest test_max_wr[] = {
	/* rpma_conn_cfg_set_max_wr() unit tests */
	cmocka_unit_test(set__cfg_NULL),

	/* rpma_conn_cfg_get_max_wr() unit tests */
	cmocka_unit_test(get__cfg_NULL),
	cmocka_unit_test_setup_teardown(get__max_wr_NULL,
		setup__conn_cfg, teardown__conn_cfg),

	/* rpma_conn_cfg_set/get_max_wr() lifecycle */
	cmocka_unit_test_setup_teardown(max_wr__lifecycle,
		setup__conn_cfg, teardown__conn_cfg),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_max_wr, NULL, NULL);
}
