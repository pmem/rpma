// SPDX-License-Identifier: BSD-3-Clause
/* Copyright (c) 2021 Fujitsu */

/*
 * mr-local.c -- local memory registration object getters unit tests
 *
 * APIs covered:
 * - rpma_mr_get_size()
 * - rpma_mr_get_ptr()
 */

#include <stdlib.h>
#include <infiniband/verbs.h>

#include "mocks-rpma-peer.h"
#include "mr-common.h"
#include "test-common.h"

/* rpma_mr_get_size() unit tests */

/*
 * get_size__mr_NULL - NULL mr is invalid
 */
static void
get_size__mr_NULL(void **unused)
{
	size_t size;

	/* run test */
	int ret = rpma_mr_get_size(NULL, &size);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_size__size_NULL - NULL size is invalid
 */
static void
get_size__size_NULL(void **pprestate)
{
	struct prestate *prestate = *pprestate;
	struct rpma_mr_local *mr = prestate->mr;

	/* run test */
	int ret = rpma_mr_get_size(mr, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_size__mr_size_NULL - NULL mr and NULL size are invalid
 */
static void
get_size__mr_size_NULL(void **unused)
{
	/* run test */
	int ret = rpma_mr_get_size(NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_size__success - happy day scenario
 */
static void
get_size__success(void **pprestate)
{
	struct prestate *prestate = *pprestate;
	struct rpma_mr_local *mr = prestate->mr;
	size_t size;

	/* run test */
	int ret = rpma_mr_get_size(mr, &size);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(size, MOCK_SIZE);
}

/* rpma_mr_get_ptr() unit tests */

/*
 * get_ptr__mr_NULL - NULL mr is invalid
 */
static void
get_ptr__mr_NULL(void **unused)
{
	void *ptr = NULL;

	/* run test */
	int ret = rpma_mr_get_ptr(NULL, &ptr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_ptr__ptr_NULL - NULL ptr is invalid
 */
static void
get_ptr__ptr_NULL(void **pprestate)
{
	struct prestate *prestate = *pprestate;
	struct rpma_mr_local *mr = prestate->mr;

	/* run test */
	int ret = rpma_mr_get_ptr(mr, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_ptr__mr_ptr_NULL - NULL mr and NULL ptr are invalid
 */
static void
get_ptr__mr_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_mr_get_ptr(NULL, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_ptr__success - happy day scenario
 */
static void
get_ptr__success(void **pprestate)
{
	struct prestate *prestate = *pprestate;
	struct rpma_mr_local *mr = prestate->mr;
	void *ptr = NULL;

	/* run test */
	int ret = rpma_mr_get_ptr(mr, &ptr);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(ptr, MOCK_PTR);
}

static struct prestate prestate =
		{RPMA_MR_USAGE_READ_SRC | RPMA_MR_USAGE_READ_DST,
		IBV_ACCESS_REMOTE_READ | IBV_ACCESS_LOCAL_WRITE, NULL};

static const struct CMUnitTest tests_mr_local[] = {
	/* rpma_mr_get_size() unit tests */
	cmocka_unit_test(get_size__mr_NULL),
	cmocka_unit_test_prestate_setup_teardown(
		get_size__size_NULL,
		setup__reg_success,
		teardown__dereg_success,
		&prestate),
	cmocka_unit_test(get_size__mr_size_NULL),
	cmocka_unit_test_prestate_setup_teardown(
		get_size__success,
		setup__reg_success,
		teardown__dereg_success,
		&prestate),

	/* rpma_mr_get_ptr() unit tests */
	cmocka_unit_test(get_ptr__mr_NULL),
	cmocka_unit_test_prestate_setup_teardown(
		get_ptr__ptr_NULL,
		setup__reg_success,
		teardown__dereg_success,
		&prestate),
	cmocka_unit_test(get_ptr__mr_ptr_NULL),
	cmocka_unit_test_prestate_setup_teardown(
		get_ptr__success,
		setup__reg_success,
		teardown__dereg_success,
		&prestate),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_mr_local, NULL, NULL);
}
