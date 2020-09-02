// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mocks-rpma-peer.c -- librpma peer.c module mocks
 */

#include <rdma/rdma_cma.h>
#include <librpma.h>

#include "cmocka_headers.h"
#include "mocks-ibverbs.h"
#include "mocks-rpma-peer.h"
#include "rpma_err.h"
#include "test-common.h"

/*
 * rpma_peer_create_qp -- rpma_peer_create_qp() mock
 */
int
rpma_peer_create_qp(struct rpma_peer *peer, struct rdma_cm_id *id,
		struct ibv_cq *cq, struct rpma_conn_cfg *cfg)
{
	assert_ptr_equal(peer, MOCK_PEER);
	check_expected_ptr(id);
	assert_ptr_equal(cq, MOCK_IBV_CQ);

	int result = mock_type(int);
	if (result == RPMA_E_PROVIDER)
		Rpma_provider_error = mock_type(int);

	return result;
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
