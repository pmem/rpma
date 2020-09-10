// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * peer_cfg-delete.c -- the rpma_peer_cfg_delete() unit tests
 *
 * APIs covered:
 * - rpma_peer_cfg_delete()
 */

#include "peer_cfg-common.h"

/*
 * delete__pcfg_NULL -- NULL pcfg is invalid
 */
static void
delete__pcfg_NULL(void **unused)
{
	/* run test */
	int ret = rpma_peer_cfg_delete(NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * delete__cfg_NULL -- NULL cfg is valid - quick exit
 */
static void
delete__cfg_NULL(void **unused)
{
	/* run test */
	struct rpma_peer_cfg *cfg = NULL;
	int ret = rpma_peer_cfg_delete(&cfg);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cfg);
}

static const struct CMUnitTest test_delete[] = {
	/* rpma_peer_cfg_delete() unit tests */
	cmocka_unit_test(delete__pcfg_NULL),
	cmocka_unit_test(delete__cfg_NULL),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_delete, NULL, NULL);
}
