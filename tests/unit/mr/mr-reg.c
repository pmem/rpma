// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * mr-reg.c -- the memory region registration/deregistration unit tests
 *
 * APIs covered:
 * - rpma_mr_reg()
 * - rpma_mr_dereg()
 */

#include <stdlib.h>
#include <infiniband/verbs.h>

#include "mocks-ibverbs.h"
#include "mocks-rpma-peer.h"
#include "mr-common.h"
#include "test-common.h"

#define USAGE_WRONG	(~((int)0)) /* not allowed value of usage */

/* array of prestate structures */
static struct prestate prestates[] = {
	/* values used in reg_dereg__success called with (prestates + 0) */
	{RPMA_MR_USAGE_READ_SRC, IBV_ACCESS_REMOTE_READ, NULL},
	/* values used in reg_dereg__success called with (prestates + 1) */
	{(RPMA_MR_USAGE_READ_SRC | RPMA_MR_USAGE_READ_DST),
		(IBV_ACCESS_REMOTE_READ | IBV_ACCESS_LOCAL_WRITE), NULL},
};

/*
 * reg__NULL_peer -- NULL peer is invalid
 */
static void
reg__NULL_peer(void **unused)
{
	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(NULL, MOCK_PTR, MOCK_SIZE, MOCK_USAGE, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(mr);
}

/*
 * reg__NULL_ptr -- NULL ptr is invalid
 */
static void
reg__NULL_ptr(void **unused)
{
	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, NULL, MOCK_SIZE, MOCK_USAGE, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(mr);
}

/*
 * reg__NULL_mr_ptr -- NULL mr_ptr is invalid
 */
static void
reg__NULL_mr_ptr(void **unused)
{
	/* run test */
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE, MOCK_USAGE, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * reg__NULL_peer_ptr_mr_ptr -- NULL peer, ptr and mr_ptr are invalid
 */
static void
reg__NULL_peer_ptr_mr_ptr(void **unused)
{
	/* run test */
	int ret = rpma_mr_reg(NULL, NULL, MOCK_SIZE, MOCK_USAGE, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * reg__0_size -- size == 0 is invalid
 */
static void
reg__0_size(void **unused)
{
	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, 0, MOCK_USAGE, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(mr);
}

/*
 * reg__0_usage -- usage == 0 is invalid
 */
static void
reg__0_usage(void **unused)
{
	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE, 0, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(mr);
}

/*
 * reg__wrong_usage -- not allowed value of usage
 */
static void
reg__wrong_usage(void **unused)
{
	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE, USAGE_WRONG, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(mr);
}

/*
 * reg__malloc_ERRNO -- malloc() fails with MOCK_ERRNO
 */
static void
reg__malloc_ERRNO(void **unused)
{
	/* configure mocks */
	struct rpma_peer_setup_mr_reg_args mr_reg_args;
	mr_reg_args.usage = RPMA_MR_USAGE_READ_SRC;
	mr_reg_args.access = IBV_ACCESS_REMOTE_READ;
	mr_reg_args.mr = MOCK_MR;
	will_return(__wrap__test_malloc, MOCK_ERRNO);
	will_return_maybe(rpma_peer_setup_mr_reg, &mr_reg_args);
	will_return_maybe(ibv_dereg_mr, MOCK_OK);

	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE,
				mr_reg_args.usage, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(mr);
}

/*
 * reg__peer_mr_reg_ERRNO -- rpma_peer_setup_mr_reg() fails with MOCK_ERRNO
 */
static void
reg__peer_mr_reg_ERRNO(void **unused)
{
	/* configure mocks */
	struct rpma_peer_setup_mr_reg_args mr_reg_args;
	mr_reg_args.usage = RPMA_MR_USAGE_READ_DST;
	mr_reg_args.access = IBV_ACCESS_LOCAL_WRITE;
	mr_reg_args.mr = NULL;
	mr_reg_args.verrno = MOCK_ERRNO;
	will_return(rpma_peer_setup_mr_reg, &mr_reg_args);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE,
				mr_reg_args.usage, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(mr);
}

/*
 * reg_dereg__success -- happy day scenario
 */
static void
reg_dereg__success(void **unused)
{
	/*
	 * The whole thing is done by setup__reg_success()
	 * and teardown__dereg_success().
	 */
}

/* rpma_mr_dereg() unit tests */

/*
 * dereg__NULL_mr_ptr -- NULL mr_ptr is invalid
 */
static void
dereg__NULL_mr_ptr(void **unused)
{
	/* run test */
	int ret = rpma_mr_dereg(NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * dereg__NULL_mr -- NULL mr is OK
 */
static void
dereg__NULL_mr(void **unused)
{
	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_dereg(&mr);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * dereg__dereg_mr_ERRNO -- ibv_dereg_mr() fails with MOCK_ERRNO
 */
static void
dereg__dereg_mr_ERRNO(void **pprestate)
{
	/*
	 * Create a local memory registration object.
	 *
	 * It is a workaround for the following issue:
	 * https://gitlab.com/cmocka/cmocka/-/issues/47
	 */
	int ret = setup__reg_success(pprestate);
	assert_int_equal(ret, MOCK_OK);

	struct prestate *prestate = *pprestate;
	struct rpma_mr_local *mr = prestate->mr;

	/* configure mocks */
	will_return(ibv_dereg_mr, MOCK_ERRNO);

	/* run test */
	ret = rpma_mr_dereg(&mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(mr);
}

static const struct CMUnitTest tests_reg[] = {
	/* rpma_mr_reg() unit tests */
	cmocka_unit_test(reg__NULL_peer),
	cmocka_unit_test(reg__NULL_ptr),
	cmocka_unit_test(reg__NULL_mr_ptr),
	cmocka_unit_test(reg__NULL_peer_ptr_mr_ptr),
	cmocka_unit_test(reg__0_size),
	cmocka_unit_test(reg__0_usage),
	cmocka_unit_test(reg__wrong_usage),
	cmocka_unit_test(reg__malloc_ERRNO),
	cmocka_unit_test(reg__peer_mr_reg_ERRNO),
	cmocka_unit_test_prestate_setup_teardown(reg_dereg__success,
		setup__reg_success, teardown__dereg_success, prestates),

	/* rpma_mr_dereg() unit tests */
	cmocka_unit_test(dereg__NULL_mr_ptr),
	cmocka_unit_test(dereg__NULL_mr),
	cmocka_unit_test_prestate(dereg__dereg_mr_ERRNO,
		prestates + 1),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_reg, NULL, NULL);
}
