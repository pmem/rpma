/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * flush.c -- unit tests of the flush module
 */

#include <stdlib.h>

#include "cmocka_headers.h"
#include "flush.h"
#include "librpma.h"
#include "test-common.h"

void *__real__test_malloc(size_t size);

/*
 * __wrap__test_malloc -- malloc() mock
 */
void *
__wrap__test_malloc(size_t size)
{
	int err = mock_type(int);

	if (err) {
		errno = err;
		return NULL;
	}

	return __real__test_malloc(size);
}

struct posix_memalign_args {
	void *ptr;
};

/*
 * __wrap_posix_memalign -- posix_memalign() mock
 */
int
__wrap_posix_memalign(void **memptr, size_t alignment, size_t size)
{
	int err = mock_type(int);
	if (err) {
		errno = err;
		return errno;
	}

	struct posix_memalign_args *args =
		mock_type(struct posix_memalign_args *);

	*memptr = __real__test_malloc(size);

	/* save the address of the allocated memory to verify it later */
	args->ptr = *memptr;

	return 0;
}

#define PAGESIZE	4096

/*
 * sysconf -- sysconf() mock
 */
long
sysconf(int name)
{
	int err = mock_type(int);
	if (err) {
		errno = err;
		return -1;
	}
	return PAGESIZE;
}

/*
 * all the resources used between ep_setup and ep_teardown
 */
struct flush_test_state {
	struct rpma_flush *flush;
};

/*
 * flush_setup - prepare a valid rpma_flush object
 */
static int
flush_setup(void **fstate_ptr)
{
	/* configure mocks */
	static struct flush_test_state fstate = {0};

	will_return_always(__wrap__test_malloc, MOCK_OK);

	will_return(sysconf, MOCK_OK);

	will_return(__wrap_posix_memalign, MOCK_OK);
	struct posix_memalign_args allocated_raw = {0};
	will_return(__wrap_posix_memalign, &allocated_raw);

	struct rpma_mr_local *raw_mr;
	expect_value(rpma_mr_reg, peer, MOCK_PEER);
	expect_value(rpma_mr_reg, size, 8);
	expect_value(rpma_mr_reg, usage, RPMA_MR_USAGE_READ_DST);
	will_return(rpma_mr_reg, &allocated_raw.ptr);
	will_return(rpma_mr_reg, raw_mr);

	/* run test */
	int ret = rpma_flush_new(MOCK_PEER, &fstate.flush);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_non_null(fstate.flush);

	*fstate_ptr = &fstate;
	return 0;
}

/*
 * flush_teardown - delete the rpma_flush object
 */
static int
flush_teardown(void **fstate_ptr)
{
	struct flush_test_state *fstate = *fstate_ptr;

	/* delete the object */

	int ret = rpma_flush_delete(&fstate->flush);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	return 0;
}

/*
 * flush_new__malloc_ENOMEM -- malloc() fail with ENOMEM
 */
static void
flush_new__malloc_ENOMEM(void **unused)
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
 * flush_new__apm_sysconf_EINVAL -- sysconf() fail with EINVAL
 */
static void
flush_new__apm_sysconf_EINVAL(void **unused)
{
	/* configure mocks */
	will_return_always(__wrap__test_malloc, MOCK_OK);
	will_return(sysconf, EINVAL);

	/* run test */
	struct rpma_flush *flush = NULL;
	int ret = rpma_flush_new(MOCK_PEER, &flush);

	/* verify the results */
	assert_int_equal(errno, EINVAL);
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(flush);
}

/*
 * flush_new__apm_posix_memalign_ENOMEM -- malloc() fail with ENOMEM
 */
static void
flush_new__apm_posix_memalign_ENOMEM(void **unused)
{
	/* configure mocks */
	will_return_always(__wrap__test_malloc, MOCK_OK);
	will_return(sysconf, MOCK_OK);
	will_return(__wrap_posix_memalign, ENOMEM);

	/* run test */
	struct rpma_flush *flush = NULL;
	int ret = rpma_flush_new(MOCK_PEER, &flush);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(flush);
}

/*
 * flush_new__apm_mr_reg_ENOMEM -- malloc() fail with ENOMEM
 */
static void
flush_new__apm_mr_reg_RPMA_E_NOMEM(void **unused)
{
	/* configure mocks */
	will_return_always(__wrap__test_malloc, MOCK_OK);
	will_return(sysconf, MOCK_OK);

	will_return(__wrap_posix_memalign, MOCK_OK);
	struct posix_memalign_args allocated_raw = {0};
	will_return(__wrap_posix_memalign, &allocated_raw);
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
 * flush_new__apm_success -- happy day scenario
 */
static void
flush_new__apm_success(void **unused)
{
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma__new() unit tests */
		cmocka_unit_test(flush_new__malloc_ENOMEM),
		cmocka_unit_test(flush_new__apm_sysconf_EINVAL),
		cmocka_unit_test(flush_new__apm_posix_memalign_ENOMEM),
		cmocka_unit_test(flush_new__apm_mr_reg_RPMA_E_NOMEM),
		cmocka_unit_test_setup_teardown(flush_new__apm_success,
			flush_setup, flush_teardown),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
