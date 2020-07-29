/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mocks-rpma-flush.c -- librpma flush.c module mocks
 */

#include <rdma/rdma_cma.h>
#include <librpma.h>

#include "rpma_err.h"
#include "cmocka_headers.h"
#include "test-common.h"

struct rpma_flush;

typedef int (*rpma_flush_func)(struct ibv_qp *qp, struct rpma_flush *flush,
	struct rpma_mr_remote *dst, size_t dst_offset, size_t len,
	enum rpma_flush_type type, int flags, void *op_context);

struct rpma_flush {
	rpma_flush_func func;
};

/*
 * rpma_flush_new -- rpma_flush_new() mock
 */
int
rpma_flush_new(struct rpma_peer *peer, struct rpma_flush **flush_ptr)
{
	assert_non_null(flush_ptr);

	/* allocate (struct rpma_flush *) */
	*flush_ptr = mock_type(struct rpma_flush *);
	if (*flush_ptr == NULL) {
		errno = mock_type(int);
		return -1;
	}

	return 0;
}

/*
 * rpma_flush_delete -- rpma_flush_delete() mock
 */
int
rpma_flush_delete(struct rpma_flush **flush_ptr)
{
	assert_non_null(flush_ptr);
	return 0;
}
