// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn_cfg-cq_size.c -- the rpma_conn_cfg_set/get_cq_size() unit tests
 *
 * APIs covered:
 * - rpma_conn_cfg_set_cq_size()
 * - rpma_conn_cfg_get_cq_size()
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
	int ret = rpma_conn_cfg_set_cq_size(NULL, MOCK_Q_SIZE);

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
	uint32_t cq_size = 0;
	int ret = rpma_conn_cfg_get_cq_size(NULL, &cq_size);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_int_equal(cq_size, 0);
}

/*
 * get__cq_size_NULL -- NULL cq_size is invalid
 */
static void
get__cq_size_NULL(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_get_cq_size(cstate->cfg, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * cq_size__lifecycle -- NULL cq_size is invalid
 */
static void
cq_size__lifecycle(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_set_cq_size(cstate->cfg, MOCK_Q_SIZE);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	uint32_t cq_size;
	ret = rpma_conn_cfg_get_cq_size(cstate->cfg, &cq_size);
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(cq_size, MOCK_Q_SIZE);
}


static const struct CMUnitTest test_connect[] = {
	/* rpma_conn_cfg_set_cq_size() unit tests */
	cmocka_unit_test(set__cfg_NULL),

	/* rpma_conn_cfg_get_cq_size() unit tests */
	cmocka_unit_test(get__cfg_NULL),
	cmocka_unit_test_setup_teardown(get__cq_size_NULL,
		setup__conn_cfg, teardown__conn_cfg),

	/* rpma_conn_cfg_set/get_cq_size() lifecycle */
	cmocka_unit_test_setup_teardown(cq_size__lifecycle,
		setup__conn_cfg, teardown__conn_cfg),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_connect, NULL, NULL);
}
