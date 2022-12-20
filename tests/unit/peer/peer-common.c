// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */
/* Copyright (c) 2022, Fujitsu Limited */

/*
 * peer-common.c -- the common part of the peer unit test
 */

#include <infiniband/verbs.h>

#include "cmocka_headers.h"
#include "conn_req.h"
#include "mocks-ibverbs.h"
#include "mocks-rpma-utils.h"
#include "peer.h"
#include "peer-common.h"
#include "test-common.h"

struct prestate prestate_Capable = {IBV_TRANSPORT_IB, 0, 0, MOCK_ODP_CAPABLE,
		MOCK_ATOMIC_WRITE_CAPABLE, NULL};
struct prestate prestate_Incapable = {IBV_TRANSPORT_IB, 0, 0, MOCK_ODP_INCAPABLE,
		MOCK_ATOMIC_WRITE_INCAPABLE, NULL};

/*
 * setup__peer -- prepare a valid rpma_peer object
 * (encapsulating the MOCK_IBV_PD)
 */
int
setup__peer(void **pprestate)
{
	struct prestate *prestate = *pprestate;

	/*
	 * configure mocks for rpma_peer_new():
	 * NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd()
	 * succeeded.
	 */
	will_return(rpma_utils_ibv_context_is_atomic_write_capable,
			prestate->is_atomic_write_capable);
	will_return(rpma_utils_ibv_context_is_odp_capable,
			prestate->is_odp_capable);
	struct ibv_alloc_pd_mock_args alloc_args = {MOCK_VALIDATE, MOCK_IBV_PD};
	will_return(ibv_alloc_pd, &alloc_args);
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_VERBS);
	will_return(__wrap__test_malloc, MOCK_OK);

	/* setup */
	int ret = rpma_peer_new(MOCK_VERBS, &prestate->peer);
	assert_int_equal(ret, 0);
	assert_non_null(prestate->peer);

	return 0;
}

/*
 * teardown__peer -- delete the rpma_peer object
 */
int
teardown__peer(void **pprestate)
{
	struct prestate *prestate = *pprestate;

	/*
	 * configure mocks for rpma_peer_delete():
	 * NOTE: it is not allowed to call ibv_alloc_pd() nor malloc() in
	 * rpma_peer_delete().
	 */
	struct ibv_dealloc_pd_mock_args dealloc_args =
		{MOCK_VALIDATE, MOCK_OK};
	will_return(ibv_dealloc_pd, &dealloc_args);
	expect_value(ibv_dealloc_pd, pd, MOCK_IBV_PD);

	/* teardown */
	int ret = rpma_peer_delete(&prestate->peer);
	assert_int_equal(ret, MOCK_OK);
	assert_null(prestate->peer);

	return 0;
}
