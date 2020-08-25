/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * mr.h -- librpma memory region-related internal definitions
 */

#ifndef LIBRPMA_MR_H
#define LIBRPMA_MR_H

#include "librpma.h"

#include <infiniband/verbs.h>

/*
 * ASSUMPTIONS
 * - qp != NULL && dst != NULL && src != NULL && flags != 0
 *
 * ERRORS
 * rpma_mr_read() can fail with the following error:
 *
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 */
int rpma_mr_read(struct ibv_qp *qp,
	struct rpma_mr_local *dst, size_t dst_offset,
	struct rpma_mr_remote *src,  size_t src_offset,
	size_t len, int flags, void *op_context);

/*
 * ASSUMPTIONS
 * - qp != NULL && dst != NULL && src != NULL && flags != 0
 *
 * ERRORS
 * rpma_mr_write() can fail with the following error:
 *
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 */
int rpma_mr_write(struct ibv_qp *qp,
	struct rpma_mr_remote *dst, size_t dst_offset,
	struct rpma_mr_local *src,  size_t src_offset,
	size_t len, int flags, void *op_context);

/*
 * ASSUMPTIONS
 * - qp != NULL && src != NULL && flags != 0
 *
 * ERRORS
 * rpma_mr_send() can fail with the following error:
 *
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 */
int rpma_mr_send(struct ibv_qp *qp,
	struct rpma_mr_local *src,  size_t offset,
	size_t len, int flags, void *op_context);

/*
 * ASSUMPTIONS
 * - qp != NULL && dst != NULL && flags != 0
 *
 * ERRORS
 * rpma_mr_recv() can fail with the following error:
 *
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 */
int rpma_mr_recv(struct ibv_qp *qp,
	struct rpma_mr_local *dst,  size_t offset,
	size_t len, int flags, void *op_context);

#endif /* LIBRPMA_MR_H */
