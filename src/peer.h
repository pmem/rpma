/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * peer.h -- librpma peer-related internal definitions
 */

#ifndef LIBRPMA_PEER_H
#define LIBRPMA_PEER_H

#include <rdma/rdma_cma.h>

int rpma_peer_create_qp(struct rpma_peer *peer, struct rdma_cm_id *id);

#endif /* LIBRPMA_PEER_H */
