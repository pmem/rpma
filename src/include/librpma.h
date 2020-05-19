/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2019-2020, Intel Corporation
 */

/*
 * librpma.h -- definitions of librpma entry points (EXPERIMENTAL)
 *
 * This library provides low-level support for remote access to persistent
 * memory utilizing RDMA-capable NICs.
 */

#ifndef LIBRPMA_H
#define LIBRPMA_H 1

#include <stddef.h>
#include <stdint.h>

#include <infiniband/verbs.h>

/** 7
 * librpma - remote persistent memory support library
 */

#define RPMA_WAIT_FOR_COMPLETION	(1)

#define RPMA_E_UNKNOWN			(-100000)
#define RPMA_E_NOSUPP			(-100001)

/* picking up an RDMA-capable device */

/** 3
 * rpma_utils_get_ibv_context - obtain an RDMA device context by IP address
 */
int rpma_utils_get_ibv_context(const char *addr, struct ibv_context **dev);

/* peer */

struct rpma_peer_cfg;
struct rpma_peer;

/** 3
 * rpma_peer_new, rpma_peer_delete - create and delete a peer object
 */
int rpma_peer_new(struct rpma_peer_cfg *pcfg, struct ibv_context *dev,
		struct rpma_peer **peer);

int rpma_peer_delete(struct rpma_peer **peer);

/* memory description structures */

struct rpma_mr_local;
struct rpma_mr_remote;

#define RPMA_MR_USAGE_READ_SRC	(1 << 0)
#define RPMA_MR_USAGE_READ_DST	(1 << 1)

/** 3
 * rpma_memory_new, rpma_memory_delete - create and delete a local memory handle
 * object
 */
int rpma_mr_reg(struct rpma_peer *peer, void *ptr, size_t size,
		int usage, int plt, struct rpma_mr_local **mem);

int rpma_mr_dereg(struct rpma_mr_local **mem);

/* connection */

struct rpma_conn_cfg;
struct rpma_conn;

#define RPMA_CONNECTION_CLOSED	1
#define RPMA_CONNECTION_LOST	2

/** 3
 * rpma_conn_get_status - obtain a connection status
 */
int rpma_conn_get_status(struct rpma_conn *conn, int *conn_status);

/** 3
 * rpma_conn_get_mem - obtain a remote handle to the memory given by the other
 * side of the connection
 */
int rpma_conn_get_mem(struct rpma_conn *conn, struct rpma_mr_remote **mem);

/** 3
 * rpma_disconnect - initialize disconnection
 */
int rpma_disconnect(struct rpma_conn *conn);

/** 3
 * rpma_conn_delete - delete already closed connection
 */
int rpma_conn_delete(struct rpma_conn *conn);

/* server-side setup */

struct rpma_ep;

/** 3
 * rpma_listen, rpma_shutdown - initialize and finalize listening for
 * the incoming connections
 */
int rpma_listen(struct rpma_peer *peer, const char *addr, const char *service,
	struct rpma_ep **ep);

int rpma_shutdown(struct rpma_ep **ep);

enum rpma_conn_status {
	RPMA_CONNECTION_REQUESTED,
	RPMA_CONNECTION_ESTABLISHED
};

/** 3
 * rpma_ep_next_event - obtain a status change of the incoming connection
 */
int rpma_ep_next_event(struct rpma_ep *ep, struct rpma_conn **conn,
	enum rpma_conn_status *status);

/** 3
 * rpma_accept - accept a connection request
 */
int rpma_accept(struct rpma_conn *conn, struct rpma_conn_cfg *ccfg,
	struct rpma_mr_local *mem);

/* client-side setup */

/** 3
 * rpma_conn_new - create a new connection object
 */
int rpma_conn_new(struct rpma_peer *peer, const char *addr,
	const char *service, struct rpma_conn **conn);

/** 3
 * rpma_connect - initialize establishing a new connection
 */
int rpma_connect(struct rpma_conn *conn, struct rpma_conn_cfg *ccfg,
	struct rpma_mr_local *mem);

/* remote memory access functions */

/** 3
 * rpma_read - initialize a read operation (transferring data from
 * the remote memory to the local memory)
 */
int rpma_read(struct rpma_conn *conn, void *op_context,
	struct rpma_mr_local *dst, size_t dst_offset,
	struct rpma_mr_remote *src,  size_t src_offset,
	size_t len, int flags);

/* completion handling */

enum rpma_cmpl {
	RPMA_CMPL_READ,
};

/** 3
 * rpma_next_completion - obtain an operation completion
 */
int rpma_next_completion(struct rpma_conn *conn, void **op_context,
	enum rpma_cmpl *cmpl, int *status);

/* error handling */

/** 3
 * rpma_errormsg - return the last error message
 */
const char *rpma_errormsg(void);

#endif /* LIBRPMA_H */
