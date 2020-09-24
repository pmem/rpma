/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2019-2020, Intel Corporation */

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
#include <stdio.h>
#include <stdbool.h>

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
#define RPMA_E_NO_NEXT			(-100006) /* No next event available */

/* picking up an RDMA-capable device */

#define RPMA_DEFAULT_TIMEOUT_MS 1000

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
 *	struct ibv_context;
 *	enum rpma_util_ibv_context_type {
 *		RPMA_UTIL_IBV_CONTEXT_LOCAL,
 *		RPMA_UTIL_IBV_CONTEXT_REMOTE
 *	};
 *
 *	int rpma_utils_get_ibv_context(const char *addr,
 *		enum rpma_util_ibv_context_type type, struct ibv_context **dev);
 *
 * DESCRIPTION
 * rpma_utils_get_ibv_context() obtains an RDMA device context
 * by the given IPv4/IPv6 address (either local or remote) using
 * the TCP RDMA port space (RDMA_PS_TCP) - reliable, connection-oriented
 * and message-based QP communication.
 * Possible values of the 'type' argument:
 * - RPMA_UTIL_IBV_CONTEXT_LOCAL - lookup for a device
 * based on the given local address
 * - RPMA_UTIL_IBV_CONTEXT_REMOTE - lookup for a device
 * based on the given remote address
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

/** 3
 * rpma_utils_ibv_context_is_odp_capable - is On-Demand Paging supported
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct ibv_context;
 *	int rpma_utils_ibv_context_is_odp_capable(struct ibv_context *dev,
 *		int *is_odp_capable);
 *
 * DESCRIPTION
 * rpma_utils_ibv_context_is_odp_capable() queries the RDMA device context's
 * capabilities and check if it supports On-Demand Paging.
 *
 * RETURN VALUE
 * The rpma_utils_ibv_context_is_odp_capable() function returns 0 on success or
 * a negative error code on failure. The *is_odp_capable value on failure is
 * undefined.
 *
 * ERRORS
 * rpma_utils_ibv_context_is_odp_capable() can fail with the following errors:
 *
 * - RPMA_E_INVAL - dev or is_odp_capable is NULL
 * - RPMA_E_PROVIDER - ibv_query_device_ex() failed, errno can be checked using
 *   rpma_err_get_provider_error()
 */
int rpma_utils_ibv_context_is_odp_capable(struct ibv_context *dev,
		int *is_odp_capable);

/* peer configuration */

struct rpma_peer_cfg;

/** 3
 * rpma_peer_cfg_new - create a new peer configuration object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_peer_cfg;
 *	int rpma_peer_cfg_new(struct rpma_peer_cfg **pcfg_ptr);
 *
 * DESCRIPTION
 * rpma_peer_cfg_new() creates a new peer configuration object.
 *
 * RETURN VALUE
 * The rpma_peer_cfg_new() function returns 0 on success or a negative
 * error code on failure. rpm_peer_cfg_new() does not set
 * *pcfg_ptr value on failure.
 *
 * ERRORS
 * rpma_peer_cfg_new() can fail with the following errors:
 *
 * - RPMA_E_INVAL - pcfg_ptr is NULL
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_peer_cfg_new(struct rpma_peer_cfg **pcfg_ptr);

/** 3
 * rpma_peer_cfg_delete - delete the peer configuration object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_peer_cfg;
 *	int rpma_peer_cfg_delete(struct rpma_peer_cfg **pcfg_ptr);
 *
 * DESCRIPTION
 * rpma_peer_cfg_delete() deletes the peer configuration object.
 *
 * RETURN VALUE
 * The rpma_peer_cfg_delete() function returns 0 on success or a negative
 * error code on failure. rpm_peer_cfg_delete() does not set
 * *pcfg_ptr value to NULL on failure.
 *
 * ERRORS
 * rpma_peer_cfg_delete() can fail with the following error:
 *
 * - RPMA_E_INVAL - pcfg_ptr is NULL
 */
int rpma_peer_cfg_delete(struct rpma_peer_cfg **pcfg_ptr);

/** 3
 * rpma_peer_cfg_set_direct_write_to_pmem - declare direct write to PMEM support
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_peer_cfg;
 *	int rpma_peer_cfg_set_direct_write_to_pmem(struct rpma_peer_cfg *pcfg,
 *			bool supported);
 *
 * DESCRIPTION
 * rpma_peer_cfg_set_direct_write_to_pmem() declares the support
 * of the direct write to PMEM.
 *
 * RETURN VALUE
 * The rpma_peer_cfg_set_direct_write_to_pmem() function returns 0 on success
 * or a negative error code on failure.
 *
 * ERRORS
 * rpma_peer_cfg_set_direct_write_to_pmem() can fail with the following error:
 *
 * - RPMA_E_INVAL - pcfg is NULL
 */
int rpma_peer_cfg_set_direct_write_to_pmem(struct rpma_peer_cfg *pcfg,
		bool supported);

/** 3
 * rpma_peer_cfg_get_direct_write_to_pmem - check direct write to PMEM support
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	const struct rpma_peer_cfg;
 *	int rpma_peer_cfg_get_direct_write_to_pmem(
 *			const struct rpma_peer_cfg *pcfg, bool *supported);
 *
 * DESCRIPTION
 * rpma_peer_cfg_get_direct_write_to_pmem() checks the support
 * of the direct write to PMEM.
 *
 * RETURN VALUE
 * The rpma_peer_cfg_get_direct_write_to_pmem() function returns 0 on success
 * or a negative error code on failure.
 *
 * ERRORS
 * rpma_peer_cfg_get_direct_write_to_pmem() can fail with the following error:
 *
 * - RPMA_E_INVAL - pcfg or supported are NULL
 */
int rpma_peer_cfg_get_direct_write_to_pmem(const struct rpma_peer_cfg *pcfg,
		bool *supported);

/** 3
 * rpma_peer_cfg_get_descriptor - get the descriptor of the peer configuration
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	const struct rpma_peer_cfg;
 *	int rpma_peer_cfg_get_descriptor(const struct rpma_peer_cfg *pcfg,
 *			void *desc);
 *
 * DESCRIPTION
 * rpma_peer_cfg_get_descriptor() gets the descriptor of the peer configuration.
 *
 * RETURN VALUE
 * The rpma_peer_cfg_get_descriptor() function returns 0 on success or
 * a negative error code on failure.
 *
 * ERRORS
 * rpma_peer_cfg_get_descriptor() can fail with the following error:
 *
 * - RPMA_E_INVAL - pcfg or desc are NULL
 */
int rpma_peer_cfg_get_descriptor(const struct rpma_peer_cfg *pcfg, void *desc);

/** 3
 * rpma_peer_cfg_get_descriptor_size -- get size of the peer cfg descriptor
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	const struct rpma_peer_cfg;
 *	int rpma_peer_cfg_get_descriptor_size(const struct rpma_peer_cfg *pcfg,
 *			size_t *desc_size);
 *
 * DESCRIPTION
 * rpma_peer_cfg_get_descriptor_size() gets size of the peer configuration
 * descriptor.
 *
 * RETURN VALUE
 * The rpma_peer_cfg_get_descriptor_size() function returns 0 on success or
 * a negative error code on failure.
 *
 * ERRORS
 * rpma_peer_cfg_get_descriptor_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - pcfg or desc_size is NULL
 */
int
rpma_peer_cfg_get_descriptor_size(const struct rpma_peer_cfg *pcfg,
		size_t *desc_size);

/** 3
 * rpma_peer_cfg_from_descriptor - create a peer cfg from the descriptor
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_peer_cfg;
 *	int rpma_peer_cfg_from_descriptor(void *desc, size_t desc_size,
 *			struct rpma_peer_cfg **pcfg_ptr);
 *
 * DESCRIPTION
 * rpma_peer_cfg_from_descriptor() creates a peer configuration object
 * from the descriptor.
 *
 * RETURN VALUE
 * The rpma_peer_cfg_from_descriptor() function returns 0 on success
 * or a negative error code on failure. rpma_peer_cfg_from_descriptor()
 * does not set *pcfg_ptr value on failure.
 *
 * ERRORS
 * rpma_peer_cfg_from_descriptor() can fail with the following errors:
 *
 * - RPMA_E_INVAL - desc or pcfg_ptr are NULL
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_peer_cfg_from_descriptor(void *desc, size_t desc_size,
		struct rpma_peer_cfg **pcfg_ptr);

/* peer */

struct rpma_peer;

/** 3
 * rpma_peer_new - create a peer object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct ibv_context;
 *	struct rpma_peer;
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
 *	struct rpma_peer;
 *	int rpma_peer_delete(struct rpma_peer **peer);
 *
 * DESCRIPTION
 * rpma_peer_delete() deletes the peer object.
 *
 * RETURN VALUE
 * The rpma_peer_delete() function returns 0 on success or a negative error
 * code on failure. rpm_peer_delete() does not set *peer value
 * to NULL on failure.
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

#define RPMA_MR_USAGE_READ_SRC			(1 << 0)
#define RPMA_MR_USAGE_READ_DST			(1 << 1)
#define RPMA_MR_USAGE_WRITE_SRC			(1 << 2)
#define RPMA_MR_USAGE_WRITE_DST			(1 << 3)
#define RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY	(1 << 4)
#define RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT	(1 << 5)
#define RPMA_MR_USAGE_SEND			(1 << 6)
#define RPMA_MR_USAGE_RECV			(1 << 7)

/** 3
 * rpma_mr_reg - create a local memory registration object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_mr_reg(const struct rpma_peer *peer, void *ptr, size_t size,
 *		int usage, struct rpma_mr_local **mr_ptr);
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
int rpma_mr_reg(const struct rpma_peer *peer, void *ptr, size_t size,
		int usage, struct rpma_mr_local **mr_ptr);

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
 * rpma_mr_dereg() can fail with the following errors:
 *
 * - RPMA_E_INVAL - mr_ptr is NULL
 * - RPMA_E_PROVIDER - memory deregistration failed
 */
int rpma_mr_dereg(struct rpma_mr_local **mr_ptr);

/** 3
 * rpma_mr_get_descriptor - get a descriptor of a memory region
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_mr_get_descriptor(const struct rpma_mr_local *mr, void *desc);
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
int rpma_mr_get_descriptor(const struct rpma_mr_local *mr, void *desc);

/** 3
 * rpma_mr_remote_from_descriptor - create a memory region from a descriptor
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_mr_remote_from_descriptor(const void *desc,
 *		size_t desc_size, struct rpma_mr_remote **mr_ptr);
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
 * - RPMA_E_INVAL - incompatible descriptor size
 * - RPMA_E_NOSUPP - deserialized information does not represent a valid memory
 *   region
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_mr_remote_from_descriptor(const void *desc,
		size_t desc_size, struct rpma_mr_remote **mr_ptr);

/** 3
 * rpma_mr_get_descriptor_size - get size of a memory region descriptor
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_mr_get_descriptor_size(const struct rpma_mr_local *mr,
 *			size_t *desc_size);
 *
 * ERRORS
 * rpma_mr_get_descriptor_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - mr or desc_size is NULL
 */
int rpma_mr_get_descriptor_size(const struct rpma_mr_local *mr,
		size_t *desc_size);

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
 * rpma_mr_remote_get_flush_type -- get a remote memory region's flush types
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_mr_remote_get_flush_type(const struct rpma_mr_remote *mr,
 *			int *flush_type);
 *
 * DESCRIPTION
 * rpma_mr_remote_get_flush_type() gets a flush types supported
 * by the remote memory region.
 *
 * ERRORS
 * rpma_mr_remote_get_flush_type() can fail with the following error:
 *
 * - RPMA_E_INVAL - mr or flush_type is NULL
 */
int rpma_mr_remote_get_flush_type(const struct rpma_mr_remote *mr,
					int *flush_type);

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

/* connection configuration */

struct rpma_conn_cfg;

/** 3
 * rpma_conn_cfg_new - create a new connection configuration object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_cfg_new(struct rpma_conn_cfg **cfg_ptr);
 *
 * ERRORS
 * rpma_conn_cfg_new() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg_ptr is NULL
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_conn_cfg_new(struct rpma_conn_cfg **cfg_ptr);

/** 3
 * rpma_conn_cfg_delete - delete the connection configuration object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_cfg_delete(struct rpma_conn_cfg **cfg_ptr);
 *
 * ERRORS
 * rpma_conn_cfg_delete() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg_ptr is NULL
 */
int rpma_conn_cfg_delete(struct rpma_conn_cfg **cfg_ptr);

/** 3
 * rpma_conn_cfg_set_timeout - set connection establishment timeout
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_cfg_set_timeout(struct rpma_conn_cfg *cfg,
 *		int timeout_ms);
 *
 * ERRORS
 * rpma_conn_cfg_set_timeout() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg is NULL or timeout_ms < 0
 */
int rpma_conn_cfg_set_timeout(struct rpma_conn_cfg *cfg, int timeout_ms);

/** 3
 * rpma_conn_cfg_get_timeout - get connection establishment timeout
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_cfg_get_timeout(struct rpma_conn_cfg *cfg,
 *		int *timeout_ms);
 *
 * ERRORS
 * rpma_conn_cfg_get_timeout() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg or timeout_ms is NULL
 */
int rpma_conn_cfg_get_timeout(struct rpma_conn_cfg *cfg, int *timeout_ms);

/** 3
 * rpma_conn_cfg_set_cq_size - set CQ size for the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_cfg_set_cq_size(struct rpma_conn_cfg *cfg,
 *		uint32_t cq_size);
 *
 * ERRORS
 * rpma_conn_cfg_set_cq_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg is NULL
 */
int rpma_conn_cfg_set_cq_size(struct rpma_conn_cfg *cfg, uint32_t cq_size);

/** 3
 * rpma_conn_cfg_get_cq_size - get CQ size for the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_cfg_get_cq_size(struct rpma_conn_cfg *cfg,
 *		uint32_t *cq_size);
 *
 * ERRORS
 * rpma_conn_cfg_get_cq_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg or cq_size is NULL
 */
int rpma_conn_cfg_get_cq_size(struct rpma_conn_cfg *cfg, uint32_t *cq_size);

/** 3
 * rpma_conn_cfg_set_sq_size - set SQ size for the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_cfg_set_sq_size(struct rpma_conn_cfg *cfg,
 *		uint32_t sq_size);
 *
 * ERRORS
 * rpma_conn_cfg_set_sq_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg is NULL
 */
int rpma_conn_cfg_set_sq_size(struct rpma_conn_cfg *cfg, uint32_t sq_size);

/** 3
 * rpma_conn_cfg_get_sq_size - get SQ size for the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_cfg_get_sq_size(struct rpma_conn_cfg *cfg,
 *		uint32_t *sq_size);
 *
 * ERRORS
 * rpma_conn_cfg_get_sq_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg or sq_size is NULL
 */
int rpma_conn_cfg_get_sq_size(struct rpma_conn_cfg *cfg, uint32_t *sq_size);

/** 3
 * rpma_conn_cfg_set_rq_size - set RQ size for the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_cfg_set_rq_size(struct rpma_conn_cfg *cfg,
 *		uint32_t rq_size);
 *
 * ERRORS
 * rpma_conn_cfg_set_rq_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg is NULL
 */
int rpma_conn_cfg_set_rq_size(struct rpma_conn_cfg *cfg, uint32_t rq_size);

/** 3
 * rpma_conn_cfg_get_rq_size - get RQ size for the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_cfg_get_rq_size(struct rpma_conn_cfg *cfg,
 *		uint32_t *rq_size);
 *
 * ERRORS
 * rpma_conn_cfg_get_rq_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg or rq_size is NULL
 */
int rpma_conn_cfg_get_rq_size(struct rpma_conn_cfg *cfg, uint32_t *rq_size);

/* connection */

struct rpma_conn;

/** 3
 * rpma_conn_get_event_fd - get an event file descriptor of the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_get_event_fd(struct rpma_conn *conn, int *fd);
 *
 * DESCRIPTION
 * Get an event file descriptor of the connection.
 *
 * ERRORS
 * rpma_conn_get_event_fd() can fail with the following error:
 *
 * - RPMA_E_INVAL - conn or fd is NULL
 */
int rpma_conn_get_event_fd(struct rpma_conn *conn, int *fd);

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
 * Obtain a pointer to the private data given by the other side
 * of the connection.
 *
 * ERRORS
 * rpma_conn_get_private_data() can fail with the following error:
 *
 * - RPMA_E_INVAL - conn or pdata is NULL
 */
int rpma_conn_get_private_data(struct rpma_conn *conn,
		struct rpma_conn_private_data *pdata);

/** 3
 * rpma_conn_apply_remote_peer_cfg - apply remote peer cfg for the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_apply_remote_peer_cfg(struct rpma_conn *conn,
 *			const struct rpma_peer_cfg *pcfg);
 *
 * ERRORS
 * rpma_conn_apply_remote_peer_cfg() can fail with the following error:
 *
 * - RPMA_E_INVAL - conn or pcfg are NULL
 */
int rpma_conn_apply_remote_peer_cfg(struct rpma_conn *conn,
		const struct rpma_peer_cfg *pcfg);

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
 *	int rpma_conn_req_new(const struct rpma_peer *peer, const char *addr,
 *		const char *port, struct rpma_conn_req **req_ptr);
 *
 * DESCRIPTION
 * Create a new outgoing connection request object using reliable,
 * connection-oriented and message-based (RDMA_PS_TCP) QP communication.
 *
 * ERRORS
 * rpma_conn_req_new() can fail with the following errors:
 *
 * - RPMA_E_INVAL - peer, addr, port or req_ptr is NULL
 * - RPMA_E_NOMEM - out of memory
 * - RPMA_E_PROVIDER - rdma_create_id(3), rdma_resolve_addr(3),
 *   rdma_resolve_route(3) or ibv_create_cq(3) failed
 */
int rpma_conn_req_new(const struct rpma_peer *peer, const char *addr,
	const char *port, struct rpma_conn_cfg *cfg,
	struct rpma_conn_req **req_ptr);

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

/** 3
 * rpma_conn_req_recv - initialize the receive operation
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_req_recv(struct rpma_conn_req *req,
 *		const struct rpma_mr_local *dst, size_t offset, size_t len,
 *		void *op_context);
 *
 * DESCRIPTION
 * Initialize the receive operation. It prepares a buffer for a message
 * send from other side of the connection. Please see rpma_send(3). This is
 * a variant of rpma_recv(3) which may be used before the connection is
 * established.
 *
 * ERRORS
 * rpma_conn_req_recv() can fail with the following errors:
 *
 * - RPMA_E_INVAL - req or src or op_context is NULL
 * - RPMA_E_PROVIDER - ibv_post_recv(3) failed
 */
int rpma_conn_req_recv(struct rpma_conn_req *req,
    const struct rpma_mr_local *dst, size_t offset, size_t len,
    void *op_context);

/* server-side setup */

struct rpma_ep;

/** 3
 * rpma_ep_listen - create a listening endpoint
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_ep_listen(const char *addr, const char *port,
 *	    struct rpma_ep **ep);
 *
 * DESCRIPTION
 * Create an endpoint and initialize listening for incoming connections
 * using reliable, connection-oriented and message-based (RDMA_PS_TCP)
 * QP communication.
 *
 * ERRORS
 * rpma_ep_listen() can fail with the following errors:
 *
 * - RPMA_E_INVAL - peer, addr, port or ep is NULL
 * - RPMA_E_PROVIDER - rdma_create_event_channel(3), rdma_create_id(3),
 *   rdma_getaddrinfo(3), rdma_listen(3) failed
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_ep_listen(const struct rpma_peer *peer, const char *addr,
	const char *port, struct rpma_ep **ep);

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
 * rpma_ep_get_fd - get a file descriptor of the endpoint
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_ep_get_fd(struct rpma_ep *ep, int *fd);
 *
 * DESCRIPTION
 * Get a file descriptor of the endpoint.
 *
 * ERRORS
 * rpma_ep_get_fd() can fail with the following error:
 *
 * - RPMA_E_INVAL - ep or fd is NULL
 */
int rpma_ep_get_fd(struct rpma_ep *ep, int *fd);

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
 * - RPMA_E_NO_NEXT - no next connection request available
 */
int rpma_ep_next_conn_req(struct rpma_ep *ep, struct rpma_conn_cfg *cfg,
		struct rpma_conn_req **req);

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
 *		const struct rpma_mr_local *dst, size_t dst_offset,
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
	const struct rpma_mr_local *dst, size_t dst_offset,
	struct rpma_mr_remote *src,  size_t src_offset,
	size_t len, int flags, void *op_context);

/** 3
 * rpma_write - initialize the write operation
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_write(struct rpma_conn *conn,
 *		struct rpma_mr_remote *dst, size_t dst_offset,
 *		const struct rpma_mr_local *src,  size_t src_offset,
 *		size_t len, int flags, void *op_context);
 *
 * DESCRIPTION
 * Initialize the write operation (transferring data from
 * the local memory to the remote memory).
 *
 * ERRORS
 * rpma_write() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn, dst or src is NULL
 * - RPMA_E_INVAL - flags are not set
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 */
int rpma_write(struct rpma_conn *conn,
	struct rpma_mr_remote *dst, size_t dst_offset,
	const struct rpma_mr_local *src,  size_t src_offset,
	size_t len, int flags, void *op_context);

#define RPMA_ATOMIC_WRITE_ALIGNMENT 8

/** 3
 * rpma_write_atomic - initialize the atomic write operation
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_write_atomic(struct rpma_conn *conn,
 *		struct rpma_mr_remote *dst, size_t dst_offset,
 *		const struct rpma_mr_local *src,  size_t src_offset,
 *		int flags, void *op_context);
 *
 * DESCRIPTION
 * Initialize the atomic write operation (transferring data from
 * the local memory to the remote memory). The atomic write operation allows
 * transferring 8 bytes of data and storing them atomically in the remote
 * memory.
 *
 * ERRORS
 * rpma_write_atomic() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn, dst or src is NULL
 * - RPMA_E_INVAL - dst_offset is not aligned to 8 bytes
 * - RPMA_E_INVAL - flags are not set
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 */
int rpma_write_atomic(struct rpma_conn *conn,
	struct rpma_mr_remote *dst, size_t dst_offset,
	const struct rpma_mr_local *src,  size_t src_offset,
	int flags, void *op_context);

/*
 * possible types of rpma_flush() operation
 */
enum rpma_flush_type {
	/* flush data down to the persistent domain */
	RPMA_FLUSH_TYPE_PERSISTENT,
	/* flush data deep enough to make it visible on the remote node */
	RPMA_FLUSH_TYPE_VISIBILITY,
};

/** 3
 * rpma_flush - initialize the flush operation
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_flush(struct rpma_conn *conn,
 *		struct rpma_mr_remote *dst, size_t dst_offset, size_t len,
 *		enum rpma_flush_type type, int flags, void *op_context);
 *
 * DESCRIPTION
 * Initialize the flush operation (finalizing a transfer of data to
 * the remote memory).
 *
 * ERRORS
 * rpma_flush() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn or dst is NULL
 * - RPMA_E_INVAL - unknown type value
 * - RPMA_E_INVAL - flags are not set
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 * - RPMA_E_NOSUPP - type is RPMA_FLUSH_TYPE_PERSISTENT and
 *                   the direct write to pmem is not supported
 */
int rpma_flush(struct rpma_conn *conn,
	struct rpma_mr_remote *dst, size_t dst_offset, size_t len,
	enum rpma_flush_type type, int flags, void *op_context);

/** 3
 * rpma_send - initialize the send operation
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_send(struct rpma_conn *conn,
 *		const struct rpma_mr_local *src, size_t offset, size_t len,
 *		int flags, void *op_context);
 *
 * DESCRIPTION
 * Initialize the send operation which transfers a message from the local
 * memory to other side of the connection.
 *
 * ERRORS
 * rpma_send() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn or src is NULL
 * - RPMA_E_INVAL - flags are not set
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 */
int rpma_send(struct rpma_conn *conn,
    const struct rpma_mr_local *src, size_t offset, size_t len,
    int flags, void *op_context);

/** 3
 * rpma_recv - initialize the receive operation
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_recv(struct rpma_conn *conn,
 *		const struct rpma_mr_local *dst, size_t offset, size_t len,
 *		void *op_context);
 *
 * DESCRIPTION
 * Initialize the receive operation which prepares a buffer for a message
 * send from other side of the connection. Please see rpma_send(3).
 *
 * All buffers prepared via rpma_recv(3) form an unordered set. When a message
 * arrives it is placed in one of the buffers awaitaning and a completion for
 * the receive operation is generated.
 *
 * A buffer for an incoming message have to be prepared beforehand.
 *
 * The order of buffers in the set does not affect the order of completions of
 * receive operations get via rpma_conn_next_completion(3).
 *
 * NOTE
 * In the RDMA standard, receive requests form an ordered queue.
 * The RPMA does NOT inherit this guarantee.
 *
 * ERRORS
 * rpma_recv() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn or src is NULL
 * - RPMA_E_PROVIDER - ibv_post_recv(3) failed
 */
int rpma_recv(struct rpma_conn *conn,
    const struct rpma_mr_local *dst, size_t offset, size_t len,
    void *op_context);

/* completion handling */

/** 3
 * rpma_conn_get_completion_fd - get a completion file descriptor
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_get_completion_fd(struct rpma_conn *conn, int *fd);
 *
 * DESCRIPTION
 * Get a completion file descriptor of the connection.
 *
 * ERRORS
 * rpma_conn_get_completion_fd() can fail with the following error:
 *
 * - RPMA_E_INVAL - conn or fd is NULL
 */
int rpma_conn_get_completion_fd(struct rpma_conn *conn, int *fd);

enum rpma_op {
	RPMA_OP_READ,
	RPMA_OP_WRITE,
	RPMA_OP_FLUSH,
	RPMA_OP_SEND,
	RPMA_OP_RECV,
};

struct rpma_completion {
	void *op_context;
	enum rpma_op op;
	uint32_t byte_len;
	enum ibv_wc_status op_status;
};

/** 3
 * rpma_conn_prepare_completions - wait for completions
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_conn_prepare_completions(struct rpma_conn *conn);
 *
 * DESCRIPTION
 * rpma_conn_prepare_completions() waits for incoming completions. If it
 * succeeded the completions can be collected using rpma_conn_next_completion().
 *
 * ERRORS
 * rpma_conn_prepare_completions() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn is NULL
 * - RPMA_E_PROVIDER - ibv_req_notify_cq(3) failed with a provider error
 * - RPMA_E_NO_COMPLETION - no completions available
 */
int rpma_conn_prepare_completions(struct rpma_conn *conn);

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
 * - RPMA_E_NO_COMPLETION - no completions available
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
 * If an error is detected during the call to a librpma(7) function,
 * the application may retrieve an error message describing the reason
 * of the failure from rpma_err_get_msg(). The error message buffer
 * is thread-local; errors encountered in one thread do not affect its value
 * in other threads. The buffer is never cleared by any library function;
 * its content is significant only when the return value of the immediately
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

/* librpma logging mechanism control */

/*
 * Available log levels in librpma. Log levels (except RPMA_LOG_DISABLED)
 * are used in logging API calls to indicate logging message severity.
 * Log levels are also used to define thresholds for logging.
 */
enum rpma_log_level {
	/* all messages will be suppressed */
	RPMA_LOG_DISABLED = -1,
	/* an error that causes the library to stop working immediately */
	RPMA_LOG_LEVEL_FATAL,
	/* an error that causes the library to stop working properly */
	RPMA_LOG_LEVEL_ERROR,
	/* an errors that could be handled in the upper level */
	RPMA_LOG_LEVEL_WARNING,
	/*
	 * non-massive info mainly related to public API function completions
	 * e.g. connection established
	 */
	RPMA_LOG_LEVEL_NOTICE,
	/* massive info e.g. every write operation indication */
	RPMA_LOG_LEVEL_INFO,
	/* debug info e.g. write operation dump */
	RPMA_LOG_LEVEL_DEBUG,
};

enum rpma_log_threshold {
	/*
	 * the main threshold level - the logging messages above this level
	 * won't trigger the logging functions
	 */
	RPMA_LOG_THRESHOLD,
	/*
	 * the auxiliary threshold level - may or may not be used by
	 * the logging function
	 */
	RPMA_LOG_THRESHOLD_AUX,
	RPMA_LOG_THRESHOLD_MAX
};

/** 3
 * rpma_log_set_threshold - set the logging threshold level
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_log_set_threshold(enum rpma_log_threshold threshold,
 *			enum rpma_log_level level);
 *
 *	enum rpma_log_level {
 *		RPMA_LOG_DISABLED,
 *		RPMA_LOG_LEVEL_FATAL,
 *		RPMA_LOG_LEVEL_ERROR,
 *		RPMA_LOG_LEVEL_WARNING,
 *		RPMA_LOG_LEVEL_NOTICE,
 *		RPMA_LOG_LEVEL_INFO,
 *		RPMA_LOG_LEVEL_DEBUG,
 *	};
 *
 *	enum rpma_log_threshold {
 *		RPMA_LOG_THRESHOLD,
 *		RPMA_LOG_THRESHOLD_AUX,
 *		RPMA_LOG_THRESHOLD_MAX
 *	};
 *
 * DESCRIPTION
 * rpma_log_set_threshold() sets the logging threshold level.
 *
 * Available thresholds are:
 * - RPMA_LOG_THRESHOLD - the main threshold used to filter out
 * undesired logging messages. Messages on a higher level than the primary
 * threshold level are ignored. RPMA_LOG_DISABLED shall be used to suppress
 * logging. The default value is RPMA_LOG_WARNING.
 * - RPMA_LOG_THRESHOLD_AUX - the auxiliary threshold intended for use
 * inside the logging function (please see rpma_log_get_threshold(3)).
 * The logging function may or may not take this threshold into consideration.
 * The default value is RPMA_LOG_DISABLED.
 *
 * Available threshold levels are defined by enum rpma_log_level:
 * - RPMA_LOG_DISABLED - all messages will be suppressed
 * - RPMA_LOG_LEVEL_FATAL - an error that causes the library to stop working
 * immediately
 * - RPMA_LOG_LEVEL_ERROR - an error that causes the library to stop working
 * properly
 * - RPMA_LOG_LEVEL_WARNING - an error that could be handled in the upper level
 * - RPMA_LOG_LEVEL_NOTICE - non-massive info mainly related to public
 * API function completions e.g. connection established
 * - RPMA_LOG_LEVEL_INFO - massive info e.g. every write operation indication
 * - RPMA_LOG_LEVEL_DEBUG - debug info e.g. write operation dump
 *
 * THE DEFAULT LOGGING FUNCTION
 * The default logging function writes messages to syslog(3) and to stderr(3),
 * where syslog(3) is the primary destination
 * (RPMA_LOG_THRESHOLD applies) whereas stderr(3) is the secondary
 * destination (RPMA_LOG_THRESHOLD_AUX applies).
 *
 * RETURN VALUE
 * rpma_log_syslog_set_threshold() function returns 0 on success or error code
 * on failure.
 *
 * ERRORS
 * rpma_log_set_threshold() can fail with the following errors:
 * - RPMA_E_INVAL - threshold is not RPMA_LOG_THRESHOLD nor
 * RPMA_LOG_THRESHOLD_AUX
 * - RPMA_E_INVAL - level is not a value defined by enum rpma_log_level type
 */
int rpma_log_set_threshold(enum rpma_log_threshold threshold,
				enum rpma_log_level level);

/** 3
 * rpma_log_get_threshold - get the logging threshold level
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_log_get_threshold(enum rpma_log_threshold threshold,
 *			enum rpma_log_level *level);
 *
 * DESCRIPTION
 * rpma_log_get_threshold() gets the current level of the threshold.
 * See rpma_log_set_threshold(3) for available thresholds and levels.
 *
 * RETURN VALUE
 * rpma_log_get_threshold() function returns 0 on success or error code
 * on failure.
 *
 * ERRORS
 * rpma_log_get_threshold() can fail with the following errors:
 * - RPMA_E_INVAL - threshold is not RPMA_LOG_THRESHOLD nor
 * RPMA_LOG_THRESHOLD_AUX
 * - RPMA_E_INVAL - *level is NULL
 */
int rpma_log_get_threshold(enum rpma_log_threshold threshold,
				enum rpma_log_level *level);

/*
 * the type used for defining logging functions
 */
typedef void log_function(
	/* the log level of the message */
	enum rpma_log_level level,
	/* name of the source file where the message coming from */
	const char *file_name,
	/* the source file line where the message coming from */
	const int line_no,
	/* the function name where the message coming from */
	const char *function_name,
	/* printf(3)-like format string of the message */
	const char *message_format,
	/* additional arguments of the message format string */
	...);

#define RPMA_LOG_USE_DEFAULT_FUNCTION (NULL)

/** 3
 * rpma_log_set_function - set the log function
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	typedef void log_function(
 *		enum rpma_log_level level,
 *		const char *file_name,
 *		const int line_no,
 *		const char *function_name,
 *		const char *message_format,
 *		...);
 *
 *	void rpma_log_set_function(log_function *log_function);
 *
 * DESCRIPTION
 * rpma_log_set_function() allows choosing the function which will get all
 * the generated logging messages. The log_function can be either
 * RPMA_LOG_USE_DEFAULT_FUNCTION which will use the default logging function
 * (built into the library) or a pointer to user-defined function.
 *
 * Parameters of a user-defined log function are as follow:
 * - level - the log level of the message
 * - file_name - name of the source file where the message coming from.
 * It could be set to NULL and in such case neither line_no nor function_name
 * are provided.
 * - line_no - the source file line where the message coming from
 * - function_name - the function name where the message coming from
 * - message_format - printf(3)-like format string of the message
 * - ... - additional arguments of the message format string
 *
 * THE DEFAULT LOGGING FUNCTION
 * The initial value of the logging function is RPMA_LOG_USE_DEFAULT_FUNCTION.
 * This function writes messages to syslog(3) (the primary destination) and to
 * stderr(3) (the secondary destination).
 *
 * NOTE
 * The logging messages on the levels above the RPMA_LOG_THRESHOLD
 * level won't trigger the logging function.
 */
void rpma_log_set_function(log_function *log_function);

#endif /* LIBRPMA_H */
