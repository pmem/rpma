/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mr-test.c -- the memory region registration/deregistration unit tests
 *
 * APIs covered:
 * - rpma_mr_reg()
 * - rpma_mr_dereg()
 */

#include <stdlib.h>
#include <infiniband/verbs.h>

#include "mr-test-common.h"

/* rpma_mr_reg() unit tests */

/*
 * test_mr_reg__NULL_peer -- NULL peer is invalid
 */
static void
test_mr_reg__NULL_peer(void **unused)
{
	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(NULL, MOCK_PTR, MOCK_SIZE,
				MOCK_USAGE, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(mr);
}

/*
 * test_mr_reg__NULL_ptr -- NULL ptr is invalid
 */
static void
test_mr_reg__NULL_ptr(void **unused)
{
	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, NULL, MOCK_SIZE,
				MOCK_USAGE, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(mr);
}

/*
 * test_mr_reg__NULL_mr -- NULL mr is invalid
 */
static void
test_mr_reg__NULL_mr(void **unused)
{
	/* run test */
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE,
				MOCK_USAGE, MOCK_PLT, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_mr_reg__NULL_peer_ptr_mr -- NULL peer, ptr and mr are invalid
 */
static void
test_mr_reg__NULL_peer_ptr_mr(void **unused)
{
	/* run test */
	int ret = rpma_mr_reg(NULL, NULL, MOCK_SIZE,
				MOCK_USAGE, MOCK_PLT, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_mr_reg__0_size -- size == 0 is invalid
 */
static void
test_mr_reg__0_size(void **unused)
{
	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, 0,
				MOCK_USAGE, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(mr);
}

/*
 * test_mr_reg__failed_E_NOMEM -- rpma_mr_reg fails with ENOMEM
 */
static void
test_mr_reg__failed_E_NOMEM(void **unused)
{
	/* configure mocks */
	struct ibv_mr ibv_mr = {0};
	will_return(__wrap__test_malloc, ENOMEM);
	will_return_maybe(rpma_peer_mr_reg, &ibv_mr);
	will_return_maybe(ibv_dereg_mr, MOCK_OK);

	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE,
				MOCK_USAGE, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(mr);
}

/*
 * test_mr_reg__failed_E_NOMEM -- rpma_mr_reg fails with ENOMEM
 */
static void
test_mr_reg__peer_mr_reg_failed_E_PROVIDER(void **unused)
{
	/* configure mocks */
	will_return(rpma_peer_mr_reg, NULL);
	will_return(rpma_peer_mr_reg, MOCK_ERRNO);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE,
				MOCK_USAGE, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(mr);
}

/*
 * test_mr_reg_dereg__success -- happy day sccenario
 */
static void
test_mr_reg_dereg__success(void **unused)
{
	/*
	 * The whole thing is done by setup__mr_reg_success()
	 * and teardown__mr_dereg_success().
	 */
}

/* rpma_mr_dereg() unit tests */

/*
 * test_mr_dereg__NULL_pptr -- NULL pptr is invalid
 */
static void
test_mr_dereg__NULL_pptr(void **unused)
{
	/* run test */
	int ret = rpma_mr_dereg(NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_mr_dereg__NULL_ptr -- NULL ptr is OK
 */
static void
test_mr_dereg__NULL_ptr(void **unused)
{
	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_dereg(&mr);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * test_mr_dereg__failed_E_PROVIDER -- rpma_mr_dereg failed
 * with RPMA_E_PROVIDER
 */
static void
test_mr_dereg__failed_E_PROVIDER(void **unused)
{
	struct rpma_mr_local *mr;

	/* create a local memory registration object */
	(void) setup__mr_reg_success((void **)&mr);

	/* configure mocks */
	will_return_maybe(ibv_dereg_mr, MOCK_ERRNO);

	/* run test */
	int ret = rpma_mr_dereg(&mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(mr);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_mr_reg() unit tests */
		cmocka_unit_test(test_mr_reg__NULL_peer),
		cmocka_unit_test(test_mr_reg__NULL_ptr),
		cmocka_unit_test(test_mr_reg__NULL_mr),
		cmocka_unit_test(test_mr_reg__NULL_peer_ptr_mr),
		cmocka_unit_test(test_mr_reg__0_size),
		cmocka_unit_test(test_mr_reg__failed_E_NOMEM),
		cmocka_unit_test(test_mr_reg__peer_mr_reg_failed_E_PROVIDER),
		cmocka_unit_test_setup_teardown(test_mr_reg_dereg__success,
			setup__mr_reg_success, teardown__mr_dereg_success),

		/* rpma_mr_dereg() unit tests */
		cmocka_unit_test(test_mr_dereg__NULL_pptr),
		cmocka_unit_test(test_mr_dereg__NULL_ptr),
		cmocka_unit_test(test_mr_dereg__failed_E_PROVIDER),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
