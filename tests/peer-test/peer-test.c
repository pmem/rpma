/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * peer-test.c -- a peer unit test
 */

#include "cmocka_headers.h"

#include "librpma.h"

#include <infiniband/verbs.h>

#define MOCK_IBV_CTX	(struct ibv_context *)0xC0
#define MOCK_PD			0xD0

#define NOT_NULLED_PTR	(void *)0x0A

#define MOCK_USE_REAL 0

/*
 * __wrap_ibv_alloc_pd -- ibv_alloc_pd() mock
 */
static struct ibv_pd *
__wrap_ibv_alloc_pd(struct ibv_context *ibv_ctx)
{
	check_expected_ptr(ibv_ctx);

	struct ibv_pd *pd = mock_type(struct ibv_pd *);

	if (pd != NULL)
		return pd;

	/*
	 * The ibv_alloc_pd(3) manual page does not document that this function
	 * returns any error via errno but seemingly it is. For the usability
	 * sake, in librpma we try to deduce what really happened using the
	 * errno value.
	 */
	errno = mock_type(int);

	return NULL;
}

/*
 * __wrap_ibv_dealloc_pd -- ibv_dealloc_pd() mock
 */
static int
__wrap_ibv_dealloc_pd(struct ibv_pd *pd)
{
	check_expected_ptr(pd);

	return mock_type(int);
}

/*
 * __wrap_malloc -- malloc() mock
 */
static void *
__wrap_malloc(size_t size)
{
	int order_or_error = mock_type(int);
	if (order_or_error == MOCK_USE_REAL)
		return __real_malloc(size);

	errno = order_or_error;

	return NULL;
}

/*
 * peer_new_test_invalid_ibv_ctx -- NULL ibv_ctx is not valid
 */
static void
peer_new_test_invalid_ibv_ctx(void **unused)
{
	/*
	 * configure mocks:
	 * - NOTE: it is not allowed for peer to pass NULL'ed ibv_ctx into
	 * ibv_alloc_pd(). So ibv_dealloc_pd() also should not be used.
	 */
	will_return_maybe(__wrap_malloc, MOCK_USE_REAL);

	/* run test */
	struct rpma_peer *peer = NOT_NULLED_PTR;
	int ret = rpma_peer_new(NULL, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_ptr_equal(peer, NULL);
}

/*
 * peer_new_test_invalid_peer_ptr -- NULL **peer is not valid
 */
static void
peer_new_test_invalid_peer_ptr(void **unused)
{
	/* configure mocks */
	expect_value(__wrap_ibv_alloc_pd, ibv_ctx, MOCK_IBV_CTX);
	will_return_maybe(__wrap_ibv_alloc_pd, MOCK_PD);
	expect_value(__wrap_ibv_dealloc_pd, pd, MOCK_PD);
	will_return_maybe(__wrap_ibv_dealloc_pd, 0);
	will_return_maybe(__wrap_malloc, MOCK_USE_REAL);

	/* run test */
	int ret = rpma_peer_new(MOCK_IBV_CTX, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * peer_new_test_alloc_pd_fail_ENOMEM -- ibv_alloc_pd() fails with ENOMEM
 */
static void
peer_new_test_alloc_pd_fail_ENOMEM(void **unused)
{
	/*
	 * configure mocks:
	 * - NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd()
	 * has failed.
	 */
	expect_value(__wrap_ibv_alloc_pd, ibv_ctx, MOCK_IBV_CTX);
	will_return(__wrap_ibv_alloc_pd, NULL);
	will_return(__wrap_ibv_alloc_pd, ENOMEM);
	will_return_maybe(__wrap_malloc, MOCK_USE_REAL);

	/* run test */
	struct rpma_peer *peer = NOT_NULLED_PTR;
	int ret = rpma_peer_new(MOCK_IBV_CTX, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_ptr_equal(peer, NULL);
}

/*
 * peer_new_test_alloc_pd_fail_not_ENOMEM -- ibv_alloc_pd() fails with not ENOMEM
 */
static void
peer_new_test_alloc_pd_fail_not_ENOMEM(void **unused)
{
	/*
	 * configure mocks:
	 * - NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd()
	 * has failed.
	 */
	int arbitrary_error = EAGAIN; /* which is != ENOMEM */
	expect_value(__wrap_ibv_alloc_pd, ibv_ctx, MOCK_IBV_CTX);
	will_return(__wrap_ibv_alloc_pd, NULL);
	will_return(__wrap_ibv_alloc_pd, arbitrary_error);
	will_return_maybe(__wrap_malloc, MOCK_USE_REAL);

	/* run test */
	struct rpma_peer *peer = NOT_NULLED_PTR;
	int ret = rpma_peer_new(MOCK_IBV_CTX, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), arbitrary_error);
}

/*
 * peer_new_test_alloc_pd_fail_no_error -- ibv_alloc_pd() fails without error
 */
static void
peer_new_test_alloc_pd_fail_no_error(void **unused)
{
	/*
	 * configure mocks:
	 * - NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd()
	 * has failed.
	 */
	int no_error = 0;
	expect_value(__wrap_ibv_alloc_pd, ibv_ctx, MOCK_IBV_CTX);
	will_return(__wrap_ibv_alloc_pd, NULL);
	will_return(__wrap_ibv_alloc_pd, no_error);
	will_return_maybe(__wrap_malloc, MOCK_USE_REAL);

	/* run test */
	struct rpma_peer *peer = NOT_NULLED_PTR;
	int ret = rpma_peer_new(MOCK_IBV_CTX, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_UNKNOWN);
	assert_int_equal(rpma_err_get_provider_error(), no_error);
}

/*
 * peer_new_test_malloc_fail -- malloc() fail
 */
static void
peer_new_test_malloc_fail(void **unused)
{
	/* configure mocks */
	expect_value(__wrap_ibv_alloc_pd, ibv_ctx, MOCK_IBV_CTX);
	will_return_maybe(__wrap_ibv_alloc_pd, MOCK_PD);
	expect_value(__wrap_ibv_dealloc_pd, pd, MOCK_PD);
	will_return_maybe(__wrap_ibv_dealloc_pd, 0);
	will_return_maybe(__wrap_malloc, ENOMEM);

	/* run test */
	struct rpma_peer *peer = NOT_NULLED_PTR;
	int ret = rpma_peer_new(MOCK_IBV_CTX, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_ptr_equal(peer, NULL);
}

/*
 * peer_new_test_success -- happy day scenario
 */
static void
peer_new_test_success(void **unused)
{
	/*
	 * configure mocks for rpma_peer_new():
	 * NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd() succeeded.
	 */
	expect_value(__wrap_ibv_alloc_pd, ibv_ctx, MOCK_IBV_CTX);
	will_return_maybe(__wrap_ibv_alloc_pd, MOCK_PD);
	will_return(__wrap_malloc, MOCK_USE_REAL);

	/* run test - step 1 */
	struct rpma_peer *peer = NOT_NULLED_PTR;
	int ret = rpma_peer_new(MOCK_IBV_CTX, &peer);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_non_null(peer);
	assert_ptr_not_equal(peer, NOT_NULLED_PTR);

	/*
	 * configure mocks for rpma_peer_delete():
	 * NOTE: it is not allowed to call ibv_alloc_pd() nor malloc() in
	 * rpma_peer_delete().
	 */
	expect_value(__wrap_ibv_dealloc_pd, pd, MOCK_PD);
	will_return(__wrap_ibv_dealloc_pd, 0);

	/* run test - step 2 */
	ret = rpma_peer_delete(&peer);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_null(peer);
}

/*
 * peer_delete_test_invalid_peer_ptr -- NULL **peer is not valid
 */
static void
peer_delete_test_invalid_peer_ptr(void **unused)
{
	/* run test */
	int ret = rpma_peer_delete(NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * peer_delete_test_null_peer -- valid NULL *peer
 */
static void
peer_delete_test_null_peer(void **unused)
{
	/* run test */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_delete(&peer);

	/* verify the result */
	assert_int_equal(ret, 0);
	assert_null(peer);
}

/*
 * peer_delete_test_dealloc_pd_fail -- ibv_dealloc_pd() fail
 */
static void
peer_delete_test_dealloc_pd_fail(void **unused)
{
	/*
	 * configure mocks for rpma_peer_new():
	 * NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd() succeeded.
	 */
	expect_value(__wrap_ibv_alloc_pd, ibv_ctx, MOCK_IBV_CTX);
	will_return_maybe(__wrap_ibv_alloc_pd, MOCK_PD);
	will_return(__wrap_malloc, MOCK_USE_REAL);

	/* setup */
	struct rpma_peer *peer = NOT_NULLED_PTR;
	int ret = rpma_peer_new(MOCK_IBV_CTX, &peer);
	assert_int_equal(ret, 0);
	assert_non_null(peer);
	assert_ptr_not_equal(peer, NOT_NULLED_PTR);

	/*
	 * configure mocks for rpma_peer_delete():
	 * NOTE: it is not allowed to call ibv_alloc_pd() nor malloc() in
	 * rpma_peer_delete().
	 */
	int arbitrary_error = EAGAIN;
	expect_value(__wrap_ibv_dealloc_pd, pd, MOCK_PD);
	will_return(__wrap_ibv_dealloc_pd, arbitrary_error);

	/* run test */
	ret = rpma_peer_delete(&peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), arbitrary_error);

	/*
	 * configure mocks for rpma_peer_delete():
	 * NOTE: it is not allowed to call ibv_alloc_pd() nor malloc() in
	 * rpma_peer_delete().
	 */
	expect_value(__wrap_ibv_dealloc_pd, pd, MOCK_PD);
	will_return(__wrap_ibv_dealloc_pd, 0);

	/* teardown */
	ret = rpma_peer_delete(&peer);
	assert_int_equal(ret, 0);
	assert_null(peer);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_peer_new() unit tests */
		cmocka_unit_test(peer_new_test_invalid_ibv_ctx),
		cmocka_unit_test(peer_new_test_invalid_peer_ptr),
		cmocka_unit_test(peer_new_test_alloc_pd_fail_ENOMEM),
		cmocka_unit_test(peer_new_test_alloc_pd_fail_not_ENOMEM),
		cmocka_unit_test(peer_new_test_alloc_pd_fail_no_error),
		cmocka_unit_test(peer_new_test_malloc_fail),
		cmocka_unit_test(peer_new_test_success),

		/* rpma_peer_delete() unit tests */
		cmocka_unit_test(peer_delete_test_invalid_peer_ptr),
		cmocka_unit_test(peer_delete_test_null_peer),
		cmocka_unit_test(peer_delete_test_dealloc_pd_fail),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
