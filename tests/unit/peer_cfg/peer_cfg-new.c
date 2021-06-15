// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * peer_cfg-new.c -- the rpma_peer_cfg_new() unit tests
 *
 * API covered:
 * - rpma_peer_cfg_new()
 */

#include "peer_cfg.h"
#include "peer_cfg-common.h"

/*
 * new__pcfg_NULL -- NULL pcfg is invalid
 */
static void
new__pcfg_NULL(void **unused)
{
	/* configure mocks */
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	int ret = rpma_peer_cfg_new(NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new__malloc_ERRNO -- malloc() fails with MOCK_ERRNO
 */
static void
new__malloc_ERRNO(void **unused)
{
	/* configure mocks */
	will_return(__wrap__test_malloc, MOCK_ERRNO);

	/* run test */
	struct rpma_peer_cfg *cfg = NULL;
	int ret = rpma_peer_cfg_new(&cfg);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(cfg);
}

/*
 * new__success -- all is OK
 */
static void
new__success(void **cstate_ptr)
{
	/* all things are done in setup__peer_cfg and teardown__peer_cfg */
}

static const struct CMUnitTest test_new[] = {
	/* rpma_peer_cfg_new() unit tests */
	cmocka_unit_test(new__pcfg_NULL),
	cmocka_unit_test(new__malloc_ERRNO),
	cmocka_unit_test_setup_teardown(new__success,
			setup__peer_cfg, teardown__peer_cfg),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_new, NULL, NULL);
}
