// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * flush.c -- librpma flush-related implementations
 */

#include <infiniband/verbs.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

#include "flush.h"
#include "log_internal.h"
#include "mr.h"
#include "rpma_err.h"

static int rpma_flush_apm_new(struct rpma_peer *peer, struct rpma_flush *flush);
static int rpma_flush_apm_delete(struct rpma_flush *flush);
static int rpma_flush_apm_do(struct ibv_qp *qp, struct rpma_flush *flush,
	struct rpma_mr_remote *dst, size_t dst_offset,
	size_t len, enum rpma_flush_type type, int flags, void *op_context);

typedef int (*rpma_flush_delete_func)(struct rpma_flush *flush);

struct rpma_flush_internal {
	rpma_flush_func flush_func;
	rpma_flush_delete_func delete_func;
	void *context;
};

/*
 * Appliance Persistency Method (APM) implementation of the flush operation
 * using Read-after-Write (RAW) technique for flushing intermediate buffers.
 */

struct flush_apm {
	void *raw; /* buffer for read-after-write memory region */
	struct rpma_mr_local *raw_mr; /* read-after-write memory region */
};

#define RAW_SIZE 8 /* read-after-write memory region size */

/*
 * rpma_flush_apm_new -- allocate a RAW buffer and register it
 */
static int
rpma_flush_apm_new(struct rpma_peer *peer, struct rpma_flush *flush)
{
	/* a memory registration has to be page-aligned */
	long pagesize = sysconf(_SC_PAGESIZE);
	if (pagesize < 0) {
		Rpma_provider_error = errno;
		RPMA_LOG_FATAL("sysconf(_SC_PAGESIZE) failed: %s",
				strerror(errno));
		return RPMA_E_PROVIDER;
	}

	/* allocate memory for the read-after-write buffer (RAW) */
	void *raw = NULL;
	int ret = posix_memalign(&raw, (size_t)pagesize, RAW_SIZE);
	if (ret)
		return RPMA_E_NOMEM;

	/* register the RAW buffer */
	struct rpma_mr_local *raw_mr = NULL;
	ret = rpma_mr_reg(peer, raw, RAW_SIZE,
		RPMA_MR_USAGE_READ_DST | RPMA_MR_USAGE_FLUSHABLE_VISIBILITY,
		&raw_mr);
	if (ret) {
		free(raw);
		return ret;
	}

	struct flush_apm *flush_apm = malloc(sizeof(struct flush_apm));
	if (flush_apm == NULL) {
		(void) rpma_mr_dereg(&raw_mr);
		free(raw);
		return RPMA_E_NOMEM;
	}

	flush_apm->raw = raw;
	flush_apm->raw_mr = raw_mr;

	struct rpma_flush_internal *flush_internal =
			(struct rpma_flush_internal *)flush;
	flush_internal->flush_func = rpma_flush_apm_do;
	flush_internal->delete_func = rpma_flush_apm_delete;
	flush_internal->context = flush_apm;

	return 0;
}

/*
 * rpma_flush_apm_delete -- unregister the RAW buffer and deallocate it
 */
static int
rpma_flush_apm_delete(struct rpma_flush *flush)
{
	struct rpma_flush_internal *flush_internal =
			(struct rpma_flush_internal *)flush;
	struct flush_apm *flush_apm =
			(struct flush_apm *)flush_internal->context;
	int ret = rpma_mr_dereg(&flush_apm->raw_mr);

	free(flush_apm->raw);
	free(flush_apm);

	return ret;
}

/*
 * rpma_flush_apm_do -- perform the APM-style flush
 */
static int
rpma_flush_apm_do(struct ibv_qp *qp, struct rpma_flush *flush,
	struct rpma_mr_remote *dst, size_t dst_offset,
	size_t len, enum rpma_flush_type type, int flags, void *op_context)
{
	struct rpma_flush_internal *flush_internal =
			(struct rpma_flush_internal *)flush;
	struct flush_apm *flush_apm =
			(struct flush_apm *)flush_internal->context;

	return rpma_mr_read(qp, flush_apm->raw_mr, 0, dst, dst_offset,
			RAW_SIZE, RPMA_F_COMPLETION_ALWAYS, op_context);
}

/* internal librpma API */

/*
 * rpma_flush_new -- peak a flush implementation and return the flushing object
 */
int
rpma_flush_new(struct rpma_peer *peer, struct rpma_flush **flush_ptr)
{
	struct rpma_flush *flush = malloc(sizeof(struct rpma_flush_internal));
	if (!flush)
		return RPMA_E_NOMEM;

	int ret = rpma_flush_apm_new(peer, flush);
	if (ret) {
		free(flush);
		return ret;
	}

	*flush_ptr = flush;

	return 0;
}

/*
 * rpma_flush_delete -- delete the flushing object
 */
int
rpma_flush_delete(struct rpma_flush **flush_ptr)
{
	struct rpma_flush_internal *flush_internal =
			*(struct rpma_flush_internal **)flush_ptr;

	int ret = flush_internal->delete_func(*flush_ptr);
	free(*flush_ptr);
	*flush_ptr = NULL;

	return ret;
}
