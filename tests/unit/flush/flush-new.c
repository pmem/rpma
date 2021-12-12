// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */
/* Copyright 2021, Fujitsu */

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
#include "mocks-unistd.h"
#include "test-common.h"
#include <sys/mman.h>

/*
 * new__malloc_ERRNO -- malloc() fails with MOCK_ERRNO
 */
static void
new__malloc_ERRNO(void **unused)
{
	/* configure mocks */
	will_return(__wrap__test_malloc, MOCK_ERRNO);

	/* run test */
	struct rpma_flush *flush = NULL;
	int ret = rpma_flush_new(MOCK_PEER, &flush);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(flush);
}

/*
 * new__apm_sysconf_ERRNO -- sysconf() fails with MOCK_ERRNO
 */
static void
new__apm_sysconf_ERRNO(void **unused)
{
	/* configure mocks */
	will_return_always(__wrap__test_malloc, MOCK_OK);
	will_return(__wrap_sysconf, MOCK_ERRNO);

	/* run test */
	struct rpma_flush *flush = NULL;
	int ret = rpma_flush_new(MOCK_PEER, &flush);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(flush);
}

/*
 * new__apm_mmap_MAP_FAILED -- mmap() fails with MAP_FAILED
 */
static void
new__apm_mmap_MAP_FAILED(void **unused)
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
 * new__apm_mr_reg_E_NOMEM_munmap_ERRNO -- munmap() fails with MOCK_ERRNO
 * after rpma_mr_reg() failed with RPMA_E_NOMEM
 */
static void
new__apm_mr_reg_E_NOMEM_munmap_ERRNO(void **unused)
{
	/* configure mocks */
	will_return_always(__wrap__test_malloc, MOCK_OK);
	will_return(__wrap_sysconf, MOCK_OK);

	struct mmap_args allocated_raw = {0};
	will_return(__wrap_mmap, MOCK_OK);
	will_return(__wrap_mmap, &allocated_raw);
	expect_value(rpma_mr_reg, peer, MOCK_PEER);
	expect_value(rpma_mr_reg, size, 8);
	expect_value(rpma_mr_reg, usage, RPMA_MR_USAGE_READ_DST);
	will_return(rpma_mr_reg, &allocated_raw.addr);
	will_return(rpma_mr_reg, NULL);
	will_return(rpma_mr_reg, RPMA_E_NOMEM);
	will_return(__wrap_munmap, &allocated_raw);
	will_return(__wrap_munmap, MOCK_ERRNO);

	/* run test */
	struct rpma_flush *flush = NULL;
	int ret = rpma_flush_new(MOCK_PEER, &flush);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(flush);
}

/*
 * new__apm_malloc_ERRNO -- malloc() fails with MOCK_ERRNO
 */
static void
new__apm_malloc_ERRNO(void **unused)
{
	/* configure mocks */
	will_return(__wrap__test_malloc, MOCK_OK);
	will_return(__wrap_sysconf, MOCK_OK);

	struct mmap_args allocated_raw = {0};
	will_return(__wrap_mmap, MOCK_OK);
	will_return(__wrap_mmap, &allocated_raw);
	expect_value(rpma_mr_reg, peer, MOCK_PEER);
	expect_value(rpma_mr_reg, size, 8);
	expect_value(rpma_mr_reg, usage, RPMA_MR_USAGE_READ_DST);
	will_return(rpma_mr_reg, &allocated_raw.addr);
	will_return(rpma_mr_reg, MOCK_RPMA_MR_LOCAL);
	will_return(__wrap__test_malloc, MOCK_ERRNO);
	will_return(rpma_mr_dereg, MOCK_OK);
	will_return(__wrap_munmap, &allocated_raw);
	will_return(__wrap_munmap, MOCK_OK);
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

/*
 * delete__apm_dereg_ERRNO -- rpma_mr_dereg() fails with MOCK_ERRNO
 */
static void
delete__apm_dereg_ERRNO(void **unused)
{
	struct flush_test_state *fstate;

	setup__flush_new((void **)&fstate);

	/* configure mocks */
	expect_value(rpma_mr_dereg, *mr_ptr, MOCK_RPMA_MR_LOCAL);
	will_return(__wrap_munmap, &fstate->allocated_raw);
	will_return_maybe(__wrap_munmap, MOCK_OK);
	will_return(rpma_mr_dereg, RPMA_E_PROVIDER);
	will_return(rpma_mr_dereg, MOCK_ERRNO);

	/* delete the object */
	int ret = rpma_flush_delete(&fstate->flush);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(fstate->flush);
}

/*
 * delete__apm_munmap_ERRNO -- munmap() fails with MOCK_ERRNO
 */
static void
delete__apm_munmap_ERRNO(void **unused)
{
	struct flush_test_state *fstate;

	setup__flush_new((void **)&fstate);

	/* configure mocks */
	expect_value(rpma_mr_dereg, *mr_ptr, MOCK_RPMA_MR_LOCAL);
	will_return_maybe(rpma_mr_dereg, MOCK_OK);
	will_return(__wrap_munmap, &fstate->allocated_raw);
	will_return(__wrap_munmap, MOCK_ERRNO);

	/* delete the object */
	int ret = rpma_flush_delete(&fstate->flush);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(fstate->flush);
}

int
main(int argc, char *argv[])
{
	enable_unistd_mocks();

	const struct CMUnitTest tests[] = {
		/* rpma_flush_new() unit tests */
		cmocka_unit_test(new__malloc_ERRNO),
		cmocka_unit_test(new__apm_sysconf_ERRNO),
		cmocka_unit_test(new__apm_mmap_MAP_FAILED),
		cmocka_unit_test(new__apm_mr_reg_E_NOMEM_munmap_ERRNO),
		cmocka_unit_test(new__apm_malloc_ERRNO),
		cmocka_unit_test_setup_teardown(new__apm_success,
			setup__flush_new, teardown__flush_delete),

		/* rpma_flush_delete() unit tests */
		cmocka_unit_test(delete__apm_dereg_ERRNO),
		cmocka_unit_test(delete__apm_munmap_ERRNO),
	};

	int ret = cmocka_run_group_tests(tests, NULL, NULL);

	disable_unistd_mocks();

	return ret;
}
