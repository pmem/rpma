/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mr-test-common.c -- the memory region unit tests's common functions
 */

#include "mr-test-common.h"

struct ibv_mr Ibv_mr = {0};

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

	(*ibv_mr)->addr = addr;
	(*ibv_mr)->length = length;
	(*ibv_mr)->rkey = MOCK_RKEY;

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

/* common setups & teardowns */

/*
 * setup__reg_success -- create a local memory registration object
 */
int
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
int
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

	return 0;
}
