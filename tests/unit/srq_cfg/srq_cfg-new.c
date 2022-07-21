// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Fujitsu */

/*
 * srq_cfg-new.c -- the rpma_srq_cfg_new() unit tests
 *
 * API covered:
 * - rpma_srq_cfg_new()
 */

#include "srq_cfg-common.h"

/*
 * new__cfg_ptr_NULL -- NULL cfg_ptr is invalid
 */
static void
new__cfg_ptr_NULL(void **unused)
{
	/* configure mocks */
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	int ret = rpma_srq_cfg_new(NULL);

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
	struct rpma_srq_cfg *cfg = NULL;
	int ret = rpma_srq_cfg_new(&cfg);

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
	struct srq_cfg_test_state *cstate = *cstate_ptr;

	uint32_t rq_size, rcq_size;
	int ret;

	/* collect values and compare to defaults */
	ret = rpma_srq_cfg_get_rq_size(cstate->cfg, &rq_size);
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(rq_size, 100);

	ret = rpma_srq_cfg_get_rcq_size(cstate->cfg, &rcq_size);
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(rcq_size, 100);
}

static const struct CMUnitTest test_new[] = {
	/* rpma_srq_cfg_new() unit tests */
	cmocka_unit_test(new__cfg_ptr_NULL),
	cmocka_unit_test(new__malloc_ERRNO),
	cmocka_unit_test_setup_teardown(new__success,
			setup__srq_cfg, teardown__srq_cfg),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_new, NULL, NULL);
}
