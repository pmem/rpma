/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * flush.c -- librpma flush-related implementations
 */

#include <infiniband/verbs.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#include "flush.h"
#include "mr.h"

#define RAW_SIZE 8

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
 * rpma_flush_apm_do -- perform the flush APM-style
 */
static int
rpma_flush_apm_do(struct ibv_qp *qp, struct rpma_flush *flush,
	struct rpma_mr_remote *dst, size_t dst_offset,
	size_t len, enum rpma_flush_type type, int flags, void *op_context)
{
	struct rpma_flush_internal *flush_internal = (struct rpma_flush_internal *)flush;

	return rpma_mr_read(qp, flush_internal->context, 0, dst, dst_offset, RAW_SIZE,
		RPMA_F_COMPLETION_ALWAYS, NULL);
}

/*
 * rpma_flush_apm_delete -- unregister the RAW buffer and deallocate it
 */
static int
rpma_flush_apm_delete(struct rpma_flush *flush)
{
	struct rpma_flush_internal *flush_internal = (struct rpma_flush_internal *)flush;
	struct rpma_mr_local *raw_mr = flush_internal->context;

	return rpma_mr_dereg(&raw_mr);
}

/*
 * rpma_flush_apm_new -- allocate a RAW buffer and register it
 */
static int
rpma_flush_apm_new(struct rpma_peer *peer, struct rpma_flush *flush)
{
	/* alloc memory for the read-after-write buffer (RAW) */
	long pagesize = sysconf(_SC_PAGESIZE);
	if (pagesize < 0) {
		return 0;
	}

	/* allocate a page size aligned local memory pool */
	void *raw;
	int ret = posix_memalign(&raw, (size_t)pagesize, RAW_SIZE);
	if (ret) {
		return 0;
	}

	struct rpma_mr_local *raw_mr = NULL;

	/* register the RAW buffer */
	ret = rpma_mr_reg(peer, raw, RAW_SIZE, RPMA_MR_USAGE_READ_DST,
			RPMA_MR_PLT_VOLATILE, &raw_mr);
	if (ret)
		return ret;

	struct rpma_flush_internal *flush_internal = (struct rpma_flush_internal *)flush;
	flush_internal->flush_func = rpma_flush_apm_do;
	flush_internal->delete_func = rpma_flush_apm_delete;
	flush_internal->context = raw_mr;
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
	struct rpma_flush *flush = malloc(sizeof(struct rpma_flush_internal));
	if (!flush){
		int ret = RPMA_E_NOMEM;
	}
	int ret = rpma_flush_apm_new(peer, flush);
	if (ret)
		return ret;
	*flush_ptr = flush;
}

/*
 * rpma_flush_delete -- delete the flushing object
 *
 * XXX call the destructor and free the object
 */
int
rpma_flush_delete(struct rpma_flush **flush_ptr)
{
	int ret = rpma_flush_apm_delete(*flush_ptr);
	free(flush_ptr);
}
