// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * peer_cfg-common.c -- the peer_cfg unit tests common functions
 */

#include "peer_cfg-common.h"

/*
 * setup__peer_cfg -- prepare a new rpma_peer_cfg
 */
int
setup__peer_cfg(void **cstate_ptr)
{
	static struct peer_cfg_test_state cstate = {0};

	/* configure mocks */
	will_return(__wrap__test_malloc, MOCK_OK);

	/* prepare an object */
	int ret = rpma_peer_cfg_new(&cstate.cfg);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(cstate.cfg);

	*cstate_ptr = &cstate;

	return 0;
}

/*
 * teardown__peer_cfg -- delete the rpma_peer_cfg
 */
int
teardown__peer_cfg(void **cstate_ptr)
{
	struct peer_cfg_test_state *cstate = *cstate_ptr;

	/* prepare an object */
	int ret = rpma_peer_cfg_delete(&cstate->cfg);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cstate->cfg);

	*cstate_ptr = NULL;

	return 0;
}
