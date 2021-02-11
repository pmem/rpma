/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2021, Intel Corporation */

/*
 * flush.h -- librpma flush-related internal definitions
 */

#ifndef LIBRPMA_FLUSH_H
#define LIBRPMA_FLUSH_H

#include "librpma.h"

struct rpma_flush;

typedef int (*rpma_flush_func)(struct ibv_qp *qp, struct rpma_flush *flush,
	struct rpma_mr_remote *dst, size_t dst_offset, size_t len,
	enum rpma_flush_type type, int flags, const void *op_context);

struct rpma_flush {
	rpma_flush_func func;
};

/*
 * ERRORS
 * rpma_flush_new() can fail with the following errors:
 *
 * - RPMA_E_NOMEM - out of memory (mmap() failed)
 * - RPMA_E_PROVIDER - sysconf() or ibv_reg_mr() failed
 */
int rpma_flush_new(struct rpma_peer *peer, struct rpma_flush **flush_ptr);

/*
 * ERRORS
 * rpma_flush_delete() can fail with the following error:
 *
 * - RPMA_E_PROVIDER - ibv_dereg_mr() failed
 * - RPMA_E_INVAL - munmap() failed
 */
int rpma_flush_delete(struct rpma_flush **flush_ptr);

#endif /* LIBRPMA_FLUSH_H */
