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

#define RPMA_E_UNKNOWN			(-100000) /* Unknown error */
#define RPMA_E_NOSUPP			(-100001) /* Not supported */
#define RPMA_E_PROVIDER			(-100002) /* Provider error occurred */
#define RPMA_E_NOMEM			(-100003) /* Out of memory */
#define RPMA_E_INVAL			(-100004) /* Invalid argument */
#define RPMA_E_NO_COMPLETION		(-100005) /* No completion available */

/* picking up an RDMA-capable device */

/* pick a type of an ibv_context to lookup for */
enum rpma_util_ibv_context_type {
	RPMA_UTIL_IBV_CONTEXT_LOCAL, /* lookup for a local device */
	RPMA_UTIL_IBV_CONTEXT_REMOTE /* lookup for a remote device */
};

/** 3
 * rpma_utils_get_ibv_context - obtain an RDMA device context by IP address
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_utils_get_ibv_context(const char *addr,
 *	        enum rpma_util_ibv_context_type type, struct ibv_context **dev);
 *
 * DESCRIPTION
 * rpma_utils_get_ibv_context() obtains an RDMA device context
 * by the given IPv4/IPv6 address (either local or remote) using
 * the TCP RDMA port space (RDMA_PS_TCP) - reliable, connection-oriented
 * and message based QP communication.
 *
 * RETURN VALUE
 * The rpma_utils_get_ibv_context() function returns 0 on success or a negative
 * error code on failure. rpma_utils_get_ibv_context() does not set *dev value
 * on failure.
 *
 * ERRORS
 * rpma_utils_get_ibv_context() can fail with the following errors:
 *
 * - RPMA_E_INVAL - addr or dev is NULL or type is unknown
 * - RPMA_E_NOMEM - out of memory
 * - RPMA_E_PROVIDER - rdma_getaddrinfo(), rdma_create_id(), rdma_bind_addr()
 *   or rdma_resolve_addr() failed, errno can be checked using
 *   rpma_err_get_provider_error()
 */
int rpma_utils_get_ibv_context(const char *addr,
		enum rpma_util_ibv_context_type type, struct ibv_context **dev);

/* peer */

struct rpma_peer;

/** 3
 * rpma_peer_new - create a peer object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_peer_new(struct ibv_context *ibv_ctx, struct rpma_peer **peer);
 *
 * DESCRIPTION
 * rpma_peer_new() creates a new peer object.
 *
 * RETURN VALUE
 * The rpma_peer_new() function returns 0 on success or a negative error code
 * on failure. rpma_peer_new() does not set *peer value on failure.
 *
 * ERRORS
 * rpma_peer_new() can fail with the following errors:
 *
 * - RPMA_E_INVAL - ibv_ctx or peer is NULL
 * - RPMA_E_NOMEM - creating a verbs protection domain failed with ENOMEM.
 * - RPMA_E_PROVIDER - creating a verbs protection domain failed with error
 *   other than ENOMEM.
 * - RPMA_E_UNKNOWN - creating a verbs protection domain failed without error
 *   value.
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_peer_new(struct ibv_context *ibv_ctx, struct rpma_peer **peer);

/** 3
 * rpma_peer_delete - delete a peer object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_peer_delete(struct rpma_peer **peer);
 *
 * RETURN VALUE
 * The rpma_peer_delete() function returns 0 on success or a negative error code
 * on failure. rpma_peer_delete() does not set *peer to NULL on failure.
 *
 * ERRORS
 * rpma_peer_delete() can fail with the following error:
 *
 * - RPMA_E_PROVIDER - deleting the verbs protection domain failed.
 */
int rpma_peer_delete(struct rpma_peer **peer);

/* memory-related structures */

struct rpma_mr_local;
struct rpma_mr_remote;

#define RPMA_MR_USAGE_READ_SRC	(1 << 0)
#define RPMA_MR_USAGE_READ_DST	(1 << 1)

enum rpma_mr_plt {
	RPMA_MR_PLT_VOLATILE, /* the region comes from volatile memory */
	RPMA_MR_PLT_PERSISTENT /* the region comes from persistent memory */
};

/** 3
 * rpma_mr_reg - create a local memory registration object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_mr_reg(struct rpma_peer *peer, void *ptr, size_t size,
 *		int usage, enum rpma_mr_plt plt, struct rpma_mr_local **mr_ptr);
 *
 * DESCRIPTION
 * rpma_mr_reg() registers a memory region and creates
 * a local memory registration object.
 *
 * ERRORS
 * rpma_mr_reg() can fail with the following errors:
 *
 * - RPMA_E_INVAL - peer or ptr or mr_ptr is NULL
 * - RPMA_E_INVAL - size equals 0
 * - RPMA_E_NOMEM - out of memory
 * - RPMA_E_PROVIDER - memory registration failed
 */
int rpma_mr_reg(struct rpma_peer *peer, void *ptr, size_t size,
		int usage, enum rpma_mr_plt plt, struct rpma_mr_local **mr_ptr);

/** 3
 * rpma_mr_dereg - delete a local memory registration object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_mr_dereg(struct rpma_mr_local **mr_ptr);
 *
 * DESCRIPTION
 * rpma_mr_dereg() deregisters a memory region
 * and deletes a local memory registration object.
 *
 * ERRORS
 * rpma_mr_dereg() can fail with the following error:
 *
 * - RPMA_E_INVAL - mr_ptr is NULL
 * - RPMA_E_PROVIDER - memory deregistration failed
 */
int rpma_mr_dereg(struct rpma_mr_local **mr_ptr);

/* The number of bytes required to store a description of a memory region */
#define RPMA_MR_DESCRIPTOR_SIZE 21

typedef struct {
	uint8_t data[RPMA_MR_DESCRIPTOR_SIZE];
} rpma_mr_descriptor;

/** 3
 * rpma_mr_get_descriptor - get a descriptor of a memory region
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_mr_get_descriptor(struct rpma_mr_local *mr,
 *	    struct rpma_mr_descriptor *desc);
 *
 * DESCRIPTION
 * rpma_mr_get_descriptor() writes a network-transferable description of
 * the provided local memory region. Once the descriptor is transferred to
 * the other side it can be consumed by rpma_mr_remote_from_descriptor() to
 * create a remote memory region's structure which allows transferring data
 * between the peers.
 *
 * ERRORS
 * rpma_mr_get_descriptor() can fail with the following error:
 *
 * - RPMA_E_INVAL - mr or desc is NULL
 */
int rpma_mr_get_descriptor(struct rpma_mr_local *mr, rpma_mr_descriptor *desc);

/** 3
 * rpma_mr_remote_from_descriptor - create a memory region from a descriptor
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_mr_remote_from_descriptor(
 *          const struct rpma_mr_descriptor *desc,
 *          struct rpma_mr_remote **mr_ptr);
 *
 * DESCRIPTION
 * Create a remote memory region's structure based on the provided descriptor
 * with a network-transferable description of the memory region local to
 * the remote peer.
 *
 * ERRORS
 * rpma_mr_remote_from_descriptor() can fail with the following errors:
 *
 * - RPMA_E_INVAL - desc or mr_ptr is NULL
 * - RPMA_E_NOSUPP - deserialized information does not represent a valid memory
 *   region
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_mr_remote_from_descriptor(const rpma_mr_descriptor *desc,
		struct rpma_mr_remote **mr_ptr);

/** 3
 * rpma_mr_remote_get_size - get a remote memory region size
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_mr_remote_get_size(struct rpma_mr_remote *mr, size_t *size);
 *
 * ERRORS
 * rpma_mr_remote_get_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - mr or size is NULL
 */
int rpma_mr_remote_get_size(struct rpma_mr_remote *mr, size_t *size);

/** 3
 * rpma_mr_remote_delete - delete a remote memory region's structure
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_mr_remote_delete(struct rpma_mr_remote **mr_ptr);
 *
 * ERRORS
 * rpma_mr_remote_delete() can fail with the following error:
 *
 * - RPMA_E_INVAL - mr_ptr is NULL
 */
int rpma_mr_remote_delete(struct rpma_mr_remote **mr_ptr);

/* connection */

struct rpma_conn;

enum rpma_conn_event {
	RPMA_CONN_UNDEFINED = -1,	/* Undefined connection event */
	RPMA_CONN_ESTABLISHED,		/* Connection established */
	RPMA_CONN_CLOSED,			/* Connection closed */
	RPMA_CONN_LOST				/* Connection lost */
};

/** 3
 * rpma_conn_next_event - obtain a connection status
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_next_event(struct rpma_conn *conn,
 *		enum rpma_conn_event *event);
 *
 * DESCRIPTION
 * Obtain the next event from the connection
 *
 * ERRORS
 * rpma_conn_next_event() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn or event is NULL
 * - RPMA_E_UNKNOWN - unexpected event
 * - RPMA_E_PROVIDER - rdma_get_cm_event() or rdma_ack_cm_event() failed
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_conn_next_event(struct rpma_conn *conn, enum rpma_conn_event *event);

/** 3
 * rpma_utils_conn_event_2str - convert RPMA_CONN_* enum to a string
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	const char *rpma_utils_conn_event_2str(enum rpma_conn_event conn_event);
 *
 * DESCRIPTION
 * Return const string representation of RPMA_CONN_* enums.
 *
 * ERRORS
 * rpma_utils_conn_event_2str() can not fail.
 */
const char *rpma_utils_conn_event_2str(enum rpma_conn_event conn_event);

struct rpma_conn_private_data {
	void *ptr;
	uint8_t len;
};

/** 3
 * rpma_conn_get_private_data - obtain a pointer to the connection's
 *                              private data
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_get_private_data(struct rpma_conn *conn,
 *	    struct rpma_conn_private_data *pdata);
 *
 * DESCRIPTION
 * Obtain a pointer to the private data given by the other side of the
 * connection.
 *
 * ERRORS
 * rpma_conn_get_private_data() can fail with the following error:
 *
 * - RPMA_E_INVAL - conn or pdata is NULL
 */
int rpma_conn_get_private_data(struct rpma_conn *conn,
		struct rpma_conn_private_data *pdata);

/** 3
 * rpma_conn_disconnect - initialize disconnection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_disconnect(struct rpma_conn *conn);
 *
 * DESCRIPTION
 * Initialize the RPMA connection disconnection process.
 *
 * ERRORS
 * rpma_conn_disconnect() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn is NULL
 * - RPMA_E_PROVIDER - rdma_disconnect() failed
 */
int rpma_conn_disconnect(struct rpma_conn *conn);

/** 3
 * rpma_conn_delete - delete already closed connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_delete(struct rpma_conn **conn);
 *
 * RETURN VALUE
 * The rpma_conn_delete() function returns 0 on success or a negative error code
 * on failure. rpma_conn_delete() sets *conn_ptr value to NULL on success and on
 * failure.
 *
 * ERRORS
 * rpma_conn_delete() can fail with the following errors:
 * - RPMA_E_INVAL - conn_ptr is NULL
 * - RPMA_E_PROVIDER - ibv_destroy_cq() or rdma_destroy_id() failed
 */
int rpma_conn_delete(struct rpma_conn **conn_ptr);

/* incoming / outgoing connection request */

struct rpma_conn_req;

/** 3
 * rpma_conn_req_new - create a new outgoing connection request object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_req_new(struct rpma_peer *peer, const char *addr,
 *		const char *service, struct rpma_conn_req **req_ptr);
 *
 * DESCRIPTION
 * Create a new outgoing connection request object.
 *
 * ERRORS
 * rpma_conn_req_new() can fail with the following errors:
 *
 * - RPMA_E_INVAL - peer, addr, service or req_ptr is NULL
 * - RPMA_E_NOMEM - out of memory
 * - RPMA_E_PROVIDER - rdma_create_id(3), rdma_resolve_addr(3),
 *   rdma_resolve_route(3) or ibv_create_cq(3) failed
 */
int rpma_conn_req_new(struct rpma_peer *peer, const char *addr,
	const char *service, struct rpma_conn_req **req_ptr);

/** 3
 * rpma_conn_req_delete - delete the connection request
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_req_delete(struct rpma_conn_req **req_ptr);
 *
 * DESCRIPTION
 * Delete the connection request both incoming and outgoing.
 *
 * ERRORS
 * rpma_conn_req_delete() can fail with the following errors:
 *
 * - RPMA_E_INVAL - req_ptr is NULL
 * - RPMA_E_PROVIDER
 *     - rdma_destroy_qp(3) or ibv_destroy_cq(3) failed
 *     - rdma_reject(3) or rdma_ack_cm_event(3) failed (passive side only)
 *     - rdma_destroy_id(3) failed (active side only)
 */
int rpma_conn_req_delete(struct rpma_conn_req **req_ptr);

/** 3
 * rpma_conn_req_connect - connect the connection request
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_req_connect(struct rpma_conn_req *req_ptr,
 *		struct rpma_conn_private_data *pdata,
 *		struct rpma_conn **conn_ptr);
 *
 * DESCRIPTION
 * Connect the connection requests both incoming and outgoing.
 *
 * RETURN VALUE
 * The rpma_conn_req_connect() function returns 0 on success or a negative error
 * code on failure. On success, the newly created connection object is stored in
 * *conn_ptr whereas *req_ptr is consumed and set to NULL. On failure,
 * rpma_conn_req_connect() does not set *conn_ptr whereas *req_ptr is consumed
 * and set to NULL.
 *
 * ERRORS
 * rpma_conn_req_connect() can fail with the following errors:
 *
 * - RPMA_E_INVAL - req_ptr, *req_ptr or conn_ptr is NULL
 * - RPMA_E_INVAL - pdata is not NULL whereas pdata->len == 0
 * - RPMA_E_NOMEM - out of memory
 * - RPMA_E_PROVIDER - initiating a connection request failed (active side only)
 * - RPMA_E_PROVIDER - accepting the connection request failed
 *                     (passive side only)
 * - RPMA_E_PROVIDER - freeing a communication event failed (passive side only)
 */
int rpma_conn_req_connect(struct rpma_conn_req **req_ptr,
	struct rpma_conn_private_data *pdata, struct rpma_conn **conn_ptr);

/* server-side setup */

struct rpma_ep;

/** 3
 * rpma_ep_listen - create a listening endpoint
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_ep_listen(const char *addr, const char *service,
 *	    struct rpma_ep **ep);
 *
 * DESCRIPTION
 * Create an endpoint and initialize listening for incoming connections.
 *
 * ERRORS
 * rpma_ep_listen() can fail with the following errors:
 *
 * - RPMA_E_INVAL - peer, addr, service or ep is NULL
 * - RPMA_E_PROVIDER - rdma_create_event_channel(3), rdma_create_id(3),
 *   rdma_getaddrinfo(3), rdma_listen(3) failed
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_ep_listen(struct rpma_peer *peer, const char *addr,
	const char *service, struct rpma_ep **ep);

/** 3
 * rpma_ep_shutdown - stop listening and delete the endpoint
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_ep_shutdown(struct rpma_ep **ep);
 *
 * DESCRIPTION
 * Stop listening for incoming connections and delete the endpoint.
 *
 * ERRORS
 * rpma_ep_shutdown() can fail with the following errors:
 *
 * - RPMA_E_INVAL - ep is NULL
 * - RPMA_E_PROVIDER - rdma_destroy_id(3) failed
 */
int rpma_ep_shutdown(struct rpma_ep **ep);

/** 3
 * rpma_ep_next_conn_req - obtain an incoming connection request
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_ep_next_conn_req(struct rpma_ep *ep,
 *	    struct rpma_conn_req **req);
 *
 * DESCRIPTION
 * Obtains the next connection request from the endpoint.
 *
 * ERRORS
 * rpma_ep_next_conn_req() can fail with the following errors:
 *
 * - RPMA_E_INVAL - ep or req is NULL
 * - RPMA_E_INVAL - obtained an event different than a connection request
 * - RPMA_E_PROVIDER - rdma_get_cm_event(3) failed
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_ep_next_conn_req(struct rpma_ep *ep, struct rpma_conn_req **req);

/* remote memory access functions */

/* generate operation completion on error */
#define RPMA_F_COMPLETION_ON_ERROR	(1 << 0)
/* generate operation completion regardless of its result */
#define RPMA_F_COMPLETION_ALWAYS	(1 << 1 | RPMA_F_COMPLETION_ON_ERROR)

/** 3
 * rpma_read - initialize the read operation
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_read(struct rpma_conn *conn,
 *		struct rpma_mr_local *dst, size_t dst_offset,
 *		struct rpma_mr_remote *src,  size_t src_offset,
 *		size_t len, int flags, void *op_context);
 *
 * DESCRIPTION
 * Initialize the read operation (transferring data from
 * the remote memory to the local memory).
 *
 * ERRORS
 * rpma_read() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn, dst or src is NULL
 * - RPMA_E_INVAL - flags are not set
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 */
int rpma_read(struct rpma_conn *conn,
	struct rpma_mr_local *dst, size_t dst_offset,
	struct rpma_mr_remote *src,  size_t src_offset,
	size_t len, int flags, void *op_context);

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
 * rpma_conn_next_completion - receive a completion of an operation
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_next_completion(struct rpma_conn *conn,
 *		struct rpma_completion *cmpl);
 *
 * DESCRIPTION
 * Receive the next available completion of an already posted operation. All
 * operations are generating completion on error. All operations posted with
 * the **RPMA_F_COMPLETION_ALWAYS** flag will also generate a completion on
 * success.
 *
 * ERRORS
 * rpma_conn_next_completion() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn or cmpl is NULL
 * - RPMA_E_PROVIDER - ibv_poll_cq(3) failed with a provider error
 * - RPMA_E_UNKNOWN - ibv_poll_cq(3) failed but no provider error is available
 * - RPMA_E_NOSUPP - not supported opcode
 */
int rpma_conn_next_completion(struct rpma_conn *conn,
	struct rpma_completion *cmpl);

/* error handling */

/** 3
 * rpma_err_get_provider_error - return the last provider error
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_err_get_provider_error(void);
 */
int rpma_err_get_provider_error(void);

/** 3
 * rpma_err_get_msg - return the last error message
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	const char *rpma_err_get_msg(void);
 *
 * DESCRIPTION
 * If an error is detected during the call to a librpma(7) function, the
 * application may retrieve an error message describing the reason of the
 * failure from rpma_err_get_msg(). The error message buffer is thread-local;
 * errors encountered in one thread do not affect its value in
 * other threads. The buffer is never cleared by any library function; its
 * content is significant only when the return value of the immediately
 * preceding call to a librpma(7) function indicated an error.
 * The application must not modify or free the error message string.
 * Subsequent calls to other library functions may modify the previous message.
 *
 * RETURN VALUE
 * The rpma_err_get_msg() function returns a pointer to a static buffer
 * containing the last error message logged for the current thread.
 *
 * SEE ALSO
 * librpma(7) and
 * .B <https://pmem.io>
 */
const char *rpma_err_get_msg(void);

/** 3
 * rpma_err_2str - convert RPMA error code to a string
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	const char *rpma_err_2str(int ret);
 *
 * DESCRIPTION
 * Return const string representation of RPMA error codes.
 *
 * ERRORS
 * rpma_err_2str() can not fail.
 */
const char *rpma_err_2str(int ret);

#endif /* LIBRPMA_H */
