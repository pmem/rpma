/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2022, Intel Corporation */
/* Copyright 2022, Fujitsu */

/*
 * mr.h -- librpma memory region-related internal definitions
 */

#ifndef LIBRPMA_MR_H
#define LIBRPMA_MR_H

#include "librpma.h"

#include <infiniband/verbs.h>

/*
 * ASSUMPTIONS
 * - qp != NULL && flags != 0
 * - (src != NULL && dst != NULL) ||
 *   (src == NULL && dst == NULL &&
 *    dst_offset == 0 && src_offset == 0 && len == 0)
 *
 * ERRORS
 * rpma_mr_read() can fail with the following error:
 *
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 */
int rpma_mr_read(struct ibv_qp *qp, struct rpma_mr_local *dst, size_t dst_offset,
	const struct rpma_mr_remote *src, size_t src_offset, size_t len, int flags,
	const void *op_context);

/*
 * ASSUMPTIONS
 * - qp != NULL && flags != 0
 * - (src != NULL && dst != NULL) ||
 *   (src == NULL && dst == NULL &&
 *    dst_offset == 0 && src_offset == 0 && len == 0)
 *
 * ERRORS
 * rpma_mr_write() can fail with the following errors:
 *
 * - RPMA_E_NOSUPP   - unsupported 'operation' argument
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 */
int rpma_mr_write(struct ibv_qp *qp, struct rpma_mr_remote *dst, size_t dst_offset,
	const struct rpma_mr_local *src, size_t src_offset, size_t len, int flags,
	enum ibv_wr_opcode operation, uint32_t imm, const void *op_context);

/*
 * ASSUMPTIONS
 * - qp != NULL && dst != NULL && src != NULL && flags != 0
 *
 * ERRORS
 * rpma_mr_atomic_write() can fail with the following error:
 *
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 */
int rpma_mr_atomic_write(struct ibv_qp *qp, struct rpma_mr_remote *dst, size_t dst_offset,
	const char src[8], int flags, const void *op_context);

/*
 * ASSUMPTIONS
 * - qp != NULL && flags != 0
 * - src != NULL || (offset == 0 && len == 0)
 *
 * ERRORS
 * rpma_mr_send() can fail with the following error:
 *
 * - RPMA_E_NOSUPP   - unsupported 'operation' argument
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 */
int rpma_mr_send(struct ibv_qp *qp, const struct rpma_mr_local *src, size_t offset, size_t len,
	int flags, enum ibv_wr_opcode operation, uint32_t imm, const void *op_context);

/*
 * ASSUMPTIONS
 * - qp != NULL
 * - dst != NULL || (offset == 0 && len == 0)
 *
 * ERRORS
 * rpma_mr_recv() can fail with the following error:
 *
 * - RPMA_E_PROVIDER - ibv_post_recv(3) failed
 */
int rpma_mr_recv(struct ibv_qp *qp, struct rpma_mr_local *dst, size_t offset, size_t len,
	const void *op_context);

/*
 * ASSUMPTIONS
 * - srq != NULL
 * - dst != NULL || (offset == 0 && len == 0)
 *
 * ERRORS
 * rpma_mr_srq_recv() can fail with the following error:
 *
 * - RPMA_E_PROVIDER - ibv_post_srq_recv(3) failed
 */
int rpma_mr_srq_recv(struct ibv_srq *ibv_srq, struct rpma_mr_local *dst, size_t offset, size_t len,
	const void *op_context);

#endif /* LIBRPMA_MR_H */
