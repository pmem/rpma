// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

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

int OdpCapable = MOCK_ODP_CAPABLE;
int OdpIncapable = MOCK_ODP_INCAPABLE;

/*
 * setup__peer -- prepare a valid rpma_peer object
 * (encapsulating the MOCK_IBV_PD)
 *
 * Note:
 * - in - int **is_odp_capable
 * - out - struct rpma_peer **
 */
int
setup__peer(void **in_out)
{
	/*
	 * configure mocks for rpma_peer_new():
	 * NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd()
	 * succeeded.
	 */
	will_return(rpma_utils_ibv_context_is_odp_capable, **(int **)in_out);
	struct ibv_alloc_pd_mock_args alloc_args = {MOCK_VALIDATE, MOCK_IBV_PD};
	will_return(ibv_alloc_pd, &alloc_args);
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_VERBS);
	will_return(__wrap__test_malloc, MOCK_OK);

	/* setup */
	int ret = rpma_peer_new(MOCK_VERBS, (struct rpma_peer **)in_out);
	assert_int_equal(ret, 0);
	assert_non_null(*in_out);

	return 0;
}

/*
 * delete__teardown -- delete the rpma_peer object
 */
int
teardown__peer(void **peer_ptr)
{
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
	int ret = rpma_peer_delete((struct rpma_peer **)peer_ptr);
	assert_int_equal(ret, MOCK_OK);
	assert_null(*peer_ptr);

	return 0;
}
