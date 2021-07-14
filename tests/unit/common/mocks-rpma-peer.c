// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * mocks-rpma-peer.c -- librpma peer.c module mocks
 */

#include <rdma/rdma_cma.h>
#include <librpma.h>

#include "cmocka_headers.h"
#include "mocks-ibverbs.h"
#include "mocks-rpma-peer.h"
#include "mocks-rpma-cq.h"

/*
 * rpma_peer_create_qp -- rpma_peer_create_qp() mock
 */
int
rpma_peer_create_qp(struct rpma_peer *peer, struct rdma_cm_id *id,
		struct rpma_cq *cq, struct rpma_cq *rcq,
		const struct rpma_conn_cfg *cfg)
{
	assert_ptr_equal(peer, MOCK_PEER);
	check_expected_ptr(id);
	assert_ptr_equal(cq, MOCK_RPMA_CQ);
	check_expected_ptr(rcq);
	check_expected_ptr(cfg);

	int result = mock_type(int);
	/* XXX validate the errno handling */
	if (result == RPMA_E_PROVIDER)
		errno = mock_type(int);

	return result;
}

/*
 * rpma_peer_mr_reg -- a mock of rpma_peer_mr_reg()
 */
int
rpma_peer_mr_reg(struct rpma_peer *peer, struct ibv_mr **ibv_mr_ptr,
		void *addr, size_t length, int usage)
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
	assert_int_equal(usage, args->usage);

	*ibv_mr_ptr = args->mr;
	if (*ibv_mr_ptr == NULL) {
		/* XXX validate the errno handling */
		errno = args->verrno;
		return RPMA_E_PROVIDER;
	}

	(*ibv_mr_ptr)->addr = addr;
	(*ibv_mr_ptr)->length = length;
	(*ibv_mr_ptr)->rkey = MOCK_RKEY;

	return 0;
}
