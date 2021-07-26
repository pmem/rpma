// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Fujitsu */

/*
 * conn_cfg-cq_size.c -- the rpma_conn_cfg_set/get_rcq_size() unit tests
 *
 * APIs covered:
 * - rpma_conn_cfg_set_rcq_size()
 * - rpma_conn_cfg_get_rcq_size()
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
	int ret = rpma_conn_cfg_set_rcq_size(NULL, MOCK_Q_SIZE);

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
	uint32_t rcq_size;
	int ret = rpma_conn_cfg_get_rcq_size(NULL, &rcq_size);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get__rcq_size_NULL -- NULL rcq_size is invalid
 */
static void
get__rcq_size_NULL(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_get_rcq_size(cstate->cfg, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * rcq_size__lifecycle -- happy day scenario
 */
static void
rcq_size__lifecycle(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_set_rcq_size(cstate->cfg, MOCK_Q_SIZE);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);

	/* run test */
	uint32_t rcq_size;
	ret = rpma_conn_cfg_get_rcq_size(cstate->cfg, &rcq_size);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(rcq_size, MOCK_Q_SIZE);
}


static const struct CMUnitTest test_rcq_size[] = {
	/* rpma_conn_cfg_set_rcq_size() unit tests */
	cmocka_unit_test(set__cfg_NULL),

	/* rpma_conn_cfg_get_rcq_size() unit tests */
	cmocka_unit_test(get__cfg_NULL),
	cmocka_unit_test_setup_teardown(get__rcq_size_NULL,
		setup__conn_cfg, teardown__conn_cfg),

	/* rpma_conn_cfg_set/get_rcq_size() lifecycle */
	cmocka_unit_test_setup_teardown(rcq_size__lifecycle,
		setup__conn_cfg, teardown__conn_cfg),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_rcq_size, NULL, NULL);
}
