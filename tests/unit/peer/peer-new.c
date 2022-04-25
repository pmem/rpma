// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * peer-new.c -- a peer unit test
 *
 * APIs covered:
 * - rpma_peer_new()
 * - rpma_peer_delete()
 */

#include <infiniband/verbs.h>

#include "cmocka_headers.h"
#include "conn_req.h"
#include "mocks-ibverbs.h"
#include "mocks-rpma-utils.h"
#include "peer.h"
#include "peer-common.h"
#include "test-common.h"

/*
 * new__ibv_ctx_eq_NULL -- NULL ibv_ctx is not valid
 */
static void
new__ibv_ctx_eq_NULL(void **unused)
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
 * new__peer_ptr_eq_NULL -- NULL **peer is not valid
 */
static void
new__peer_ptr_eq_NULL(void **unused)
{
	/*
	 * NOTE: it is not allowed for peer to allocate any resource before
	 * validating arguments.
	 */

	/* run test */
	int ret = rpma_peer_new(MOCK_VERBS, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new__ibv_ctx_and_peer_ptr_eq_NULL -- NULL ibv_ctx and NULL **peer
 * are not valid
 */
static void
new__ibv_ctx_and_peer_ptr_eq_NULL(void **unused)
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
 * new__alloc_pd_ENOMEM -- ibv_alloc_pd() fails with ENOMEM
 */
static void
new__alloc_pd_ENOMEM(void **unused)
{
	/*
	 * configure mocks:
	 * - NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd()
	 * has failed.
	 */
	struct ibv_alloc_pd_mock_args alloc_args = {MOCK_VALIDATE, NULL};
	will_return(ibv_alloc_pd, &alloc_args);
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_VERBS);
	will_return(ibv_alloc_pd, ENOMEM);
	will_return_maybe(rpma_utils_ibv_context_is_odp_capable, 1);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_new(MOCK_VERBS, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(peer);
}

/*
 * new__alloc_pd_ERRNO -- ibv_alloc_pd() fails with MOCK_ERRNO
 */
static void
new__alloc_pd_ERRNO(void **unused)
{
	/*
	 * configure mocks:
	 * - NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd()
	 * has failed.
	 */
	struct ibv_alloc_pd_mock_args alloc_args = {MOCK_VALIDATE, NULL};
	will_return(ibv_alloc_pd, &alloc_args);
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_VERBS);
	will_return(ibv_alloc_pd, MOCK_ERRNO);
	will_return_maybe(rpma_utils_ibv_context_is_odp_capable, 1);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_new(MOCK_VERBS, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(peer);
}

/*
 * new__alloc_pd_no_error -- ibv_alloc_pd() fails without error
 */
static void
new__alloc_pd_no_error(void **unused)
{
	/*
	 * configure mocks:
	 * - NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd()
	 * has failed.
	 */
	struct ibv_alloc_pd_mock_args alloc_args = {MOCK_VALIDATE, NULL};
	will_return(ibv_alloc_pd, &alloc_args);
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_VERBS);
	will_return(ibv_alloc_pd, MOCK_OK);
	will_return_maybe(rpma_utils_ibv_context_is_odp_capable, 1);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_new(MOCK_VERBS, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_UNKNOWN);
	assert_null(peer);
}

/*
 * new__odp_ERRNO -- rpma_utils_ibv_context_is_odp_capable()
 * fails with MOCK_ERRNO
 */
static void
new__odp_ERRNO(void **unused)
{
	/* configure mocks */
	will_return(rpma_utils_ibv_context_is_odp_capable, MOCK_ERR_PENDING);
	will_return(rpma_utils_ibv_context_is_odp_capable, RPMA_E_PROVIDER);
	will_return(rpma_utils_ibv_context_is_odp_capable, MOCK_ERRNO);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);
	will_return_maybe(ibv_alloc_pd, MOCK_IBV_PD);
	will_return_maybe(ibv_dealloc_pd, MOCK_OK);

	/* run test */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_new(MOCK_VERBS, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(peer);
}

/*
 * new__malloc_ERRNO-- malloc() fails with MOCK_ERRNO
 */
static void
new__malloc_ERRNO(void **unused)
{
	/* configure mocks */
	will_return(__wrap__test_malloc, MOCK_ERRNO);
	struct ibv_alloc_pd_mock_args alloc_args =
		{MOCK_PASSTHROUGH, MOCK_IBV_PD};
	will_return_maybe(ibv_alloc_pd, &alloc_args);
	struct ibv_dealloc_pd_mock_args dealloc_args =
		{MOCK_PASSTHROUGH, MOCK_OK};
	will_return_maybe(ibv_dealloc_pd, &dealloc_args);
	will_return_maybe(rpma_utils_ibv_context_is_odp_capable, 1);

	/* run test */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_new(MOCK_VERBS, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(peer);
}

/*
 * new__success -- happy day scenario
 */
static void
new__success(void **unused)
{
	/*
	 * configure mocks for rpma_peer_new():
	 * NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd()
	 * succeeded.
	 */
	struct ibv_alloc_pd_mock_args alloc_args = {MOCK_VALIDATE, MOCK_IBV_PD};
	will_return(ibv_alloc_pd, &alloc_args);
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_VERBS);
	will_return(rpma_utils_ibv_context_is_odp_capable, 1);
	will_return(__wrap__test_malloc, MOCK_OK);

	/* run test - step 1 */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_new(MOCK_VERBS, &peer);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(peer);

	/*
	 * configure mocks for rpma_peer_delete():
	 * NOTE: it is not allowed to call ibv_alloc_pd() nor malloc() in
	 * rpma_peer_delete().
	 */
	struct ibv_dealloc_pd_mock_args dealloc_args =
		{MOCK_VALIDATE, MOCK_OK};
	will_return(ibv_dealloc_pd, &dealloc_args);
	expect_value(ibv_dealloc_pd, pd, MOCK_IBV_PD);

	/* run test - step 2 */
	ret = rpma_peer_delete(&peer);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(peer);
}

/*
 * delete__invalid_peer_ptr -- NULL **peer is not valid
 */
static void
delete__invalid_peer_ptr(void **unused)
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
 * delete__null_peer -- valid NULL *peer
 */
static void
delete__null_peer(void **unused)
{
	/*
	 * NOTE: it is not allowed for peer to allocate any resource when
	 * quick-exiting.
	 */

	/* run test */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_delete(&peer);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_null(peer);
}

/*
 * delete__dealloc_pd_ERRNO -- ibv_dealloc_pd() fails with MOCK_ERRNO
 */
static void
delete__dealloc_pd_ERRNO(void **unused)
{
	struct prestate *prestate = &prestate_OdpCapable;
	assert_int_equal(setup__peer((void **)&prestate), 0);
	assert_non_null(prestate->peer);

	/*
	 * configure mocks for rpma_peer_delete():
	 * NOTE: it is not allowed to call ibv_alloc_pd() nor malloc() in
	 * rpma_peer_delete().
	 */
	struct ibv_dealloc_pd_mock_args dealloc_args = {MOCK_VALIDATE,
			MOCK_ERRNO};
	will_return(ibv_dealloc_pd, &dealloc_args);
	expect_value(ibv_dealloc_pd, pd, MOCK_IBV_PD);

	/* run test */
	int ret = rpma_peer_delete(&prestate->peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(prestate->peer);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_peer_new() unit tests */
		cmocka_unit_test(new__ibv_ctx_eq_NULL),
		cmocka_unit_test(new__peer_ptr_eq_NULL),
		cmocka_unit_test(new__ibv_ctx_and_peer_ptr_eq_NULL),
		cmocka_unit_test(new__alloc_pd_ENOMEM),
		cmocka_unit_test(new__alloc_pd_ERRNO),
		cmocka_unit_test(new__alloc_pd_no_error),
		cmocka_unit_test(new__odp_ERRNO),
		cmocka_unit_test(new__malloc_ERRNO),
		cmocka_unit_test(new__success),

		/* rpma_peer_delete() unit tests */
		cmocka_unit_test(delete__invalid_peer_ptr),
		cmocka_unit_test(delete__null_peer),
		cmocka_unit_test(delete__dealloc_pd_ERRNO)
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
