/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mr-test-common.c -- the memory region unit tests's common functions
 */

#include "mr-test-common.h"

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
rpma_peer_mr_reg(struct rpma_peer *peer, struct ibv_mr **ibv_mr_ptr, void *addr,
	size_t length, int access)
{
	/*
	 * rpma_peer_mr_reg() and malloc() may be called in any order.
	 * If the first one fails, then the second one won't be called,
	 * so we cannot add cmocka's expects here.
	 * Otherwise, unconsumed expects would cause a test failure.
	 */
	struct ibv_mr *ibv_mr = mock_type(struct ibv_mr *);
	if (ibv_mr == NULL) {
		Rpma_provider_error = mock_type(int);
		return RPMA_E_PROVIDER;
	}

	ibv_mr->addr = addr;
	ibv_mr->length = length;
	ibv_mr->rkey = MOCK_RKEY;
	*ibv_mr_ptr = ibv_mr;

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
	return mock_type(int); /* errno */
}

/* common setups & teardowns */

/*
 * setup__mr_reg_success -- create a local memory registration object
 */
int
setup__mr_reg_success(void **mr_ptr)
{
	/* configure mocks */
	static struct ibv_mr ibv_mr = {0};
	will_return(__wrap__test_malloc, MOCK_OK);
	will_return(rpma_peer_mr_reg, &ibv_mr);

	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE,
				MOCK_USAGE, MOCK_PLT, &mr);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(mr);

	*mr_ptr = mr;

	return 0;
}

/*
 * teardown__mr_dereg_success -- delete a local memory
 * registration object
 */
int
teardown__mr_dereg_success(void **mr_ptr)
{
	struct rpma_mr_local *mr = *mr_ptr;

	/* configure mocks */
	will_return_maybe(ibv_dereg_mr, MOCK_OK);

	int ret = rpma_mr_dereg(&mr);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_null(mr);

	*mr_ptr = NULL;

	return 0;
}
