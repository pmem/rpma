/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mr.h -- librpma memory region-related internal definitions
 */

#ifndef LIBRPMA_MR_H
#define LIBRPMA_MR_H

#include "librpma.h"

#include <infiniband/verbs.h>

/*
 * ASSUMPTIONS
 * - qp != NULL && dst != NULL && src != NULL
 *
 * ERRORS
 * rpma_mr_read() can fail with the following error:
 *
 * - RPMA_E_PROVIDER - allocating a QP failed
 */
int rpma_mr_read(struct ibv_qp *qp, void *op_context,
	struct rpma_mr_local *dst, size_t dst_offset,
	struct rpma_mr_remote *src,  size_t src_offset,
	size_t len, int flags);

#endif /* LIBRPMA_MR_H */
