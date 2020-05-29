/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * peer-test.c -- a peer unit test
 */

#include "cmocka_headers.h"

#include "fault_injection.h"
#include "librpma.h"

#include <infiniband/verbs.h>

#define MOCK_IBV_CTX			(struct ibv_context *)0x00C0
#define MOCK_IBV_PD				(struct ibv_pd *)0x00D0

#define MOCK_VALIDATE_PARAMS	0x0100

#define MOCK_VALUE_MASK			0x00FF
#define MOCK_CTRL_MASK			0xFF00

#define NOT_NULLED_PTR	(void *)0x0A
#define NULL_VALUE				0

/*
 * ibv_alloc_pd -- ibv_alloc_pd() mock
 */
struct ibv_pd *
ibv_alloc_pd(struct ibv_context *ibv_ctx)
{
	uint64_t tmp = mock_type(uint64_t);
	int ctrl = tmp & MOCK_CTRL_MASK;
	if (ctrl & MOCK_VALIDATE_PARAMS)
		check_expected_ptr(ibv_ctx);

	struct ibv_pd *pd = (struct ibv_pd *)(tmp & MOCK_VALUE_MASK);

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
 * ibv_dealloc_pd -- ibv_dealloc_pd() mock
 */
int
ibv_dealloc_pd(struct ibv_pd *pd)
{
	int tmp = mock_type(int);
	int ctrl = tmp & MOCK_CTRL_MASK;
	if (ctrl & MOCK_VALIDATE_PARAMS)
		check_expected_ptr(pd);

	return tmp & MOCK_VALUE_MASK;
}

void *__real_malloc(size_t size);

/*
 * __wrap_malloc -- malloc() mock
 */
void *
__wrap_malloc(size_t size)
{
	int err = mock_type(int);

	if (err) {
		errno = err;
		return NULL;
	}

	return __real_malloc(size);
}

/*
 * peer_new_test_ibv_ctx_eq_NULL -- NULL ibv_ctx is not valid
 */
static void
peer_new_test_ibv_ctx_eq_NULL(void **unused)
{
	/*
	 * NOTE: it is not allowed for peer to pass NULL'ed ibv_ctx into
	 * ibv_alloc_pd(). So ibv_dealloc_pd() also should not be used.
	 */

	/* run test */
	struct rpma_peer *peer = NOT_NULLED_PTR;
	int ret = rpma_peer_new(NULL, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_ptr_equal(peer, NULL);
}

/*
 * peer_new_test_peer_ptr_eq_NULL -- NULL **peer is not valid
 */
static void
peer_new_test_peer_ptr_eq_NULL(void **unused)
{
	/* configure mocks */
	will_return_maybe(ibv_alloc_pd, MOCK_IBV_PD);
	will_return_maybe(ibv_dealloc_pd, 0);

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
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_IBV_CTX);
	will_return(ibv_alloc_pd, MOCK_VALIDATE_PARAMS | NULL_VALUE);
	will_return(ibv_alloc_pd, ENOMEM);
	will_return_maybe(__wrap_malloc, 0 /* no error */);

	/* run test */
	struct rpma_peer *peer = NOT_NULLED_PTR;
	int ret = rpma_peer_new(MOCK_IBV_CTX, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_ptr_equal(peer, NULL);
}

/*
 * peer_new_test_alloc_pd_fail_EAGAIN -- ibv_alloc_pd() fails with EAGAIN
 * (arbitrarily picked error which is not ENOMEM)
 */
static void
peer_new_test_alloc_pd_fail_EAGAIN(void **unused)
{
	/*
	 * configure mocks:
	 * - NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd()
	 * has failed.
	 */
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_IBV_CTX);
	will_return(ibv_alloc_pd, MOCK_VALIDATE_PARAMS | NULL_VALUE);
	will_return(ibv_alloc_pd, EAGAIN);
	will_return_maybe(__wrap_malloc, 0 /* no error */);

	/* run test */
	struct rpma_peer *peer = NOT_NULLED_PTR;
	int ret = rpma_peer_new(MOCK_IBV_CTX, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
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
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_IBV_CTX);
	will_return(ibv_alloc_pd, MOCK_VALIDATE_PARAMS | NULL_VALUE);
	will_return(ibv_alloc_pd, 0 /* no error */);
	will_return_maybe(__wrap_malloc, 0 /* no error */);

	/* run test */
	struct rpma_peer *peer = NOT_NULLED_PTR;
	int ret = rpma_peer_new(MOCK_IBV_CTX, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_UNKNOWN);
}

/*
 * peer_new_test_malloc_fail -- malloc() fail
 */
static void
peer_new_test_malloc_fail(void **unused)
{
	/* configure mocks */
	will_return_maybe(ibv_alloc_pd, MOCK_IBV_PD);
	will_return_maybe(ibv_dealloc_pd, 0);
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
	 * NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd()
	 * succeeded.
	 */
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_IBV_CTX);
	will_return_maybe(ibv_alloc_pd,
			MOCK_VALIDATE_PARAMS | (uint64_t)MOCK_IBV_PD);
	will_return_maybe(__wrap_malloc, 0 /* no error */);

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
	expect_value(ibv_dealloc_pd, pd, MOCK_IBV_PD);
	will_return(ibv_dealloc_pd, MOCK_VALIDATE_PARAMS | 0);

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
 * peer_setup -- prepare a valid rpma_peer object
 */
static int
peer_setup(void **peer_ptr)
{
	/*
	 * configure mocks for rpma_peer_new():
	 * NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd()
	 * succeeded.
	 */
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_IBV_CTX);
	will_return(ibv_alloc_pd, MOCK_VALIDATE_PARAMS | (uint64_t)MOCK_IBV_PD);
	will_return(__wrap_malloc, 0 /* no error */);

	/* setup */
	int ret = rpma_peer_new(MOCK_IBV_CTX, (struct rpma_peer **)peer_ptr);
	assert_int_equal(ret, 0);
	assert_non_null(*peer_ptr);

	return 0;
}

/*
 * peer_delete_test_teardown -- delete the rpma_peer object
 */
static int
peer_teardown(void **peer_ptr)
{
	/*
	 * configure mocks for rpma_peer_delete():
	 * NOTE: it is not allowed to call ibv_alloc_pd() nor malloc() in
	 * rpma_peer_delete().
	 */
	expect_value(ibv_dealloc_pd, pd, MOCK_IBV_PD);
	will_return(ibv_dealloc_pd, MOCK_VALIDATE_PARAMS | 0);

	/* teardown */
	int ret = rpma_peer_delete((struct rpma_peer **)peer_ptr);
	assert_int_equal(ret, 0);
	assert_null(*peer_ptr);

	return 0;
}

/*
 * peer_delete_test_dealloc_pd_fail -- ibv_dealloc_pd() fail
 */
static void
peer_delete_test_dealloc_pd_fail(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;

	/*
	 * configure mocks for rpma_peer_delete():
	 * NOTE: it is not allowed to call ibv_alloc_pd() nor malloc() in
	 * rpma_peer_delete().
	 */
	expect_value(ibv_dealloc_pd, pd, MOCK_IBV_PD);
	will_return(ibv_dealloc_pd, MOCK_VALIDATE_PARAMS | EBUSY);

	/* run test */
	int ret = rpma_peer_delete(&peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EBUSY);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_peer_new() unit tests */
		cmocka_unit_test(peer_new_test_ibv_ctx_eq_NULL),
		cmocka_unit_test(peer_new_test_peer_ptr_eq_NULL),
		cmocka_unit_test(peer_new_test_alloc_pd_fail_ENOMEM),
		cmocka_unit_test(peer_new_test_alloc_pd_fail_EAGAIN),
		cmocka_unit_test(peer_new_test_alloc_pd_fail_no_error),
		cmocka_unit_test(peer_new_test_malloc_fail),
		cmocka_unit_test(peer_new_test_success),

		/* rpma_peer_delete() unit tests */
		cmocka_unit_test(peer_delete_test_invalid_peer_ptr),
		cmocka_unit_test(peer_delete_test_null_peer),
		cmocka_unit_test_setup_teardown(
				peer_delete_test_dealloc_pd_fail,
				peer_setup, peer_teardown),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
