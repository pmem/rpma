// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * flush-new.c -- unit tests of the flush module
 *
 * APIs covered:
 * - rpma_flush_new
 * - rpma_flush_delete
 */

#include "cmocka_headers.h"
#include "flush.h"
#include "flush-common.h"
#include "mocks-stdlib.h"
#include "test-common.h"
#include <sys/mman.h>

/*
 * new__malloc_ENOMEM -- malloc() fail with ENOMEM
 */
static void
new__malloc_ENOMEM(void **unused)
{
	/* configure mocks */
	will_return(__wrap__test_malloc, ENOMEM);

	/* run test */
	struct rpma_flush *flush = NULL;
	int ret = rpma_flush_new(MOCK_PEER, &flush);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(flush);
}

/*
 * new__apm_sysconf_EINVAL -- sysconf() fail with EINVAL
 */
static void
new__apm_sysconf_EINVAL(void **unused)
{
	/* configure mocks */
	will_return_always(__wrap__test_malloc, MOCK_OK);
	will_return(__wrap_sysconf, EINVAL);

	/* run test */
	struct rpma_flush *flush = NULL;
	int ret = rpma_flush_new(MOCK_PEER, &flush);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(flush);
}

/*
 * new__apm_mmap_ENOMEM -- malloc() fail with ENOMEM
 */
static void
new__apm_mmap_ENOMEM(void **unused)
{
	/* configure mocks */
	will_return_always(__wrap__test_malloc, MOCK_OK);
	will_return(__wrap_sysconf, MOCK_OK);
	will_return(__wrap_mmap, MAP_FAILED);

	/* run test */
	struct rpma_flush *flush = NULL;
	int ret = rpma_flush_new(MOCK_PEER, &flush);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(flush);
}

/*
 * new__apm_mr_reg_RPMA_E_NOMEM -- malloc() fail with ENOMEM
 */
static void
new__apm_mr_reg_RPMA_E_NOMEM(void **unused)
{
	/* configure mocks */
	will_return_always(__wrap__test_malloc, MOCK_OK);
	will_return(__wrap_sysconf, MOCK_OK);

	will_return(__wrap_mmap, MOCK_OK);
	struct mmap_args allocated_raw = {0};
	will_return(__wrap_mmap, &allocated_raw);
	expect_value(rpma_mr_reg, peer, MOCK_PEER);
	expect_value(rpma_mr_reg, size, 8);
	expect_value(rpma_mr_reg, usage, RPMA_MR_USAGE_READ_DST);
	will_return(rpma_mr_reg, &allocated_raw.ptr);
	will_return(rpma_mr_reg, NULL);
	will_return(rpma_mr_reg, RPMA_E_NOMEM);

	/* run test */
	struct rpma_flush *flush = NULL;
	int ret = rpma_flush_new(MOCK_PEER, &flush);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(flush);
}

/*
 * new__apm_malloc_ENOMEM -- malloc() fail with ENOMEM
 */
static void
new__apm_malloc_ENOMEM(void **unused)
{
	/* configure mocks */
	will_return(__wrap__test_malloc, MOCK_OK);
	will_return(__wrap_sysconf, MOCK_OK);

	will_return(__wrap_mmap, MOCK_OK);
	struct mmap_args allocated_raw = {0};
	will_return(__wrap_mmap, &allocated_raw);
	expect_value(rpma_mr_reg, peer, MOCK_PEER);
	expect_value(rpma_mr_reg, size, 8);
	expect_value(rpma_mr_reg, usage, RPMA_MR_USAGE_READ_DST);
	will_return(rpma_mr_reg, &allocated_raw.ptr);
	will_return(rpma_mr_reg, MOCK_RPMA_MR_LOCAL);
	will_return(__wrap__test_malloc, ENOMEM);
	expect_value(rpma_mr_dereg, *mr_ptr, MOCK_RPMA_MR_LOCAL);

	/* run test */
	struct rpma_flush *flush = NULL;
	int ret = rpma_flush_new(MOCK_PEER, &flush);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(flush);
}

/*
 * new__apm_success -- happy day scenario
 */
static void
new__apm_success(void **unused)
{
	/*
	 * The thing is done by setup__flush_new()
	 * and teardown__flush_delete().
	 */
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma__new() unit tests */
		cmocka_unit_test(new__malloc_ENOMEM),
		cmocka_unit_test(new__apm_sysconf_EINVAL),
		cmocka_unit_test(new__apm_mmap_ENOMEM),
		cmocka_unit_test(new__apm_mr_reg_RPMA_E_NOMEM),
		cmocka_unit_test(new__apm_malloc_ENOMEM),
		cmocka_unit_test_setup_teardown(new__apm_success,
			setup__flush_new, teardown__flush_delete),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
