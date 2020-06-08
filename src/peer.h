/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * peer.h -- librpma peer-related internal definitions
 */

#ifndef LIBRPMA_PEER_H
#define LIBRPMA_PEER_H

#include "librpma.h"

#include <rdma/rdma_cma.h>

/*
 * ERRORS
 * rpma_peer_create_qp() can fail with the following errors:
 *
 * - RPMA_E_INVAL - peer, id or cq is NULL
 * - RPMA_E_PROVIDER - allocating a QP failed
 */
int rpma_peer_create_qp(struct rpma_peer *peer, struct rdma_cm_id *id,
		struct ibv_cq *cq);

#endif /* LIBRPMA_PEER_H */
