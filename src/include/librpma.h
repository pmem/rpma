/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2019-2020, Intel Corporation
 */

/*
 * librpma.h -- definitions of librpma entry points (EXPERIMENTAL)
 *
 * This library provides low-level support for remote access to persistent
 * memory utilizing RDMA-capable NICs.
 *
 * See librpma(7) for details.
 */

#ifndef LIBRPMA_H
#define LIBRPMA_H 1

#include <stddef.h>
#include <stdint.h>

#include <infiniband/verbs.h>

#define RPMA_WAIT_FOR_COMPLETION	(      1)

#define RPMA_E_UNKNOWN			(-100000)
#define RPMA_E_NOSUPP			(-100001)

/* picking up an RDMA-capable device */

int rpma_utils_get_ibv_context(const char *addr, struct ibv_context **dev);

/* peer */

struct rpma_peer_cfg;
struct rpma_peer;

int rpma_peer_new(struct rpma_peer_cfg *pcfg, struct ibv_context *dev,
		struct rpma_peer **peer);

int rpma_peer_delete(struct rpma_peer **peer);

/* memory description structures */

struct rpma_memory_local_handle;
struct rpma_memory_remote_handle;

#define RPMA_MR_READ_SRC	(1 << 0)
#define RPMA_MR_READ_DST	(1 << 1)

int rpma_memory_new(struct rpma_peer *peer, void *ptr, size_t size,
		int usage, int plt, struct rpma_memory_local_handle **mem);

int rpma_memory_delete(struct rpma_memory_local_handle **mem);

/* connection */

struct rpma_conn_cfg;
struct rpma_conn;

#define RPMA_CONNECTION_CLOSED	1
#define RPMA_CONNECTION_LOST	2

int rpma_conn_get_status(struct rpma_conn *conn, int *conn_status);

int rpma_conn_get_mem(struct rpma_conn *conn,
	struct rpma_memory_remote_handle **mem);

int rpma_disconnect(struct rpma_conn **conn);

/* server-side setup */

struct rpma_socket;

int rpma_listen(struct rpma_peer *peer, const char *addr, const char *service,
	struct rpma_socket **socket);

#define RPMA_CONNECTION_REQUESTED	1
#define RPMA_CONNECTION_ESTABLISHED	2

int rpma_socket_read(struct rpma_socket *socket, struct rpma_conn **conn,
	int *conn_status);

int rpma_accept(struct rpma_conn *conn, struct rpma_conn_cfg *ccfg,
	struct rpma_memory_local_handle *mem);

int rpma_socket_delete(struct rpma_socket **socket);

/* client-side setup */

int rpma_conn_setup(struct rpma_peer *peer, const char *addr,
	const char *service, struct rpma_conn **conn);

int rpma_connect(struct rpma_conn *conn, struct rpma_conn_cfg *ccfg,
	struct rpma_memory_local_handle *mem);

/* remote memory access functions */

int rpma_read(struct rpma_conn *conn, void *op_context,
	struct rpma_memory_local_handle *dst, size_t dst_offset,
	struct rpma_memory_remote_handle *src,  size_t src_offset,
	size_t len, int flags);

/* completion handling */

#define RPMA_OP_READ	(1 << 3)

int rpma_next_completion(struct rpma_conn *conn, void **op_context, int *op,
	int *status);

/* error handling */

const char *rpma_errormsg(void);

#endif /* LIBRPMA_H */
