/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mr-test.c -- the memory region unit tests
 */

#include <stdlib.h>
#include <infiniband/verbs.h>

#include "cmocka_headers.h"
#include "mr.h"
#include "librpma.h"
#include "rpma_err.h"

#define MOCK_PEER	(struct rpma_peer *)0x0AD0
#define MOCK_PTR	(void *)0x0AD1
#define MOCK_SIZE	(size_t)0x0AD2
#define MOCK_PLT	(enum rpma_mr_plt)0x0AD4
#define MOCK_MR		(struct ibv_mr *)0x0AD5
#define MOCK_ERRNO	(int)(123)
#define MOCK_OK		(int)0
#define MOCK_USAGE	(RPMA_MR_USAGE_READ_SRC | RPMA_MR_USAGE_READ_DST)
#define USAGE_WRONG	(~((int)0)) /* not allowed value of usage */

/* prestate structure passed to unit test functions */
struct prestate {
	int usage;
	int access;
	/* mr passed from setup to test and to teardown */
	struct rpma_mr_local *mr;
};

/* array of prestate structures */
static struct prestate prestates[] = {
	/* values used in test_reg_dereg__success called with (prestates + 0) */
	{RPMA_MR_USAGE_READ_SRC, IBV_ACCESS_REMOTE_READ, NULL},
	/* values used in test_reg_dereg__success called with (prestates + 1) */
	{RPMA_MR_USAGE_READ_DST, IBV_ACCESS_LOCAL_WRITE, NULL},
	/* values used in test_reg_dereg__success called with (prestates + 2) */
	{(RPMA_MR_USAGE_READ_SRC | RPMA_MR_USAGE_READ_DST),
		(IBV_ACCESS_REMOTE_READ | IBV_ACCESS_LOCAL_WRITE), NULL},
};

/* structure of arguments used in rpma_peer_mr_reg() */
struct rpma_peer_mr_reg_args {
	int usage;
	int access;
	struct ibv_mr *mr;
	int verrno;
};

/* mocks */

/*
 * rpma_peer_mr_reg -- a mock of rpma_peer_mr_reg()
 */
int
rpma_peer_mr_reg(struct rpma_peer *peer, struct ibv_mr **ibv_mr, void *addr,
	size_t length, int access)
{
	/*
	 * rpma_peer_mr_reg() and malloc() may be called in any order.
	 * If the first one fails, then the second one won't be called,
	 * so we cannot add cmocka's expects here.
	 * Otherwise, unconsumed expects would cause a test failure.
	 */
	struct rpma_peer_mr_reg_args *args =
				mock_type(struct rpma_peer_mr_reg_args *);
	assert_ptr_equal(peer, MOCK_PEER);
	assert_ptr_equal(addr, MOCK_PTR);
	assert_int_equal(length, MOCK_SIZE);
	assert_int_equal(access, args->access);

	*ibv_mr = args->mr;
	if (*ibv_mr == NULL) {
		Rpma_provider_error = args->verrno;
		return RPMA_E_PROVIDER;
	}

	return 0;
}

/*
 * ibv_dereg_mr -- a mock of ibv_dereg_mr()
 */
int
ibv_dereg_mr(struct ibv_mr *mr)
{
	/*
	 * rpma_peer_mr_reg() and malloc() may be called in any order.
	 * If the first one fails, then the second one won't be called.
	 * ibv_dereg_mr() will be called in rpma_mr_reg() only if:
	 * 1) rpma_peer_mr_reg() succeeded and
	 * 2) malloc() failed.
	 * In the opposite case, when:
	 * 1) malloc() succeeded and
	 * 2) rpma_peer_mr_reg() failed,
	 * ibv_dereg_mr() will not be called,
	 * so we cannot add cmocka's expects here.
	 * Otherwise, unconsumed expects would cause a test failure.
	 */
	assert_int_equal(mr, MOCK_MR);

	return mock_type(int); /* errno */
}

void *__real__test_malloc(size_t size);

/*
 * __wrap__test_malloc -- malloc() mock
 */
void *
__wrap__test_malloc(size_t size)
{
	errno = mock_type(int);

	if (errno)
		return NULL;

	return __real__test_malloc(size);
}

/* unit tests */

/*
 * test_reg__NULL_peer -- NULL peer is invalid
 */
static void
test_reg__NULL_peer(void **unused)
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
 * test_reg__NULL_ptr -- NULL ptr is invalid
 */
static void
test_reg__NULL_ptr(void **unused)
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
 * test_reg__NULL_mr_ptr -- NULL mr_ptr is invalid
 */
static void
test_reg__NULL_mr_ptr(void **unused)
{
	/* run test */
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE,
				MOCK_USAGE, MOCK_PLT, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_reg__NULL_peer_ptr_mr_ptr -- NULL peer, ptr and mr_ptr are invalid
 */
static void
test_reg__NULL_peer_ptr_mr_ptr(void **unused)
{
	/* run test */
	int ret = rpma_mr_reg(NULL, NULL, MOCK_SIZE,
				MOCK_USAGE, MOCK_PLT, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_reg__0_size -- size == 0 is invalid
 */
static void
test_reg__0_size(void **unused)
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
 * test_reg__0_usage -- usage == 0 is invalid
 */
static void
test_reg__0_usage(void **unused)
{
	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE,
				0, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(mr);
}

/*
 * test_reg__wrong_usage -- not allowed value of usage
 */
static void
test_reg__wrong_usage(void **unused)
{
	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE,
				USAGE_WRONG, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(mr);
}

/*
 * test_reg__failed_E_NOMEM -- rpma_mr_reg fails with ENOMEM
 */
static void
test_reg__failed_E_NOMEM(void **unused)
{
	/* configure mocks */
	struct rpma_peer_mr_reg_args mr_reg_args;
	mr_reg_args.usage = RPMA_MR_USAGE_READ_SRC;
	mr_reg_args.access = IBV_ACCESS_REMOTE_READ;
	mr_reg_args.mr = MOCK_MR;
	will_return(__wrap__test_malloc, ENOMEM);
	will_return_maybe(rpma_peer_mr_reg, &mr_reg_args);
	will_return_maybe(ibv_dereg_mr, MOCK_OK);

	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE,
				mr_reg_args.usage, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(mr);
}

/*
 * test_reg__peer_mr_reg_failed_E_PROVIDER -- rpma_peer_mr_reg fails with ENOMEM
 */
static void
test_reg__peer_mr_reg_failed_E_PROVIDER(void **unused)
{
	/* configure mocks */
	struct rpma_peer_mr_reg_args mr_reg_args;
	mr_reg_args.usage = RPMA_MR_USAGE_READ_DST;
	mr_reg_args.access = IBV_ACCESS_LOCAL_WRITE;
	mr_reg_args.mr = NULL;
	mr_reg_args.verrno = MOCK_ERRNO;
	will_return_maybe(rpma_peer_mr_reg, &mr_reg_args);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE,
				mr_reg_args.usage, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), MOCK_ERRNO);
	assert_null(mr);
}

/*
 * setup__reg_success -- create a local memory registration object
 */
static int
setup__reg_success(void **pprestate)
{
	struct prestate *prestate = *pprestate;

	/* configure mocks */
	struct rpma_peer_mr_reg_args mr_reg_args;
	mr_reg_args.usage = prestate->usage;
	mr_reg_args.access = prestate->access;
	mr_reg_args.mr = MOCK_MR;
	will_return_maybe(rpma_peer_mr_reg, &mr_reg_args);
	will_return(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE,
				mr_reg_args.usage, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(mr);

	/* pass mr to the test */
	prestate->mr = mr;

	return 0;
}

/*
 * teardown__dereg_success -- delete the local memory registration object
 */
static int
teardown__dereg_success(void **pprestate)
{
	struct prestate *prestate = *pprestate;
	struct rpma_mr_local *mr = prestate->mr;

	/* configure mocks */
	will_return(ibv_dereg_mr, MOCK_OK);

	int ret = rpma_mr_dereg(&mr);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_null(mr);

	*pprestate = NULL;

	return 0;
}

/*
 * test_reg_dereg__success -- happy day scenario
 */
static void
test_reg_dereg__success(void **unused)
{
	/*
	 * The whole thing is done by setup__reg_success()
	 * and teardown__dereg_success().
	 */
}

/*
 * test_dereg__NULL_mr_ptr -- NULL mr_ptr is invalid
 */
static void
test_dereg__NULL_mr_ptr(void **unused)
{
	/* run test */
	int ret = rpma_mr_dereg(NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_dereg__NULL_mr -- NULL mr is OK
 */
static void
test_dereg__NULL_mr(void **unused)
{
	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_dereg(&mr);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * test_dereg__failed_E_PROVIDER -- rpma_mr_dereg failed with RPMA_E_PROVIDER
 */
static void
test_dereg__failed_E_PROVIDER(void **pprestate)
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
	will_return_maybe(ibv_dereg_mr, MOCK_ERRNO);

	/* run test */
	ret = rpma_mr_dereg(&mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), MOCK_ERRNO);
	assert_null(mr);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_mr_reg() unit tests */
		cmocka_unit_test(test_reg__NULL_peer),
		cmocka_unit_test(test_reg__NULL_ptr),
		cmocka_unit_test(test_reg__NULL_mr_ptr),
		cmocka_unit_test(test_reg__NULL_peer_ptr_mr_ptr),
		cmocka_unit_test(test_reg__0_size),
		cmocka_unit_test(test_reg__0_usage),
		cmocka_unit_test(test_reg__wrong_usage),
		cmocka_unit_test(test_reg__failed_E_NOMEM),
		cmocka_unit_test(test_reg__peer_mr_reg_failed_E_PROVIDER),
		cmocka_unit_test_prestate_setup_teardown(
			test_reg_dereg__success,
			setup__reg_success, teardown__dereg_success,
			prestates + 0),
		cmocka_unit_test_prestate_setup_teardown(
			test_reg_dereg__success,
			setup__reg_success, teardown__dereg_success,
			prestates + 1),
		cmocka_unit_test_prestate_setup_teardown(
			test_reg_dereg__success,
			setup__reg_success, teardown__dereg_success,
			prestates + 2),

		/* rpma_mr_dereg() unit tests */
		cmocka_unit_test(test_dereg__NULL_mr_ptr),
		cmocka_unit_test(test_dereg__NULL_mr),
		cmocka_unit_test_prestate(test_dereg__failed_E_PROVIDER,
			prestates + 2),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
