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

#define RPMA_W_WAIT_FOR_COMPLETION	(1)

#define RPMA_E_UNKNOWN			(-100000)
#define RPMA_E_NOSUPP			(-100001)
#define RPMA_E_PROVIDER			(-100002)

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
 * rpma_mr_reg, rpma_mr_dereg - create and delete a local memory handle object
 */
int rpma_mr_reg(struct rpma_peer *peer, void *ptr, size_t size,
		int usage, int plt, struct rpma_mr_local **mr);

int rpma_mr_dereg(struct rpma_mr_local **mr);

/* connection */

struct rpma_conn;

enum rpma_conn_event {
	RPMA_CONN_ESTABLISHED,
	RPMA_CONN_CLOSED,
	RPMA_CONN_LOST
};

/** 3
 * rpma_conn_next_event - obtain a connection status
 */
int rpma_conn_next_event(struct rpma_conn *conn, enum rpma_conn_event *event);

/** 3
 * rpma_conn_get_mr - obtain a remote handle to the memory given by the other
 * side of the connection
 */
int rpma_conn_get_mr(struct rpma_conn *conn, struct rpma_mr_remote **mr);

/** 3
 * rpma_conn_disconnect - initialize disconnection
 */
int rpma_conn_disconnect(struct rpma_conn *conn);

/** 3
 * rpma_conn_delete - delete already closed connection
 */
int rpma_conn_delete(struct rpma_conn **conn);

/* incoming / outgoing connection request */

struct rpma_conn_cfg;
struct rpma_conn_req;

/** 3
 * rpma_conn_req_new - create a new outgoing connection request object
 */
int rpma_conn_req_new(struct rpma_peer *peer, const char *addr,
	const char *service, struct rpma_conn_req **req);

/** 3
 * rpma_conn_req_delete - delete the connection request both incoming and
 * outgoing
 */
int rpma_conn_req_delete(struct rpma_conn_req **req);

/** 3
 * rpma_conn_req_connect - connect the connection request both incoming and
 * outgoing
 */
int rpma_conn_req_connect(struct rpma_conn_req *req, struct rpma_conn_cfg *ccfg,
	struct rpma_mr_local *mr, struct rpma_conn **conn);

/* server-side setup */

struct rpma_ep;

/** 3
 * rpma_ep_listen, rpma_ep_shutdown - create an endpoint and initialize
 * listening for the incoming connections / stop listening and delete an
 * endpoint
 */
int rpma_ep_listen(struct rpma_peer *peer, const char *addr,
	const char *service, struct rpma_ep **ep);

int rpma_ep_shutdown(struct rpma_ep **ep);

/** 3
 * rpma_ep_next_conn_req - obtain an incoming connection request
 */
int rpma_ep_next_conn_req(struct rpma_ep *ep, struct rpma_conn_req **req);

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

enum rpma_op {
	RPMA_OP_READ,
};

struct rpma_completion {
	void *op_context;
	enum rpma_op op;
	enum ibv_wc_status op_status;
};

/** 3
 * rpma_conn_next_completion - obtain an operation completion
 */
int rpma_conn_next_completion(struct rpma_conn *conn,
	struct rpma_completion *cmpl);

/* error handling */

/** 3
 * rpma_err_get_provider_error - return the last provider error
 */
int rpma_err_get_provider_error(void);

/** 3
 * rpma_err_get_msg - return the last error message
 */
const char *rpma_err_get_msg(void);

#endif /* LIBRPMA_H */
