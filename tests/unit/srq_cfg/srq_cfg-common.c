// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Fujitsu */

/*
 * srq_cfg-common.c -- the srq_cfg unit tests common functions
 */

#include "srq_cfg-common.h"

/*
 * setup__srq_cfg -- prepare a new rpma_srq_cfg
 */
int
setup__srq_cfg(void **cstate_ptr)
{
	static struct srq_cfg_test_state cstate = {0};

	/* configure mocks */
	will_return(__wrap__test_malloc, MOCK_OK);

	/* prepare an object */
	int ret = rpma_srq_cfg_new(&cstate.cfg);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(cstate.cfg);

	*cstate_ptr = &cstate;

	return 0;
}

/*
 * teardown__srq_cfg -- delete the rpma_srq_cfg
 */
int
teardown__srq_cfg(void **cstate_ptr)
{
	struct srq_cfg_test_state *cstate = *cstate_ptr;

	/* prepare an object */
	int ret = rpma_srq_cfg_delete(&cstate->cfg);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cstate->cfg);

	*cstate_ptr = NULL;

	return 0;
}
