/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mocks-rpma-flush.h -- a librpma_flush mocks header
 */
#include <librpma.h>
struct rpma_flush;

typedef int (*rpma_flush_func)(struct ibv_qp *qp, struct rpma_flush *flush,
	struct rpma_mr_remote *dst, size_t dst_offset, size_t len,
	enum rpma_flush_type type, int flags, void *op_context);

struct rpma_flush {
	rpma_flush_func func;
};

extern struct rpma_flush Rpma_flush;
#define MOCK_FLUSH &Rpma_flush
