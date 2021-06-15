// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn_cfg-new.c -- the rpma_conn_cfg_new() unit tests
 *
 * API covered:
 * - rpma_conn_cfg_new()
 */

#include "conn_cfg.h"
#include "conn_cfg-common.h"

/*
 * new__cfg_ptr_NULL -- NULL cfg_ptr is invalid
 */
static void
new__cfg_ptr_NULL(void **unused)
{
	/* configure mocks */
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	int ret = rpma_conn_cfg_new(NULL);

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
	struct rpma_conn_cfg *cfg = NULL;
	int ret = rpma_conn_cfg_new(&cfg);

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
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* get the default configuration for comparison */
	struct rpma_conn_cfg *cfg_default = rpma_conn_cfg_default();

	int a;
	int b;
	uint32_t ua;
	uint32_t ub;
	int ret;

	/* collect values and compare to defaults */
	ret = rpma_conn_cfg_get_timeout(cstate->cfg, &a);
	assert_int_equal(ret, MOCK_OK);
	ret = rpma_conn_cfg_get_timeout(cfg_default, &b);
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(a, b);

	ret = rpma_conn_cfg_get_cq_size(cstate->cfg, &ua);
	assert_int_equal(ret, MOCK_OK);
	ret = rpma_conn_cfg_get_cq_size(cfg_default, &ub);
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(ua, ub);

	ret = rpma_conn_cfg_get_sq_size(cstate->cfg, &ua);
	assert_int_equal(ret, MOCK_OK);
	ret = rpma_conn_cfg_get_sq_size(cfg_default, &ub);
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(ua, ub);

	ret = rpma_conn_cfg_get_rq_size(cstate->cfg, &ua);
	assert_int_equal(ret, MOCK_OK);
	ret = rpma_conn_cfg_get_rq_size(cfg_default, &ub);
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(ua, ub);
}

static const struct CMUnitTest test_new[] = {
	/* rpma_conn_cfg_new() unit tests */
	cmocka_unit_test(new__cfg_ptr_NULL),
	cmocka_unit_test(new__malloc_ERRNO),
	cmocka_unit_test_setup_teardown(new__success,
			setup__conn_cfg, teardown__conn_cfg),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_new, NULL, NULL);
}
