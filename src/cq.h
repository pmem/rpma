/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2021, Fujitsu */

/*
 * cq.h -- librpma completion-queue-related internal definitions
 */

#ifndef LIBRPMA_CQ_H
#define LIBRPMA_CQ_H

#include <infiniband/verbs.h>
#include <rdma/rdma_cma.h>

#include "librpma.h"

struct rpma_cq;

/*
 * ERRORS
 * rpma_cq_get_fd() cannot fail.
 */
int rpma_cq_get_fd(const struct rpma_cq *cq, int *fd);

/*
 * ERRORS
 * rpma_cq_wait() can fail with the following errors:
 *
 * - RPMA_E_PROVIDER - ibv_req_notify_cq(3) failed with a provider error
 * - RPMA_E_NO_COMPLETION - no completions available
 */
int rpma_cq_wait(struct rpma_cq *cq);

/*
 * ERRORS
 * rpma_cq_get_completion() can fail with the following errors:
 *
 * - RPMA_E_NO_COMPLETION - no completions available
 * - RPMA_E_PROVIDER - ibv_poll_cq(3) failed with a provider error
 * - RPMA_E_UNKNOWN - ibv_poll_cq(3) failed but no provider error is available
 * - RPMA_E_NOSUPP - not supported opcode
 */
int rpma_cq_get_completion(struct rpma_cq *cq, struct rpma_completion *cmpl);

/*
 * ERRORS
 * rpma_cq_new() can fail with the following errors:
 *
 * - RPMA_E_PROVIDER - ibv_create_comp_channel(3), ibv_create_cq(3) or
 * ibv_req_notify_cq(3) failed with a provider error
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_cq_new(struct ibv_context *dev, int cqe, struct rpma_cq **cq_ptr);

/*
 * ERRORS
 * rpma_cq_delete() can fail with the following errors:
 *
 * - RPMA_E_PROVIDER - ibv_destroy_cq(3) or ibv_destroy_comp_channel(3)
 * failed with a provider error
 */
int rpma_cq_delete(struct rpma_cq **cq_ptr);

/*
 * ERRORS
 * rpma_cq_get_ibv_cq() cannot fail.
 */
struct ibv_cq *rpma_cq_get_ibv_cq(const struct rpma_cq *cq);

#endif /* LIBRPMA_CQ_H */
