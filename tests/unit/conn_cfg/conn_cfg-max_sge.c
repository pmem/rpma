// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn_cfg-max_sge.c -- the rpma_conn_cfg_set/get_max_sge() unit tests
 *
 * APIs covered:
 * - rpma_conn_cfg_set_max_sge()
 * - rpma_conn_cfg_get_max_sge()
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
	int ret = rpma_conn_cfg_set_max_sge(NULL, MOCK_MAX_SGE);

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
	uint32_t max_sge;
	int ret = rpma_conn_cfg_get_max_sge(NULL, &max_sge);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get__max_sge_NULL -- NULL max_sge is invalid
 */
static void
get__max_sge_NULL(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_get_max_sge(cstate->cfg, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * max_sge__lifecycle -- happy day scenario
 */
static void
max_sge__lifecycle(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_set_max_sge(cstate->cfg, MOCK_MAX_SGE);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	uint32_t max_sge;
	ret = rpma_conn_cfg_get_max_sge(cstate->cfg, &max_sge);
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(max_sge, MOCK_MAX_SGE);
}


static const struct CMUnitTest test_max_sge[] = {
	/* rpma_conn_cfg_set_max_sge() unit tests */
	cmocka_unit_test(set__cfg_NULL),

	/* rpma_conn_cfg_get_max_sge() unit tests */
	cmocka_unit_test(get__cfg_NULL),
	cmocka_unit_test_setup_teardown(get__max_sge_NULL,
		setup__conn_cfg, teardown__conn_cfg),

	/* rpma_conn_cfg_set/get_max_sge() lifecycle */
	cmocka_unit_test_setup_teardown(max_sge__lifecycle,
		setup__conn_cfg, teardown__conn_cfg),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_max_sge, NULL, NULL);
}
