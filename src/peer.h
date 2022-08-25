/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2022, Intel Corporation */
/* Copyright 2021-2022, Fujitsu */

/*
 * peer.h -- librpma peer-related internal definitions
 */

#ifndef LIBRPMA_PEER_H
#define LIBRPMA_PEER_H

#include "librpma.h"
#include "cq.h"

#include <rdma/rdma_cma.h>

/*
 * ASSUMPTIONS
 * - peer != NULL && cfg != NULL && ibv_srq_ptr != NULL && rcq_ptr != NULL
 *
 * ERRORS
 * rpma_peer_create_srq() can fail with the following errors:
 *
 * - RPMA_E_PROVIDER - creating a new shared RQ or a shared receive CQ failed
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_peer_create_srq(struct rpma_peer *peer, struct rpma_srq_cfg *cfg,
		struct ibv_srq **ibv_srq_ptr, struct rpma_cq **rcq_ptr);

/*
 * ERRORS
 * rpma_peer_setup_qp() can fail with the following errors:
 *
 * - RPMA_E_INVAL - peer, id or cq is NULL
 * - RPMA_E_PROVIDER - allocating a QP failed
 */
int rpma_peer_setup_qp(struct rpma_peer *peer, struct rdma_cm_id *id, struct rpma_cq *cq,
		struct rpma_cq *rcq, const struct rpma_conn_cfg *cfg);

/*
 * ASSUMPTIONS
 * - peer != NULL && ibv_mr_ptr != NULL && addr != NULL && length > 0 && && peer->pd != NULL
 *
 * ERRORS
 * rpma_peer_setup_mr_reg() can fail with the following error:
 *
 * - RPMA_E_PROVIDER - registering the memory region failed
 */
int rpma_peer_setup_mr_reg(struct rpma_peer *peer, struct ibv_mr **ibv_mr_ptr, void *addr,
		size_t length, int usage);

#endif /* LIBRPMA_PEER_H */
