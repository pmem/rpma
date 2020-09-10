// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * peer_cfg-direct_write_to_pmem.c --
 * the rpma_peer_cfg_set/get_direct_write_to_pmem() unit tests
 *
 * APIs covered:
 * - rpma_peer_cfg_set_direct_write_to_pmem()
 * - rpma_peer_cfg_get_direct_write_to_pmem()
 */

#include "peer_cfg-common.h"
#include "test-common.h"

/*
 * set_dw2p__pcfg_NULL -- NULL pcfg is invalid
 */
static void
set_dw2p__pcfg_NULL(void **unused)
{
	/* run test */
	int ret = rpma_peer_cfg_set_direct_write_to_pmem(NULL, MOCK_SUPPORTED);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_dw2p__pcfg_NULL -- NULL pcfg is invalid
 */
static void
get_dw2p__pcfg_NULL(void **unused)
{
	/* run test */
	bool supported;
	int ret = rpma_peer_cfg_get_direct_write_to_pmem(NULL, &supported);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_dw2p__supported_NULL -- NULL supported is invalid
 */
static void
get_dw2p__supported_NULL(void **unused)
{
	/* run test */
	int ret = rpma_peer_cfg_get_direct_write_to_pmem(MOCK_PEER_PCFG, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_dw2p__pcfg_supported_NULL -- NULL pcfg and supported are invalid
 */
static void
get_dw2p__pcfg_supported_NULL(void **unused)
{
	/* run test */
	int ret = rpma_peer_cfg_get_direct_write_to_pmem(NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * direct_write_to_pmem__lifecycle -- happy day scenario
 */
static void
direct_write_to_pmem__lifecycle(void **cstate_ptr)
{
	struct peer_cfg_test_state *cstate = *cstate_ptr;

	/* run test of rpma_peer_cfg_get_direct_write_to_pmem() */
	bool supported;
	int ret = rpma_peer_cfg_get_direct_write_to_pmem(cstate->cfg,
			&supported);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	/* 'false' is the default value */
	assert_int_equal(supported, false);

	/* first 'true', then 'false' */
	for (int supp = 1; supp >= 0; supp--) {
		/* run test of rpma_peer_cfg_set_direct_write_to_pmem() */
		ret = rpma_peer_cfg_set_direct_write_to_pmem(cstate->cfg,
				(bool)supp);

		/* verify the results */
		assert_int_equal(ret, MOCK_OK);

		/* run test of rpma_peer_cfg_get_direct_write_to_pmem() */
		ret = rpma_peer_cfg_get_direct_write_to_pmem(cstate->cfg,
				&supported);

		/* verify the results */
		assert_int_equal(ret, MOCK_OK);
		assert_int_equal(supported, (bool)supp);
	}
}


static const struct CMUnitTest test_direct_write_to_pmem[] = {
	/* rpma_peer_cfg_set_direct_write_to_pmem() unit tests */
	cmocka_unit_test(set_dw2p__pcfg_NULL),

	/* rpma_peer_cfg_get_direct_write_to_pmem() unit tests */
	cmocka_unit_test(get_dw2p__pcfg_NULL),
	cmocka_unit_test(get_dw2p__supported_NULL),
	cmocka_unit_test(get_dw2p__pcfg_supported_NULL),

	/* rpma_peer_cfg_set/get_direct_write_to_pmem() lifecycle */
	cmocka_unit_test_setup_teardown(direct_write_to_pmem__lifecycle,
		setup__peer_cfg, teardown__peer_cfg),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_direct_write_to_pmem, NULL, NULL);
}
