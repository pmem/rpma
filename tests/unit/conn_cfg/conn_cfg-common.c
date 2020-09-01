// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn_cfg-common.c -- the conn_cfg unit tests common functions
 */

#include "conn_cfg-common.h"

/*
 * setup__conn_cfg -- prepare a new rpma_conn_cfg
 */
int
setup__conn_cfg(void **cstate_ptr)
{
	static struct conn_cfg_test_state cstate = {0};

	/* configure mocks */
	will_return(__wrap__test_malloc, MOCK_OK);

	/* prepare an object */
	int ret = rpma_conn_cfg_new(&cstate.cfg);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(cstate.cfg);

	*cstate_ptr = &cstate;

	return 0;
}

/*
 * teardown__conn_cfg -- delete the rpma_conn_cfg
 */
int
teardown__conn_cfg(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* prepare an object */
	int ret = rpma_conn_cfg_delete(&cstate->cfg);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cstate->cfg);

	*cstate_ptr = NULL;

	return 0;
}
