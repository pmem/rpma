/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * cq.h -- librpma completion-queue-related internal definitions
 */

#ifndef LIBRPMA_CQ_H
#define LIBRPMA_CQ_H

#include <infiniband/verbs.h>
#include <rdma/rdma_cma.h>

#include "librpma.h"

/*
 * ERRORS
 * rpma_cq_get_ibv_cq() cannot fail.
 */
struct ibv_cq *rpma_cq_get_ibv_cq(const struct rpma_cq *cq);

/*
 * ERRORS
 * rpma_cq_new() can fail with the following errors:
 *
 * - RPMA_E_PROVIDER - ibv_create_comp_channel(3), ibv_create_cq(3) or ibv_req_notify_cq(3) failed
 *   with a provider error
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_cq_new(struct ibv_context *ibv_ctx, int cqe, struct ibv_comp_channel *shared_channel,
		struct rpma_cq **cq_ptr);

/*
 * ERRORS
 * rpma_cq_delete() can fail with the following errors:
 *
 * - RPMA_E_PROVIDER - ibv_destroy_cq(3) or ibv_destroy_comp_channel(3) failed with a provider
 *   error
 */
int rpma_cq_delete(struct rpma_cq **cq_ptr);

#endif /* LIBRPMA_CQ_H */
