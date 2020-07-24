/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * flush.c -- librpma flush-related implementations
 */

#include <infiniband/verbs.h>
#include <stddef.h>

#include "flush.h"
#include "mr.h"

typedef int (*rpma_flush_delete_func)(struct rpma_flush *flush);

struct rpma_flush_internal {
	rpma_flush_func flush_func;
	rpma_flush_delete_func delete_func;
	void *context;
};

/*
 * Appliance Persistency Method (APM) implementation of a flush using
 * Read-after-Write (RAW) technique for flushing intermediate buffers.
 */

/*
 * rpma_flush_apm_new -- allocate a RAW buffer and register it
 *
 * XXX use rpma_peer_mr_reg()
 * XXX store the RAW buffer region in the rpma_flush context
 */
static int
rpma_flush_apm_new(struct rpma_peer *peer, struct rpma_flush *flush)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_flush_apm_do -- perform the flush APM-style
 *
 * XXX rpma_read() from dst to RAW buffer
 */
static int
rpma_flush_apm_do(struct ibv_qp *qp, struct rpma_flush *flush,
	struct rpma_mr_remote *dst, size_t dst_offset, size_t len,
	enum rpma_flush_type type, int flags, void *op_context)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_flush_apm_delete -- unregister the RAW buffer and deallocate it
 */
static int
rpma_flush_apm_delete(struct rpma_flush *flush)
{
	return RPMA_E_NOSUPP;
}

/* internal librpma API */

/*
 * rpma_flush_new -- peak a flush implementation and return the flushing object
 *
 * XXX allocate the flush object, fill the pointers and call the appropriate
 * constructor. For now the only implementation is APM.
 */
int
rpma_flush_new(struct rpma_peer *peer, struct rpma_flush **flush_ptr)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_flush_delete -- delete the flushing object
 *
 * XXX call the destructor and free the object
 */
int
rpma_flush_delete(struct rpma_flush **flush_ptr)
{
	return RPMA_E_NOSUPP;
}
