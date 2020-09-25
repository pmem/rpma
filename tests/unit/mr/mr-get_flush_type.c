// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mr-get_flush_type.c -- the memory region get_flush_type unit tests
 *
 * APIs covered:
 * - rpma_mr_remote_get_flush_type()
 */

#include <stdlib.h>
#include <infiniband/verbs.h>

#include "mocks-rpma-peer.h"
#include "mr-common.h"
#include "test-common.h"

/* rpma_mr_remote_get_flush_type() unit test */

/*
 * remote_get_flush_type__mr_NULL - NULL mr is invalid
 */
static void
remote_get_flush_type__mr_NULL(void **unused)
{
	/* run test */
	int flush_type = 0;
	int ret = rpma_mr_remote_get_flush_type(NULL, &flush_type);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_int_equal(flush_type, 0);
}

/*
 * remote_get_flush_type__flush_type_NULL - NULL flush_type pointer is invalid
 */
static void
remote_get_flush_type__flush_type_NULL(void **mr_ptr)
{
	struct rpma_mr_remote *mr = *mr_ptr;

	/* run test */
	int ret = rpma_mr_remote_get_flush_type(mr, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * remote_get_flush_type__mr_NULL_flush_type_NULL - NULL mr and NULL flush_type
 * pointer are invalid
 */
static void
remote_get_flush_type__mr_NULL_flush_type_NULL(void **unused)
{
	/* run test */
	int ret = rpma_mr_remote_get_flush_type(NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * remote_get_flush_type__success - rpma_mr_remote_get_flush_type() success
 */
static void
remote_get_flush_type__success(void **mr_ptr)
{
	struct rpma_mr_remote *mr = *mr_ptr;

	/* verify the remote memory region correctness */
	int flush_type = 0;
	int ret = rpma_mr_remote_get_flush_type(mr, &flush_type);
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(flush_type, MOCK_FLUSH_TYPE);
}

static const struct CMUnitTest tests_get_flush_type[] = {
	/* rpma_mr_remote_get_flush_type() unit test */
	cmocka_unit_test(remote_get_flush_type__mr_NULL),
	cmocka_unit_test_setup_teardown(remote_get_flush_type__flush_type_NULL,
		setup__mr_remote, teardown__mr_remote),
	cmocka_unit_test(remote_get_flush_type__mr_NULL_flush_type_NULL),
	cmocka_unit_test_setup_teardown(remote_get_flush_type__success,
		setup__mr_remote, teardown__mr_remote),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_get_flush_type, NULL, NULL);
}
