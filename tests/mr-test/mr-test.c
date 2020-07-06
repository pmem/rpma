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

#define MOCK_USAGE_1	(int)(RPMA_MR_USAGE_READ_SRC)
#define MOCK_USAGE_2	(int)(RPMA_MR_USAGE_READ_DST)
#define MOCK_USAGE_3	(int)(RPMA_MR_USAGE_READ_SRC | RPMA_MR_USAGE_READ_DST)
#define MOCK_USAGE	MOCK_USAGE_3 /* the default value of usage */
#define USAGE_WRONG	(~((int)0)) /* not allowed value of usage */

#define MOCK_ACCESS_1	(int)(IBV_ACCESS_REMOTE_READ)
#define MOCK_ACCESS_2	(int)(IBV_ACCESS_LOCAL_WRITE)
#define MOCK_ACCESS_3	(int)(IBV_ACCESS_REMOTE_READ | IBV_ACCESS_LOCAL_WRITE)
#define MOCK_ACCESS	MOCK_ACCESS_3 /* the default value of access */

/* state structure passed to unit test functions */
struct state {
	unsigned index;
	int usage;
	int access;
};

/* array of state structures */
static struct state states[] = {
	/* values used in test_reg__peer_mr_reg_failed_E_PROVIDER */
	/* called with (states + 0) */
	{0, MOCK_USAGE, MOCK_ACCESS},
	/* values used in test_reg_dereg__success called with (states + 1) */
	{1, MOCK_USAGE_1, MOCK_ACCESS_1},
	/* values used in test_reg_dereg__success called with (states + 2) */
	{2, MOCK_USAGE_2, MOCK_ACCESS_2},
	/* values used in test_reg_dereg__success called with (states + 3) */
	{3, MOCK_USAGE_3, MOCK_ACCESS_3},
	/* values used in test_dereg__failed_E_PROVIDER */
	/* called with (states + 4) */
	{4, MOCK_USAGE, MOCK_ACCESS},
};

static unsigned number_of_states = sizeof(states) / sizeof(states[0]);

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
	assert_ptr_equal(peer, MOCK_PEER);
	assert_ptr_equal(addr, MOCK_PTR);
	/* length = MOCK_SIZE + (index of states[]) */
	assert_in_range(length - MOCK_SIZE, 0, number_of_states - 1);
	assert_int_equal(access, states[length - MOCK_SIZE].access);

	*ibv_mr = mock_type(struct ibv_mr *);
	if (*ibv_mr == NULL) {
		Rpma_provider_error = mock_type(int);
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
test_reg__failed_E_NOMEM(void **pstate)
{
	/* set value of usage */
	struct state *state = *pstate;
	int usage = state->usage;

	/* configure mocks */
	will_return(__wrap__test_malloc, ENOMEM);
	will_return_maybe(rpma_peer_mr_reg, MOCK_MR);
	will_return_maybe(ibv_dereg_mr, MOCK_OK);

	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE + state->index,
				usage, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(mr);
}

/*
 * test_reg__peer_mr_reg_failed_E_PROVIDER -- rpma_peer_mr_reg fails with ENOMEM
 */
static void
test_reg__peer_mr_reg_failed_E_PROVIDER(void **pstate)
{
	/* set value of usage */
	struct state *state = *pstate;
	int usage = state->usage;

	/* configure mocks */
	will_return(rpma_peer_mr_reg, NULL);
	will_return(rpma_peer_mr_reg, MOCK_ERRNO);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE + state->index,
				usage, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), MOCK_ERRNO);
	assert_null(mr);
}

/*
 * setup__reg_success -- create a local memory registration object
 */
static int
setup__reg_success(void **pstate)
{
	/* set value of usage */
	struct state *state = *pstate;
	int usage = state->usage;

	/* configure mocks */
	will_return(__wrap__test_malloc, MOCK_OK);
	will_return(rpma_peer_mr_reg, MOCK_MR);

	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE + state->index,
				usage, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(mr);

	*pstate = mr;

	return 0;
}

/*
 * teardown__dereg_success -- delete a local memory registration object
 */
static int
teardown__dereg_success(void **mr_ptr)
{
	struct rpma_mr_local *mr = *mr_ptr;

	/* configure mocks */
	will_return(ibv_dereg_mr, MOCK_OK);

	int ret = rpma_mr_dereg(&mr);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_null(mr);

	*mr_ptr = NULL;

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
test_dereg__failed_E_PROVIDER(void **pstate)
{
	/*
	 * Create a local memory registration object.
	 *
	 * It is a workaround for the following issue:
	 * https://gitlab.com/cmocka/cmocka/-/issues/47
	 */
	int ret = setup__reg_success(pstate);
	assert_int_equal(ret, MOCK_OK);
	struct rpma_mr_local *mr = *pstate;

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
		cmocka_unit_test_prestate(test_reg__failed_E_NOMEM,
			states + 0),
		cmocka_unit_test_prestate(
			test_reg__peer_mr_reg_failed_E_PROVIDER,
			states + 0),
		cmocka_unit_test_prestate_setup_teardown(
			test_reg_dereg__success,
			setup__reg_success, teardown__dereg_success,
			states + 1),
		cmocka_unit_test_prestate_setup_teardown(
			test_reg_dereg__success,
			setup__reg_success, teardown__dereg_success,
			states + 2),
		cmocka_unit_test_prestate_setup_teardown(
			test_reg_dereg__success,
			setup__reg_success, teardown__dereg_success,
			states + 3),

		/* rpma_mr_dereg() unit tests */
		cmocka_unit_test(test_dereg__NULL_mr_ptr),
		cmocka_unit_test(test_dereg__NULL_mr),
		cmocka_unit_test_prestate(test_dereg__failed_E_PROVIDER,
			states + 4),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
