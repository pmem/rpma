// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn-apply_remote_peer_cfg.c -- the connection apply_remote_peer_cfg
 * unit tests
 *
 * API covered:
 * - rpma_conn_apply_remote_peer_cfg()
 */

#include "conn-common.h"
#include "mocks-ibverbs.h"

/*
 * apply_remote_peer_cfg__conn_NULL -- conn NULL is invalid
 */
static void
apply_remote_peer_cfg__conn_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_apply_remote_peer_cfg(NULL, MOCK_PEER_PCFG);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
}

/*
 * apply_remote_peer_cfg__pcfg_NULL -- pcfg NULL is invalid
 */
static void
apply_remote_peer_cfg__pcfg_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_apply_remote_peer_cfg(MOCK_CONN, NULL);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
}

/*
 * apply_remote_peer_cfg__conn_pcfg_NULL -- conn and pcfg NULL are invalid
 */
static void
apply_remote_peer_cfg__conn_pcfg_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_apply_remote_peer_cfg(NULL, NULL);

	/* verify the results */
	assert_ptr_equal(ret, RPMA_E_INVAL);
}

/*
 * apply_remote_peer_cfg__success -- happy day scenario
 */
static void
apply_remote_peer_cfg__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_apply_remote_peer_cfg(cstate->conn, MOCK_PEER_PCFG);

	/* verify the results */
	assert_ptr_equal(ret, 0);
}

static const struct CMUnitTest tests_apply_remote_peer_cfg[] = {
	/* rpma_conn_apply_remote_peer_cfg() unit tests */
	cmocka_unit_test(apply_remote_peer_cfg__conn_NULL),
	cmocka_unit_test(apply_remote_peer_cfg__pcfg_NULL),
	cmocka_unit_test(apply_remote_peer_cfg__conn_pcfg_NULL),
	cmocka_unit_test_setup_teardown(
		apply_remote_peer_cfg__success,
		setup__conn_new, teardown__conn_delete),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_apply_remote_peer_cfg, NULL, NULL);
}
