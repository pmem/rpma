/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2021, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * peer.h -- librpma peer-related internal definitions
 */

#ifndef LIBRPMA_PEER_H
#define LIBRPMA_PEER_H

#include "librpma.h"
#include "cq.h"

#include <rdma/rdma_cma.h>

/*
 * ERRORS
 * rpma_peer_create_qp() can fail with the following errors:
 *
 * - RPMA_E_INVAL - peer, id or cq is NULL
 * - RPMA_E_PROVIDER - allocating a QP failed
 */
int rpma_peer_create_qp(struct rpma_peer *peer, struct rdma_cm_id *id,
		struct rpma_cq *cq, struct rpma_cq *rcq,
		const struct rpma_conn_cfg *cfg);

/*
 * ASSUMPTIONS
 * - peer != NULL && ibv_mr_ptr != NULL && addr != NULL && length > 0 &&
 *   && peer->pd != NULL
 *
 * ERRORS
 * rpma_peer_mr_reg() can fail with the following error:
 *
 * - RPMA_E_PROVIDER - registering the memory region failed
 */
int rpma_peer_mr_reg(struct rpma_peer *peer, struct ibv_mr **ibv_mr_ptr,
		void *addr, size_t length, int usage);

#endif /* LIBRPMA_PEER_H */
