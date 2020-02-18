/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * rma.h -- base definitions of librpma RMA entry points (EXPERIMENTAL)
 *
 * This library provides low-level support for remote access to persistent
 * memory utilizing RDMA-capable RNICs.
 *
 * See librpma(7) for details.
 */

#ifndef LIBRPMA_RMA_H
#define LIBRPMA_RMA_H 1

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <base.h>

/* local memory region */

struct rpma_memory_local;

#define RPMA_MR_READ_SRC (1 << 0)
#define RPMA_MR_READ_DST (1 << 1)
#define RPMA_MR_WRITE_SRC (1 << 2)
#define RPMA_MR_WRITE_DST (1 << 3)

int rpma_memory_local_new(struct rpma_zone *zone, void *ptr, size_t size,
		int usage, struct rpma_memory_local **mem);

int rpma_memory_local_get_ptr(struct rpma_memory_local *mem, void **ptr);

int rpma_memory_local_get_size(struct rpma_memory_local *mem, size_t *size);

struct rpma_memory_id {
	uint64_t data[4];
};

int rpma_memory_local_get_id(struct rpma_memory_local *mem,
		struct rpma_memory_id *id);

int rpma_memory_local_delete(struct rpma_memory_local **mem);

/* remote memory region */

struct rpma_memory_remote;

int rpma_memory_remote_new(struct rpma_zone *zone, struct rpma_memory_id *id,
		struct rpma_memory_remote **rmem);

int rpma_memory_remote_get_size(struct rpma_memory_remote *rmem, size_t *size);

int rpma_memory_remote_delete(struct rpma_memory_remote **rmem);

/* remote memory access commands */

int rpma_connection_read(struct rpma_connection *conn,
		struct rpma_memory_local *dst, size_t dst_off,
		struct rpma_memory_remote *src, size_t src_off, size_t length);

int rpma_connection_write(struct rpma_connection *conn,
		struct rpma_memory_remote *dst, size_t dst_off,
		struct rpma_memory_local *src, size_t src_off, size_t length);

int rpma_connection_atomic_write(struct rpma_connection *conn,
		struct rpma_memory_remote *dst, size_t dst_off,
		struct rpma_memory_local *src, size_t src_off, size_t length);

int rpma_connection_commit(struct rpma_connection *conn);

#ifdef __cplusplus
}
#endif
#endif /* rma.h */
