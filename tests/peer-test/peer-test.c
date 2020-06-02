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

#define MOCK_IBV_CTX		(struct ibv_context *)0x00C0
#define MOCK_IBV_PD		(struct ibv_pd *)0x00D0

#define MOCK_PASSTHROUGH	0
#define MOCK_VALIDATE		1

#define NO_ERROR		0

struct ibv_alloc_pd_mock_args {
	int validate_params;
	struct ibv_pd *pd;
};

/*
 * ibv_alloc_pd -- ibv_alloc_pd() mock
 */
struct ibv_pd *
ibv_alloc_pd(struct ibv_context *ibv_ctx)
{
	struct ibv_alloc_pd_mock_args *args =
			mock_type(struct ibv_alloc_pd_mock_args *);
	if (args->validate_params == MOCK_VALIDATE)
		check_expected_ptr(ibv_ctx);

	if (args->pd != NULL)
		return args->pd;

	/*
	 * The ibv_alloc_pd(3) manual page does not document that this function
	 * returns any error via errno but seemingly it is. For the usability
	 * sake, in librpma we try to deduce what really happened using the
	 * errno value.
	 */
	errno = mock_type(int);

	return NULL;
}

struct ibv_dealloc_pd_mock_args {
	int validate_params;
	int ret;
};

/*
 * ibv_dealloc_pd -- ibv_dealloc_pd() mock
 */
int
ibv_dealloc_pd(struct ibv_pd *pd)
{
	struct ibv_dealloc_pd_mock_args *args =
			mock_type(struct ibv_dealloc_pd_mock_args *);
	if (args->validate_params == MOCK_VALIDATE)
		check_expected_ptr(pd);

	return args->ret;
}

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

/*
 * peer_new_test_ibv_ctx_eq_NULL -- NULL ibv_ctx is not valid
 */
static void
peer_new_test_ibv_ctx_eq_NULL(void **unused)
{
	/*
	 * NOTE: it is not allowed for peer to allocate any resource before
	 * validating arguments.
	 */

	/* run test */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_new(NULL, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(peer);
}

/*
 * peer_new_test_peer_ptr_eq_NULL -- NULL **peer is not valid
 */
static void
peer_new_test_peer_ptr_eq_NULL(void **unused)
{
	/*
	 * NOTE: it is not allowed for peer to allocate any resource before
	 * validating arguments.
	 */

	/* run test */
	int ret = rpma_peer_new(MOCK_IBV_CTX, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * peer_new_test_ibv_ctx_and_peer_ptr_eq_NULL -- NULL ibv_ctx and NULL **peer
 * are not valid
 */
static void
peer_new_test_ibv_ctx_and_peer_ptr_eq_NULL(void **unused)
{
	/*
	 * NOTE: it is not allowed for peer to allocate any resource before
	 * validating arguments.
	 */

	/* run test */
	int ret = rpma_peer_new(NULL, NULL);

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
	struct ibv_alloc_pd_mock_args alloc_args = {MOCK_VALIDATE, NULL};
	will_return(ibv_alloc_pd, &alloc_args);
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_IBV_CTX);
	will_return(ibv_alloc_pd, ENOMEM);
	will_return_maybe(__wrap__test_malloc, NO_ERROR);

	/* run test */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_new(MOCK_IBV_CTX, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(peer);
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
	struct ibv_alloc_pd_mock_args alloc_args = {MOCK_VALIDATE, NULL};
	will_return(ibv_alloc_pd, &alloc_args);
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_IBV_CTX);
	will_return(ibv_alloc_pd, EAGAIN);
	will_return_maybe(__wrap__test_malloc, NO_ERROR);

	/* run test */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_new(MOCK_IBV_CTX, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(peer);
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
	struct ibv_alloc_pd_mock_args alloc_args = {MOCK_VALIDATE, NULL};
	will_return(ibv_alloc_pd, &alloc_args);
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_IBV_CTX);
	will_return(ibv_alloc_pd, NO_ERROR);
	will_return_maybe(__wrap__test_malloc, NO_ERROR);

	/* run test */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_new(MOCK_IBV_CTX, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_UNKNOWN);
	assert_null(peer);
}

/*
 * peer_new_test_malloc_fail -- malloc() fail
 */
static void
peer_new_test_malloc_fail(void **unused)
{
	/* configure mocks */
	struct ibv_alloc_pd_mock_args alloc_args =
		{MOCK_PASSTHROUGH, MOCK_IBV_PD};
	will_return_maybe(ibv_alloc_pd, &alloc_args);
	struct ibv_dealloc_pd_mock_args dealloc_args =
		{MOCK_PASSTHROUGH, NO_ERROR};
	will_return_maybe(ibv_dealloc_pd, &dealloc_args);
	will_return(__wrap__test_malloc, ENOMEM);

	/* run test */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_new(MOCK_IBV_CTX, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(peer);
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
	struct ibv_alloc_pd_mock_args alloc_args = {MOCK_VALIDATE, MOCK_IBV_PD};
	will_return(ibv_alloc_pd, &alloc_args);
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_IBV_CTX);
	will_return(__wrap__test_malloc, NO_ERROR);

	/* run test - step 1 */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_new(MOCK_IBV_CTX, &peer);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
	assert_non_null(peer);

	/*
	 * configure mocks for rpma_peer_delete():
	 * NOTE: it is not allowed to call ibv_alloc_pd() nor malloc() in
	 * rpma_peer_delete().
	 */
	struct ibv_dealloc_pd_mock_args dealloc_args =
		{MOCK_VALIDATE, NO_ERROR};
	will_return(ibv_dealloc_pd, &dealloc_args);
	expect_value(ibv_dealloc_pd, pd, MOCK_IBV_PD);

	/* run test - step 2 */
	ret = rpma_peer_delete(&peer);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
	assert_null(peer);
}

/*
 * peer_delete_test_invalid_peer_ptr -- NULL **peer is not valid
 */
static void
peer_delete_test_invalid_peer_ptr(void **unused)
{
	/*
	 * NOTE: it is not allowed for peer to allocate any resource before
	 * validating arguments.
	 */

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
	/*
	 * NOTE: it is not allowed for peer to allocate any resource when
	 * quick-exiting.
	 */

	/* run test */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_delete(&peer);

	/* verify the result */
	assert_int_equal(ret, NO_ERROR);
	assert_null(peer);
}

/*
 * peer_setup -- prepare a valid rpma_peer object
 * (encapsulating the MOCK_IBV_PD)
 */
static int
peer_setup(void **peer_ptr)
{
	/*
	 * configure mocks for rpma_peer_new():
	 * NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd()
	 * succeeded.
	 */
	struct ibv_alloc_pd_mock_args alloc_args = {MOCK_VALIDATE, MOCK_IBV_PD};
	will_return(ibv_alloc_pd, &alloc_args);
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_IBV_CTX);
	will_return(__wrap__test_malloc, NO_ERROR);

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
	struct ibv_dealloc_pd_mock_args dealloc_args =
		{MOCK_VALIDATE, NO_ERROR};
	will_return(ibv_dealloc_pd, &dealloc_args);
	expect_value(ibv_dealloc_pd, pd, MOCK_IBV_PD);

	/* teardown */
	int ret = rpma_peer_delete((struct rpma_peer **)peer_ptr);
	assert_int_equal(ret, NO_ERROR);
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
	struct ibv_dealloc_pd_mock_args dealloc_args = {MOCK_VALIDATE, EBUSY};
	will_return(ibv_dealloc_pd, &dealloc_args);
	expect_value(ibv_dealloc_pd, pd, MOCK_IBV_PD);

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
		cmocka_unit_test(peer_new_test_ibv_ctx_and_peer_ptr_eq_NULL),
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
