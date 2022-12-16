/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2022, Fujitsu */
/* Copyright (c) 2022 Fujitsu Limited */

/*
 * utils.h -- the internal definitions of generic helper functions
 */

#ifndef LIBRPMA_UTILS_H
#define LIBRPMA_UTILS_H

#include "librpma.h"

#define RPMA_ATOMIC_WRITE_CAP_FLAG	(1UL << 40)

/*
 * ERRORS
 * rpma_utils_ibv_context_is_atomic_write_capable() can fail with the following error:
 *
 * - RPMA_E_PROVIDER - ibv_query_device_ex() failed
 *
 * ASSUMPTIONS
 * ibv_ctx != NULL && is_atomic_write_capable != NULL
 */
int rpma_utils_ibv_context_is_atomic_write_capable(struct ibv_context *ibv_ctx,
		int *is_atomic_write_capable);

#endif /* LIBRPMA_UTILS_H */
