// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn_cfg-rq_size.c -- the rpma_conn_cfg_set/get_rq_size() unit tests
 *
 * APIs covered:
 * - rpma_conn_cfg_set_rq_size()
 * - rpma_conn_cfg_get_rq_size()
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
	int ret = rpma_conn_cfg_set_rq_size(NULL, MOCK_Q_SIZE);

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
	uint32_t rq_size = 0;
	int ret = rpma_conn_cfg_get_rq_size(NULL, &rq_size);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_int_equal(rq_size, 0);
}

/*
 * get__rq_size_NULL -- NULL rq_size is invalid
 */
static void
get__rq_size_NULL(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_get_rq_size(cstate->cfg, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * rq_size__lifecycle -- NULL rq_size is invalid
 */
static void
rq_size__lifecycle(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_set_rq_size(cstate->cfg, MOCK_Q_SIZE);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	uint32_t rq_size;
	ret = rpma_conn_cfg_get_rq_size(cstate->cfg, &rq_size);
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(rq_size, MOCK_Q_SIZE);
}


static const struct CMUnitTest test_connect[] = {
	/* rpma_conn_cfg_set_rq_size() unit tests */
	cmocka_unit_test(set__cfg_NULL),

	/* rpma_conn_cfg_get_rq_size() unit tests */
	cmocka_unit_test(get__cfg_NULL),
	cmocka_unit_test_setup_teardown(get__rq_size_NULL,
		setup__conn_cfg, teardown__conn_cfg),

	/* rpma_conn_cfg_set/get_rq_size() lifecycle */
	cmocka_unit_test_setup_teardown(rq_size__lifecycle,
		setup__conn_cfg, teardown__conn_cfg),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_connect, NULL, NULL);
}
