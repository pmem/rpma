// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * conn_cfg-compl_channel.c -- the rpma_conn_cfg_set/get_compl_channel()
 *		unit tests
 *
 * APIs covered:
 * - rpma_conn_cfg_set_compl_channel()
 * - rpma_conn_cfg_get_compl_channel()
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
	int ret = rpma_conn_cfg_set_compl_channel(NULL, true);

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
	bool shared;
	int ret = rpma_conn_cfg_get_compl_channel(NULL, &shared);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get__shared_NULL -- NULL shared is invalid
 */
static void
get__shared_NULL(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_get_compl_channel(cstate->cfg, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_default__success -- get the default value
 */
static void
get_default__success(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	bool shared;
	int ret = rpma_conn_cfg_get_compl_channel(cstate->cfg, &shared);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(shared, false);
}

/*
 * compl_channel__lifecycle -- happy day scenario
 */
static void
compl_channel__lifecycle(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_set_compl_channel(cstate->cfg, true);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	bool shared;
	ret = rpma_conn_cfg_get_compl_channel(cstate->cfg, &shared);
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(shared, true);
}


static const struct CMUnitTest test_compl_channel[] = {
	/* rpma_conn_cfg_set_compl_channel() unit tests */
	cmocka_unit_test(set__cfg_NULL),

	/* rpma_conn_cfg_get_compl_channel() unit tests */
	cmocka_unit_test(get__cfg_NULL),
	cmocka_unit_test_setup_teardown(get__shared_NULL,
		setup__conn_cfg, teardown__conn_cfg),
	cmocka_unit_test_setup_teardown(get_default__success,
		setup__conn_cfg, teardown__conn_cfg),

	/* rpma_conn_cfg_set/get_compl_channel() lifecycle */
	cmocka_unit_test_setup_teardown(compl_channel__lifecycle,
		setup__conn_cfg, teardown__conn_cfg),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_compl_channel, NULL, NULL);
}
