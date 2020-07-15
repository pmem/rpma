/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mocks-peer.c -- an peer.c module mocks
 */

#include <rdma/rdma_cma.h>
#include <librpma.h>

#include "rpma_err.h"
#include "cmocka_headers.h"
#include "test-common.h"

/*
 * rpma_peer_create_qp -- rpma_peer_create_qp() mock
 */
int
rpma_peer_create_qp(struct rpma_peer *peer, struct rdma_cm_id *id,
		struct ibv_cq *cq)
{
	assert_ptr_equal(peer, MOCK_PEER);
	check_expected_ptr(id);
	assert_ptr_equal(cq, MOCK_CQ);

	int result = mock_type(int);
	if (result == RPMA_E_PROVIDER)
		Rpma_provider_error = mock_type(int);

	return result;
}
