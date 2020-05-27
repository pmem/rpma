/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * peer-test.c -- a peer unit test
 */

#include "cmocka_headers.h"

#include "peer.h"
#include "rpma_err.h"

#include "librpma.h"

#include <infiniband/verbs.h>

#define MOCK_DEV  0xDEDEDEDE
#define MOCK_PTR1 0xABCDEFAB
#define MOCK_PTR2 0xFEDCBAFE

struct peer_new_state {
	/* input arguments */
	struct rpma_peer_cfg *pcfg;
	struct ibv_context *dev;

	/* output object */
	struct rpma_peer peer_obj;

	/* output arguments */
	struct rpma_peer *peer;
};

struct peer_delete_state {
	/* argument */
	struct rpma_peer *peer;

	/* input object */
	struct rpma_peer peer_obj;
};

/*
 * __wrap_ibv_alloc_pd -- ibv_alloc_pd() mock
 */
static struct ibv_pd *
__wrap_ibv_alloc_pd(struct ibv_context *context)
{
	check_expected_ptr(context);

	/*
	 * The ibv_alloc_pd(3) manual page does not document that this function
	 * returns any error via errno but seemingly it is. For the usability
	 * sake, in librpma we try to deduce what really happened using the
	 * errno value.
	 */
	errno = mock_type(int);

	return mock_type(struct ibv_pd *);
}

/*
 * ibv_alloc_pd_configure -- configure ibv_alloc_pd() mock
 */
static void
ibv_alloc_pd_configure(struct ibv_context *context, int error_code,
		struct ibv_pd *pd_exp)
{
	expect_value(__wrap_ibv_alloc_pd, context, context);
	will_return(__wrap_ibv_dealloc_pd, error_code);
	will_return(__wrap_ibv_alloc_pd, pd_exp);
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
 * ibv_dealloc_pd_configure -- configure ibv_dealloc_pd() mock
 */
static void
ibv_dealloc_pd_configure(struct ibv_pd *pd, int ret_exp)
{
	expect_value(__wrap_ibv_dealloc_pd, pd, pd);
	will_return(__wrap_ibv_dealloc_pd, ret_exp);
}

/*
 * __wrap_malloc -- malloc() mock
 */
static void *
__wrap_malloc(size_t size)
{
	check_expected(size);

	errno = mock_type(int);

	return mock_type(int);
}

/*
 * malloc_configure -- configure malloc() mock
 */
static void
malloc_configure(size_t size, int error_code, void *ret_exp)
{
	expect_value(__wrap_malloc, size, size);
	will_return(__wrap_malloc, error_code);
	will_return(__wrap_malloc, ret_exp);
}

/*
 * __wrap_free -- free() mock
 */
static void
__wrap_free(void *addr)
{
	check_expected(addr);
}

/*
 * free_configure -- configure free() mock
 */
static void
free_configure(void *addr)
{
	expect_value(__wrap_free, addr, addr);
}

/*
 * peer_new_setup -- prepare valid arguments
 */
static void
peer_new_setup(void **state_ptr)
{
	struct peer_new_state *st = *state_ptr;
	st->pcfg = NULL;
	st->dev = MOCK_DEV;
	st->peer = NULL;
}

/*
 * peer_new_test_invalid_cfg -- non-NULL cfg is not valid
 * NOTE: This test is valid only to a moment when cfg support will be ready.
 */
static void
peer_new_test_invalid_cfg(void **state_ptr)
{
	struct peer_new_state *st = *state_ptr;
	st->pcfg = MOCK_PTR1;

	/* run test */
	int ret = rpma_peer_new(st->pcfg, st->dev, &st->peer);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * peer_new_test_invalid_dev -- NULL dev is not valid
 */
static void
peer_new_test_invalid_dev(void **state_ptr)
{
	struct peer_new_state *st = *state_ptr;
	st->dev = NULL;

	/* run test */
	int ret = rpma_peer_new(st->pcfg, st->dev, &st->peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * peer_new_test_invalid_peer_ptr -- NULL **peer is not valid
 */
static void
peer_new_test_invalid_peer_ptr(void **state_ptr)
{
	struct peer_new_state *st = *state_ptr;

	/* run test */
	int ret = rpma_peer_new(st->pcfg, st->dev, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * peer_new_test_alloc_pd_fail_01 -- ibv_alloc_pd() returns NULL and
 * errno == ENOMEM
 */
static void
peer_new_test_alloc_pd_fail_01(void **state_ptr)
{
	struct peer_new_state *st = *state_ptr;

	/* configure mock */
	struct ibv_pd *pd_exp = NULL;
	ibv_alloc_pd_configure(st->dev, ENOMEM, pd_exp);

	/* run test */
	int ret = rpma_peer_new(st->pcfg, st->dev, &st->peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
}

/*
 * peer_new_test_alloc_pd_fail_02 -- ibv_alloc_pd() returns NULL and
 * (errno != ENOMEM && errno != 0)
 */
static void
peer_new_test_alloc_pd_fail_02(void **state_ptr)
{
	struct peer_new_state *st = *state_ptr;

	/* configure mock - EAGAIN picked arbitrarily */
	int error_code = EAGAIN;
	struct ibv_pd *pd_exp = NULL;
	ibv_alloc_pd_configure(st->dev, error_code, pd_exp);

	/* run test */
	int ret = rpma_peer_new(st->pcfg, st->dev, &st->peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(Rpma_provider_error, error_code);
}

/*
 * peer_new_test_alloc_pd_fail_03 -- ibv_alloc_pd() returns NULL and
 * (errno == 0)
 */
static void
peer_new_test_alloc_pd_fail_03(void **state_ptr)
{
	struct peer_new_state *st = *state_ptr;

	/* configure mock */
	struct ibv_pd *pd_exp = NULL;
	ibv_alloc_pd_configure(st->dev, 0, pd_exp);

	/* run test */
	int ret = rpma_peer_new(st->pcfg, st->dev, &st->peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_UNKNOWN);
}

/*
 * peer_new_test_malloc_fail -- malloc() returns NULL and (errno == 0)
 */
static void
peer_new_test_malloc_fail(void **state_ptr)
{
	struct peer_new_state *st = *state_ptr;

	/* configure mocks */
	struct ibv_pd *pd_exp = MOCK_PTR1;
	ibv_alloc_pd_configure(st->dev, 0, pd_exp);
	malloc_configure(sizeof(*st->peer), ENOMEM, NULL);
	ibv_dealloc_pd_configure(pd_exp, 0, 0);

	/* run test */
	int ret = rpma_peer_new(st->pcfg, st->dev, &st->peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
}

/*
 * peer_new_test_success -- happy day scenario
 */
static void
peer_new_test_success(void **state_ptr)
{
	struct peer_new_state *st = *state_ptr;

	/* configure mocks */
	struct ibv_pd *pd_exp = MOCK_PTR1;
	ibv_alloc_pd_configure(st->dev, 0, pd_exp);
	malloc_configure(sizeof(*st->peer), 0, &st->peer_obj);

	/* run test */
	int ret = rpma_peer_new(st->pcfg, st->dev, &st->peer);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_ptr_equal(st->peer, &st->peer_obj);
	assert_ptr_equal(st->peer->pd, pd_exp);
}

/*
 * peer_delete_setup -- prepare valid arguments
 */
static void
peer_delete_setup(void **state_ptr)
{
	struct peer_delete_state *st = *state_ptr;
	st->peer = &st->peer_obj;
	st->peer->pd = MOCK_PTR1;
}

/*
 * peer_delete_test_invalid_peer_ptr -- NULL **peer is not valid
 */
static void
peer_delete_test_invalid_peer_ptr(void **state_ptr)
{
	struct peer_delete_state *st = *state_ptr;

	/* run test */
	int ret = rpma_peer_delete(NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * peer_delete_test_null_peer -- valid NULL *peer
 */
static void
peer_delete_test_null_peer(void **state_ptr)
{
	struct peer_delete_state *st = *state_ptr;
	st->peer = NULL;

	/* run test */
	int ret = rpma_peer_delete(&st->peer);

	/* verify the result */
	assert_int_equal(ret, 0);
}

/*
 * peer_delete_test_dealloc_pd_fail -- ibv_dealloc_pd() returns error code
 */
static void
peer_delete_test_dealloc_pd_fail(void **state_ptr)
{
	struct peer_delete_state *st = *state_ptr;

	/* configure mocks */
	int error_code = EAGAIN;
	ibv_dealloc_pd_configure(st->peer->pd, error_code);

	/* run test */
	int ret = rpma_peer_delete(&st->peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(Rpma_provider_error, error_code);
	assert_ptr_equal(st->peer, &st->peer_obj);
}

/*
 * peer_delete_test_success -- happy day scenario
 */
static void
peer_delete_test_success(void **state_ptr)
{
	struct peer_delete_state *st = *state_ptr;

	/* configure mocks */
	ibv_dealloc_pd_configure(st->peer->pd, 0);
	free_configure(st->peer);

	/* run test */
	int ret = rpma_peer_delete(&st->peer);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_ptr_equal(st->peer, NULL);
}

#define peer_new_test_cmocka(test) \
	cmocka_unit_test_prestate_setup( \
			(test), \
			peer_new_setup, &prestate_new)

#define peer_delete_test_cmocka(test) \
	cmocka_unit_test_prestate_setup( \
			(test), \
			peer_delete_setup, &prestate_delete)

int
main(int argc, char *argv[])
{
	struct peer_new_state prestate_new;
	struct peer_delete_state prestate_delete;

	const struct CMUnitTest tests[] = {
		/* rpma_peer_new() unit tests */
		peer_new_test_cmocka(peer_new_test_invalid_cfg),
		peer_new_test_cmocka(peer_new_test_invalid_dev),
		peer_new_test_cmocka(peer_new_test_invalid_peer_ptr),
		peer_new_test_cmocka(peer_new_test_alloc_pd_fail_01),
		peer_new_test_cmocka(peer_new_test_alloc_pd_fail_02),
		peer_new_test_cmocka(peer_new_test_alloc_pd_fail_03),
		peer_new_test_cmocka(peer_new_test_malloc_fail),
		peer_new_test_cmocka(peer_new_test_success),

		/* rpma_peer_delete() unit tests */
		peer_delete_test_cmocka(peer_delete_test_invalid_peer_ptr),
		peer_delete_test_cmocka(peer_delete_test_null_peer),
		peer_delete_test_cmocka(peer_delete_test_dealloc_pd_fail),
		peer_delete_test_cmocka(peer_delete_test_success),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
