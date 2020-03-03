/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * peer.h -- internal definitions for librpma peer
 */
#ifndef RPMA_PEER_H
#define RPMA_PEER_H

#include <rdma/rdma_cma.h>
#include <librpma.h>

struct rpma_peer {
	struct ibv_pd *pd;
};

#endif /* RPMA_PEER_H */
