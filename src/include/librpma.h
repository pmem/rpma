/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2019-2022, Intel Corporation */
/* Copyright 2021-2022, Fujitsu */

/*
 * librpma.h -- definitions of librpma entry points
 *
 * This library provides low-level support for remote access to persistent memory utilizing
 * RDMA-capable NICs.
 */

#ifndef LIBRPMA_H
#define LIBRPMA_H 1

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include <infiniband/verbs.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 7
 * librpma - remote persistent memory access library
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *	cc ... -lrpma
 *
 * DESCRIPTION
 *
 * librpma is a C library to simplify accessing persistent memory (PMem) on remote hosts over
 * Remote Direct Memory Access (RDMA).
 *
 * The librpma library provides two possible schemes of operation: Remote Memory Access and
 * Messaging. Both of them are available over a connection established between two peers. Both of
 * these schemes can make use of PMem as well as DRAM for the sake of building efficient and
 * scalable Remote Persistent Memory Accessing (RPMA) applications.
 *
 * REMOTE MEMORY ACCESS
 *
 * The librpma library implements four basic API calls dedicated for accessing a remote memory:
 * - rpma_read() - initiates transferring data from the remote memory to the local memory,
 * - rpma_write() - initiates transferring data from the local memory to the remote memory),
 * - rpma_atomic_write() - works like rpma_write(), but it allows transferring 8 bytes of data
 *   (RPMA_ATOMIC_WRITE_ALIGNMENT) and storing them atomically in the remote memory (see
 *   rpma_atomic_write(3) for details and restrictions), and:
 * - rpma_flush() - initiates finalizing a transfer of data to the remote memory. Possible types of
 *   rpma_flush() operation:
 *   - RPMA_FLUSH_TYPE_PERSISTENT - flush data down to the persistent domain,
 *   - RPMA_FLUSH_TYPE_VISIBILITY - flush data deep enough to make it visible on the remote node.
 *
 * All the above functions use the attribute flags to set the completion notification indicator:
 * - RPMA_F_COMPLETION_ON_ERROR - generates the completion only on error
 * - RPMA_F_COMPLETION_ALWAYS - generates the completion regardless of a result of the operation.
 *
 * All of these operations are considered as finished when the respective completion is generated.
 *
 * DIRECT WRITE TO PMEM
 *
 * \f[B]Direct Write to PMem\f[R] is a feature of a platform and its configuration which allows
 * an RDMA-capable network interface to write data to platform's PMem in a persistent way. It may
 * be impossible because of e.g. caching mechanisms existing on the data's way. When
 * \f[B]Direct Write to PMem\f[R] is impossible, operating in the way assuming it is possible may
 * corrupt data on PMem, so this is why \f[B]Direct Write to PMem\f[R] is not enabled by default.
 *
 * On the current Intel platforms, the only thing you have to do in order to enable
 * \f[B]Direct Write to PMem\f[R] is turning off Intel Direct Data I/O (DDIO). Sometimes, you can
 * turn off DDIO either globally for the whole platform or for a specific PCIe Root Port.
 * For details, please see the manual of your platform.
 *
 * When you have a platform which allows \f[B]Direct Write to PMem\f[R], you have to declare this
 * is the case in your peer's configuration. The peer's configuration has to be transferred to all
 * the peers which want to execute rpma_flush() with RPMA_FLUSH_TYPE_PERSISTENT against
 * the platform's PMem and applied to the connection object which safeguards access to PMem.
 *
 * - rpma_peer_cfg_set_direct_write_to_pmem() - declare \f[B]Direct Write to PMem\f[R] support
 * - rpma_peer_cfg_get_descriptor() - get the descriptor of the peer configuration
 * - rpma_peer_cfg_from_descriptor() - create a peer configuration from the descriptor
 * - rpma_conn_apply_remote_peer_cfg() - apply remote peer cfg to the connection
 *
 * For details on how to use these APIs please see
 * https://github.com/pmem/rpma/tree/master/examples/05-flush-to-persistent.
 *
 * CLIENT OPERATION
 * A client is the active side of the process of establishing a connection. A role of the peer
 * during the process of establishing connection does not determine direction of the data flow
 * (neither via Remote Memory Access nor via Messaging). After establishing the connection both
 * peers have the same capabilities.
 *
 * The client, in order to establish a connection, has to perform the following steps:
 *
 * - rpma_conn_req_new() - create a new outgoing connection request object
 * - rpma_conn_req_connect() - initiate processing the connection request
 * - rpma_conn_next_event() - wait for the RPMA_CONN_ESTABLISHED event
 *
 * After establishing the connection both peers can perform Remote Memory Access and/or Messaging
 * over the connection.
 *
 * The client, in order to close a connection, has to perform the following steps:
 *
 * - rpma_conn_disconnect() - initiate disconnection
 * - rpma_conn_next_event() - wait for the RPMA_CONN_CLOSED event
 * - rpma_conn_delete() - delete the closed connection
 *
 * SERVER OPERATION
 * A server is the passive side of the process of establishing a connection. Note that after
 * establishing the connection both peers have the same capabilities.
 *
 * The server, in order to establish a connection, has to perform the following steps:
 *
 * - rpma_ep_listen() - create a listening endpoint
 * - rpma_ep_next_conn_req() - obtain an incoming connection request
 * - rpma_conn_req_connect() - initiate connecting the connection request
 * - rpma_conn_next_event() - wait for the RPMA_CONN_ESTABLISHED event
 *
 * After establishing the connection both peers can perform Remote Memory Access and/or Messaging
 * over the connection.
 *
 * The server, in order to close a connection, has to perform the following steps:
 *
 * - rpma_conn_next_event() - wait for the RPMA_CONN_CLOSED event
 * - rpma_conn_disconnect() - disconnect the connection
 * - rpma_conn_delete() - delete the closed connection
 *
 * When no more incoming connections are expected, the server can stop waiting for them:
 *
 * - rpma_ep_shutdown() - stop listening and delete the endpoint
 *
 * MEMORY MANAGEMENT
 *
 * Every piece of memory (either volatile or persistent) must be registered and its usage must be
 * specified in order to be used in Remote Memory Access or Messaging. This can be done using
 * the following memory management librpma functions:
 * - rpma_mr_reg() which registers a memory region and creates a local memory registration object
 *   and
 * - rpma_mr_dereg() which deregisters the memory region and deletes the local memory registration
 *   object.
 *
 * A description of the registered memory region sometimes has to be transferred via network to
 * the other side of the connection. In order to do that a network-transferable description of
 * the provided memory region (called 'descriptor') has to be created using
 * rpma_mr_get_descriptor(). On the other side of the connection the received descriptor should be
 * decoded using rpma_mr_remote_from_descriptor(). It creates a remote memory region's structure
 * that allows for Remote Memory Access.
 *
 * MESSAGING
 *
 * The librpma messaging API allows transferring messages (buffers of arbitrary data) between
 * the peers. Transferring messages requires preparing buffers (memory regions) on the remote side
 * to receive the sent data. The received data are written to those dedicated buffers and
 * the sender does not have to have a respective remote memory region object to send a message.
 * The memory buffers used for messaging have to be registered using rpma_mr_reg() prior to
 * rpma_send() or rpma_recv() function call.
 *
 * The librpma library implements the following messaging API:
 * - rpma_send() - initiates the send operation which transfers a message from the local memory to
 *   other side of the connection,
 * - rpma_recv() - initiates the receive operation which prepares a buffer for a message sent from
 *   other side of the connection,
 * - rpma_conn_req_recv() works as rpma_recv(), but it may be used before the connection is
 *   established.
 *
 * All of these operations are considered as finished when the respective completion is generated.
 *
 * COMPLETIONS
 *
 * RDMA operations generate complitions that notify a user that the respective operation has been
 * completed.
 *
 * The following operations are available in librpma:
 * - IBV_WC_RDMA_READ - RMA read operation
 * - IBV_WC_RDMA_WRITE - RMA write operation
 * - IBV_WC_SEND - messaging send operation
 * - IBV_WC_RECV - messaging receive operation
 * - IBV_WC_RECV_RDMA_WITH_IMM - messaging receive operation for RMA write operation with immediate
 *   data
 *
 * All operations generate completion on error. The operations posted with
 * the \f[B]RPMA_F_COMPLETION_ALWAYS\f[R] flag also generate a completion on success.
 * Completion codes are reused from the libibverbs library, where the IBV_WC_SUCCESS status
 * indicates the successful completion of an operation. Completions are collected in the completion
 * queue (CQ) (see the \f[B]QUEUES, PERFORMANCE AND RESOURCE USE\f[R] section for more details
 * on queues).
 *
 * The librpma library implements the following API for handling completions:
 * - rpma_conn_get_cq() gets the connection's main CQ,
 * - rpma_conn_get_rcq() gets the connection's receive CQ,
 * - rpma_cq_wait() waits for an incoming completion from the specified CQ (main or receive CQ) -
 *   if it succeeds the completion can be collected using rpma_cq_get_wc(),
 * - rpma_cq_get_wc() receives the next available completion of an already posted operation.
 *
 * PEER
 *
 * A peer is an abstraction representing an RDMA-capable device.
 * All other RPMA objects have to be created in the context of a peer.
 * A peer allows one to:
 * - establish connections (Client Operation)
 * - register memory regions (Memory Management)
 * - create endpoints for listening for incoming connections (Server Operation)
 *
 * At the beginning, in order to create a peer, a user has to obtain
 * an RDMA device context by the given IPv4/IPv6 address using
 * rpma_utils_get_ibv_context(). Then a new peer object can be created
 * using rpma_peer_new() and deleted using rpma_peer_delete().
 *
 * SYNCHRONOUS AND ASYNCHRONOUS MODES
 * By default, all endpoints and connections operate in the synchronous mode where:
 *
 * - rpma_ep_next_conn_req(),
 * - rpma_cq_wait() and
 * - rpma_conn_get_next_event()
 *
 * are blocking calls. You can make those API calls non-blocking by modifying the respective file
 * descriptors:
 *
 * - rpma_ep_get_fd() - provides a file descriptor for rpma_ep_next_conn_req()
 * - rpma_cq_get_fd() - provides a file descriptor for rpma_cq_wait()
 * - rpma_conn_get_event_fd() - provides a file descriptor for rpma_conn_get_next_event()
 *
 * When you have a file descriptor, you can make it non-blocking using fcntl(2) as follows:
 *
 *	int ret = fcntl(fd, F_GETFL);
 *	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
 *
 * Such change makes the respective API call non-blocking automatically.
 *
 * The provided file descriptors can also be used for scalable I/O handling like epoll(7).
 *
 * Please see the example showing how to make use of RPMA file descriptors:
 * https://github.com/pmem/rpma/tree/master/examples/06-multiple-connections
 *
 * .SH QUEUES, PERFORMANCE AND RESOURCE USE
 *
 * \f[B]Remote Memory Access\f[R] operations, \f[B]Messaging\f[R] operations and their
 * \f[B]Completions\f[R] consume space in queues allocated in an RDMA-capable network interface
 * (RNIC) hardware for each of the connections. You must be aware of the existence of these queues:
 *
 * - completion queue \f[B](CQ)\f[R] where completions of operations are placed, either when
 *   a completion was required by a user (RPMA_F_COMPLETION_ALWAYS) or a completion with an error
 *   occurred. All \f[B]Remote Memory Access\f[R] operations and \f[B]Messaging\f[R] operations can
 *   consume \f[B]CQ\f[R] space.
 * - send queue \f[B](SQ)\f[R] where all \f[B]Remote Memory Access\f[R] operations and rpma_send()
 *   operations are placed before they are executed by RNIC.
 * - receive queue \f[B](RQ)\f[R] where rpma_recv() entries are placed before they are consumed by
 *   the rpma_send() coming from another side of the connection.
 *
 * You must assume \f[B]SQ\f[R] and \f[B]RQ\f[R] entries occupy the place in their respective queue
 * till:
 *
 * - a respective operation's completion is generated or
 * - a completion of an operation, which was scheduled later, is generated.
 *
 * You must also be aware that RNIC has limited resources so it is impossible to store a very long
 * set of queues for many possibly existing connections. If all of the queues will not fit into
 * RNIC's resources it will start using the platform's memory for this purpose. In this case,
 * the performance will be degraded because of inevitable cache misses.
 *
 * Because the length of queues has so profound impact on the performance of RPMA application you
 * can configure the length of each of the queues separately for each of the connections:
 *
 * - rpma_conn_cfg_set_cq_size() - set length of \f[B]CQ\f[R]
 * - rpma_conn_cfg_set_sq_size() - set length of \f[B]SQ\f[R]
 * - rpma_conn_cfg_set_rq_size() - set length of \f[B]RQ\f[R]
 *
 * When the connection configuration object is ready it has to be used for either
 * rpma_conn_req_new() or rpma_ep_next_conn_req() for the settings to take effect.
 *
 * THREAD SAFETY
 *
 * The analysis of thread safety of the librpma library is described in details in
 * the THREAD_SAFETY.md file:
 *
 *	https://github.com/pmem/rpma/blob/master/THREAD_SAFETY.md
 *
 * .SH ON-DEMAND PAGING SUPPORT
 *
 * On-Demand-Paging (ODP) is a technique that simplifies the memory registration process (for
 * example, applications no longer need to pin down the underlying physical pages of the address
 * space and track the validity of the mappings). On-Demand Paging is available if both
 * the hardware and the kernel support it. The detailed description of ODP can be found here:
 *
 *      https://community.mellanox.com/s/article/understanding-on-demand-paging--odp-x
 *
 * State of ODP support can be checked using the rpma_utils_ibv_context_is_odp_capable() function
 * that queries the RDMA device context's capabilities and checks if it supports On-Demand Paging.
 *
 * The librpma library uses ODP automatically if it is supported. ODP support is required to
 * register PMem memory region mapped from File System DAX (FSDAX).
 *
 * DEBUGGING AND ERROR HANDLING
 *
 * If a librpma function may fail, it returns a negative error code. Checking if the returned value
 * is non-negative is the only programmatically available way to verify if the API call succeeded.
 * The exact meaning of all error codes is described in the manual of each function.
 *
 * The librpma library implements the logging API which may give additional information in case of
 * an error and during normal operation as well, according to the current logging threshold levels.
 *
 * The function that will handle all generated log messages can be set using
 * rpma_log_set_function(). The logging function can be either the default logging function (built
 * into the library) or a user-defined, thread-safe, function. The default logging function can
 * write messages to syslog(3) and stderr(3). The logging threshold level can be set or got using
 * rpma_log_set_threshold() or rpma_log_get_threshold() respectively.
 *
 * There is an example of the usage of the logging functions:
 * https://github.com/pmem/rpma/tree/master/examples/log
 *
 * EXAMPLES
 *
 * See https://github.com/pmem/rpma/tree/master/examples for examples of using the librpma API.
 *
 * ACKNOWLEDGEMENTS
 *
 * librpma is built on the top of libibverbs and librdmacm APIs.
 *
 * DEPRECATING
 *
 * Using of the API calls which are marked as deprecated should be avoided, because they will be
 * removed in a new major release.
 *
 * NOTE: API calls deprecated in 0.X release will be removed in 0.(X+1) release usually.
 *
 * SEE ALSO
 *
 * https://pmem.io/rpma/
 */

#define RPMA_W_WAIT_FOR_COMPLETION	(1)

#define RPMA_E_UNKNOWN		(-100000) /* Unknown error */
#define RPMA_E_NOSUPP		(-100001) /* Not supported */
#define RPMA_E_PROVIDER		(-100002) /* Provider error occurred */
#define RPMA_E_NOMEM		(-100003) /* Out of memory */
#define RPMA_E_INVAL		(-100004) /* Invalid argument */
#define RPMA_E_NO_COMPLETION	(-100005) /* No next completion available */
#define RPMA_E_NO_EVENT		(-100006) /* No next event available */
#define RPMA_E_AGAIN		(-100007) /* Temporary error */
#define RPMA_E_SHARED_CHANNEL	(-100008) /* Completion channel is shared */
#define RPMA_E_NOT_SHARED_CHNL	(-100009) /* Completion channel isn't shared */

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
 *	int rpma_utils_get_ibv_context(const char *addr, enum rpma_util_ibv_context_type type,
 *		struct ibv_context **ibv_ctx_ptr);
 *
 * DESCRIPTION
 * rpma_utils_get_ibv_context() obtains an RDMA device context by the given IPv4/IPv6 address
 * (either local or remote) using the TCP RDMA port space (RDMA_PS_TCP) - reliable,
 * connection-oriented and message-based QP communication. Possible values of the 'type' argument:
 * - RPMA_UTIL_IBV_CONTEXT_LOCAL - lookup for a device based on the given local address
 * - RPMA_UTIL_IBV_CONTEXT_REMOTE - lookup for a device based on the given remote address
 *
 * RETURN VALUE
 * The rpma_utils_get_ibv_context() function returns 0 on success or a negative error code on
 * failure. rpma_utils_get_ibv_context() does not set *ibv_ctx_ptr value on failure.
 *
 * ERRORS
 * rpma_utils_get_ibv_context() can fail with the following errors:
 *
 * - RPMA_E_INVAL - addr or ibv_ctx_ptr is NULL or type is unknown
 * - RPMA_E_NOMEM - out of memory
 * - RPMA_E_PROVIDER - rdma_getaddrinfo(), rdma_create_id(), rdma_bind_addr() or
 *   rdma_resolve_addr() failed, the exact cause of the error can be read from the log
 *
 * SEE ALSO
 * rpma_peer_new(3), rpma_utils_ibv_context_is_odp_capable(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_utils_get_ibv_context(const char *addr, enum rpma_util_ibv_context_type type,
		struct ibv_context **ibv_ctx_ptr);

/** 3
 * rpma_utils_ibv_context_is_odp_capable - is On-Demand Paging supported
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct ibv_context;
 *	int rpma_utils_ibv_context_is_odp_capable(struct ibv_context *ibv_ctx,
 *		int *is_odp_capable);
 *
 * DESCRIPTION
 * rpma_utils_ibv_context_is_odp_capable() queries the RDMA device context's capabilities and check
 * if it supports On-Demand Paging.
 *
 * RETURN VALUE
 * The rpma_utils_ibv_context_is_odp_capable() function returns 0 on success or a negative error
 * code on failure. The *is_odp_capable value on failure is undefined.
 *
 * ERRORS
 * rpma_utils_ibv_context_is_odp_capable() can fail with the following errors:
 *
 * - RPMA_E_INVAL - ibv_ctx or is_odp_capable is NULL
 * - RPMA_E_PROVIDER - ibv_query_device_ex() failed, the exact cause of the error can be read from
 *   the log
 *
 * SEE ALSO
 * rpma_utils_get_ibv_context(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_utils_ibv_context_is_odp_capable(struct ibv_context *ibv_ctx, int *is_odp_capable);

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
 * The rpma_peer_cfg_new() function returns 0 on success or a negative error code on failure.
 * rpm_peer_cfg_new() does not set *pcfg_ptr value on failure.
 *
 * ERRORS
 * rpma_peer_cfg_new() can fail with the following errors:
 *
 * - RPMA_E_INVAL - pcfg_ptr is NULL
 * - RPMA_E_NOMEM - out of memory
 *
 * SEE ALSO
 * rpma_conn_apply_remote_peer_cfg(3), rpma_peer_cfg_delete(3), rpma_peer_cfg_from_descriptor(3),
 * rpma_peer_cfg_get_descriptor(3), rpma_peer_cfg_get_descriptor_size(3),
 * rpma_peer_cfg_get_direct_write_to_pmem(3), rpma_peer_cfg_set_direct_write_to_pmem(3), librpma(7)
 * and https://pmem.io/rpma/
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
 * The rpma_peer_cfg_delete() function returns 0 on success or a negative error code on failure.
 * rpm_peer_cfg_delete() does not set *pcfg_ptr value to NULL on failure.
 *
 * ERRORS
 * rpma_peer_cfg_delete() can fail with the following error:
 *
 * - RPMA_E_INVAL - pcfg_ptr is NULL
 *
 * SEE ALSO
 * rpma_peer_cfg_new(3), librpma(7) and https://pmem.io/rpma/
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
 *	int rpma_peer_cfg_set_direct_write_to_pmem(struct rpma_peer_cfg *pcfg, bool supported);
 *
 * DESCRIPTION
 * rpma_peer_cfg_set_direct_write_to_pmem() declares the support of the direct write to PMEM.
 *
 * RETURN VALUE
 * The rpma_peer_cfg_set_direct_write_to_pmem() function returns 0 on success or a negative error
 * code on failure.
 *
 * ERRORS
 * rpma_peer_cfg_set_direct_write_to_pmem() can fail with the following error:
 *
 * - RPMA_E_INVAL - pcfg is NULL
 *
 * SEE ALSO
 * rpma_conn_apply_remote_peer_cfg(3), rpma_peer_cfg_get_descriptor(3),
 * rpma_peer_cfg_get_direct_write_to_pmem(3), rpma_peer_cfg_new(3), librpma(7) and
 * https://pmem.io/rpma/
 */
int rpma_peer_cfg_set_direct_write_to_pmem(struct rpma_peer_cfg *pcfg, bool supported);

/** 3
 * rpma_peer_cfg_get_direct_write_to_pmem - check direct write to PMEM support
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_peer_cfg;
 *	int rpma_peer_cfg_get_direct_write_to_pmem(
 *			const struct rpma_peer_cfg *pcfg, bool *supported);
 *
 * DESCRIPTION
 * rpma_peer_cfg_get_direct_write_to_pmem() checks the support of the direct write to PMEM.
 *
 * RETURN VALUE
 * The rpma_peer_cfg_get_direct_write_to_pmem() function returns 0 on success or a negative error
 * code on failure.
 *
 * ERRORS
 * rpma_peer_cfg_get_direct_write_to_pmem() can fail with the following error:
 *
 * - RPMA_E_INVAL - pcfg or supported are NULL
 *
 * SEE ALSO
 * rpma_peer_cfg_from_descriptor(3), rpma_peer_cfg_new(3),
 * rpma_peer_cfg_set_direct_write_to_pmem(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_peer_cfg_get_direct_write_to_pmem(const struct rpma_peer_cfg *pcfg, bool *supported);

/** 3
 * rpma_peer_cfg_get_descriptor - get the descriptor of the peer configuration
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_peer_cfg;
 *	int rpma_peer_cfg_get_descriptor(const struct rpma_peer_cfg *pcfg, void *desc);
 *
 * DESCRIPTION
 * rpma_peer_cfg_get_descriptor() gets the descriptor of the peer configuration.
 *
 * RETURN VALUE
 * The rpma_peer_cfg_get_descriptor() function returns 0 on success or a negative error code on
 * failure.
 *
 * ERRORS
 * rpma_peer_cfg_get_descriptor() can fail with the following error:
 *
 * - RPMA_E_INVAL - pcfg or desc are NULL
 *
 * SEE ALSO
 * rpma_peer_cfg_from_descriptor(3), rpma_peer_cfg_new(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_peer_cfg_get_descriptor(const struct rpma_peer_cfg *pcfg, void *desc);

/** 3
 * rpma_peer_cfg_get_descriptor_size - get size of the peer cfg descriptor
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_peer_cfg;
 *	int rpma_peer_cfg_get_descriptor_size(const struct rpma_peer_cfg *pcfg, size_t *desc_size);
 *
 * DESCRIPTION
 * rpma_peer_cfg_get_descriptor_size() gets size of the peer configuration descriptor.
 *
 * RETURN VALUE
 * The rpma_peer_cfg_get_descriptor_size() function returns 0 on success or a negative error code
 * on failure.
 *
 * ERRORS
 * rpma_peer_cfg_get_descriptor_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - pcfg or desc_size is NULL
 *
 * SEE ALSO
 * rpma_peer_cfg_get_descriptor(3), rpma_peer_cfg_new(3), librpma(7) and https://pmem.io/rpma/
 */
int
rpma_peer_cfg_get_descriptor_size(const struct rpma_peer_cfg *pcfg, size_t *desc_size);

/** 3
 * rpma_peer_cfg_from_descriptor - create a peer cfg from the descriptor
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_peer_cfg;
 *	int rpma_peer_cfg_from_descriptor(const void *desc, size_t desc_size,
 *			struct rpma_peer_cfg **pcfg_ptr);
 *
 * DESCRIPTION
 * rpma_peer_cfg_from_descriptor() creates a peer configuration object from the descriptor.
 *
 * RETURN VALUE
 * The rpma_peer_cfg_from_descriptor() function returns 0 on success or a negative error code on
 * failure. rpma_peer_cfg_from_descriptor() does not set *pcfg_ptr value on failure.
 *
 * ERRORS
 * rpma_peer_cfg_from_descriptor() can fail with the following errors:
 *
 * - RPMA_E_INVAL - desc or pcfg_ptr are NULL
 * - RPMA_E_NOMEM - out of memory
 *
 * SEE ALSO
 * rpma_conn_apply_remote_peer_cfg(3), rpma_peer_cfg_get_descriptor(3), rpma_peer_cfg_new(3),
 * librpma(7) and https://pmem.io/rpma/
 */
int rpma_peer_cfg_from_descriptor(const void *desc, size_t desc_size,
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
 *	int rpma_peer_new(struct ibv_context *ibv_ctx, struct rpma_peer **peer_ptr);
 *
 * DESCRIPTION
 * rpma_peer_new() creates a new peer object.
 *
 * RETURN VALUE
 * The rpma_peer_new() function returns 0 on success or a negative error code on failure.
 * rpma_peer_new() does not set *peer_ptr value on failure.
 *
 * ERRORS
 * rpma_peer_new() can fail with the following errors:
 *
 * - RPMA_E_INVAL - ibv_ctx or peer_ptr is NULL
 * - RPMA_E_NOMEM - creating a verbs protection domain failed with ENOMEM.
 * - RPMA_E_PROVIDER - creating a verbs protection domain failed with error other than ENOMEM.
 * - RPMA_E_UNKNOWN - creating a verbs protection domain failed without error value.
 * - RPMA_E_NOMEM - out of memory
 *
 * SEE ALSO
 * rpma_conn_req_new(3), rpma_ep_listen(3), rpma_mr_reg(3), rpma_peer_delete(3),
 * rpma_utils_get_ibv_context(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_peer_new(struct ibv_context *ibv_ctx, struct rpma_peer **peer_ptr);

/** 3
 * rpma_peer_delete - delete a peer object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_peer;
 *	int rpma_peer_delete(struct rpma_peer **peer_ptr);
 *
 * DESCRIPTION
 * rpma_peer_delete() deletes the peer object.
 *
 * RETURN VALUE
 * The rpma_peer_delete() function returns 0 on success or a negative error code on failure.
 * rpm_peer_delete() does not set *peer_ptr value to NULL on failure.
 *
 * RETURN VALUE
 * The rpma_peer_delete() function returns 0 on success or a negative error code on failure.
 * rpma_peer_delete() does not set *peer_ptr to NULL on failure.
 *
 * ERRORS
 * rpma_peer_delete() can fail with the following error:
 *
 * - RPMA_E_PROVIDER - deleting the verbs protection domain failed.
 *
 * SEE ALSO
 * rpma_peer_new(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_peer_delete(struct rpma_peer **peer_ptr);

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
 *	struct rpma_peer;
 *	struct rpma_mr_local;
 *
 *	int rpma_mr_reg(struct rpma_peer *peer, void *ptr, size_t size, int usage,
 *		struct rpma_mr_local **mr_ptr);
 *
 * DESCRIPTION
 * rpma_mr_reg() registers a memory region and creates a local memory registration object. The
 * usage parameter specifies the operations that can be performed on the given memory region which
 * should be expressed as bitwise-inclusive OR of the following:
 * - RPMA_MR_USAGE_READ_SRC - memory used as a source of the read operation
 * - RPMA_MR_USAGE_READ_DST - memory used as a destination of the read operation
 * - RPMA_MR_USAGE_WRITE_SRC - memory used as a source of the write operation
 * - RPMA_MR_USAGE_WRITE_DST - memory used as a destination of the write operation
 * - RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY - memory with available flush operation
 * - RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT - memory with available persistent flush operation
 * - RPMA_MR_USAGE_SEND - memory used for send operation
 * - RPMA_MR_USAGE_RECV - memory used for receive operation
 *
 * RETURN VALUE
 * The rpma_mr_reg() function returns 0 on success or a negative error code on failure.
 * rpma_mr_reg() does not set *mr_ptr value on failure.
 *
 * ERRORS
 * rpma_mr_reg() can fail with the following errors:
 *
 * - RPMA_E_INVAL - peer or ptr or mr_ptr is NULL
 * - RPMA_E_INVAL - size equals 0
 * - RPMA_E_NOMEM - out of memory
 * - RPMA_E_PROVIDER - memory registration failed
 *
 * SEE ALSO
 * rpma_conn_req_recv(3), rpma_mr_dereg(3), rpma_mr_get_descriptor(3),
 * rpma_mr_get_descriptor_size(3), rpma_peer_new(3), rpma_read(3), rpma_recv(3), rpma_send(3),
 * rpma_write(3), rpma_atomic_write(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_mr_reg(struct rpma_peer *peer, void *ptr, size_t size, int usage,
		struct rpma_mr_local **mr_ptr);

/** 3
 * rpma_mr_dereg - delete a local memory registration object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_mr_local;
 *	int rpma_mr_dereg(struct rpma_mr_local **mr_ptr);
 *
 * DESCRIPTION
 * rpma_mr_dereg() deregisters a memory region and deletes a local memory registration object.
 *
 * RETURN VALUE
 * The rpma_mr_dereg() function returns 0 on success or a negative error code on failure.
 * rpma_mr_dereg() does not set *mr_ptr value to NULL on failure.
 *
 * ERRORS
 * rpma_mr_dereg() can fail with the following errors:
 *
 * - RPMA_E_INVAL - mr_ptr is NULL
 * - RPMA_E_PROVIDER - memory deregistration failed
 *
 * SEE ALSO
 * rpma_mr_reg(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_mr_dereg(struct rpma_mr_local **mr_ptr);

/** 3
 * rpma_mr_get_descriptor - get a descriptor of a memory region
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_mr_local;
 *	int rpma_mr_get_descriptor(const struct rpma_mr_local *mr, void *desc);
 *
 * DESCRIPTION
 * rpma_mr_get_descriptor() writes a network-transferable description of the provided local memory
 * region (called 'descriptor'). Once the descriptor is transferred to the other side it should be
 * decoded by rpma_mr_remote_from_descriptor() to create a remote memory region's structure which
 * allows for Remote Memory Access. Please see librpma(7) for details.
 *
 * RETURN VALUE
 * The rpma_mr_get_descriptor() function returns 0 on success or a negative error code on failure.
 *
 * ERRORS
 * rpma_mr_get_descriptor() can fail with the following error:
 *
 * - RPMA_E_INVAL - mr or desc is NULL
 *
 * SEE ALSO
 * rpma_mr_get_descriptor_size(3), rpma_mr_reg(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_mr_get_descriptor(const struct rpma_mr_local *mr, void *desc);

/** 3
 * rpma_mr_remote_from_descriptor - create a memory region from a descriptor
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_mr_remote;
 *	int rpma_mr_remote_from_descriptor(const void *desc, size_t desc_size,
 *			struct rpma_mr_remote **mr_ptr);
 *
 * DESCRIPTION
 * Create a remote memory region's structure based on the provided descriptor with
 * a network-transferable description of the memory region local to the remote peer.
 *
 * RETURN VALUE
 * The rpma_mr_remote_from_descriptor() function returns 0 on success or a negative error code on
 * failure. rpma_mr_remote_from_descriptor() does not set *mr_ptr value on failure.
 *
 * ERRORS
 * rpma_mr_remote_from_descriptor() can fail with the following errors:
 *
 * - RPMA_E_INVAL - desc or mr_ptr is NULL
 * - RPMA_E_INVAL - incompatible descriptor size
 * - RPMA_E_NOSUPP - deserialized information does not represent a valid memory region
 * - RPMA_E_NOMEM - out of memory
 *
 * SEE ALSO
 * rpma_mr_remote_delete(3), rpma_mr_remote_get_flush_type(3), rpma_mr_remote_get_size(3),
 * rpma_flush(3), rpma_read(3), rpma_write(3), rpma_atomic_write(3), librpma(7) and
 * https://pmem.io/rpma/
 */
int rpma_mr_remote_from_descriptor(const void *desc, size_t desc_size,
		struct rpma_mr_remote **mr_ptr);

/** 3
 * rpma_mr_get_descriptor_size - get size of the memory region descriptor
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_mr_local;
 *	int rpma_mr_get_descriptor_size(const struct rpma_mr_local *mr, size_t *desc_size);
 *
 * DESCRIPTION
 * rpma_mr_get_descriptor_size() gets size of the memory region descriptor.
 *
 * RETURN VALUE
 * The rpma_mr_get_descriptor_size() function returns 0 on success or a negative error code on
 * failure. rpma_mr_get_descriptor_size() does not set *desc_size value on failure.
 *
 * ERRORS
 * rpma_mr_get_descriptor_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - mr or desc_size is NULL
 *
 * SEE ALSO
 * rpma_mr_get_descriptor(3), rpma_mr_reg(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_mr_get_descriptor_size(const struct rpma_mr_local *mr, size_t *desc_size);

/** 3
 * rpma_mr_get_ptr - get the pointer to the local memory region
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_mr_local;
 *	int rpma_mr_get_ptr(const struct rpma_mr_local *mr, void **ptr);
 *
 * DESCRIPTION
 * rpma_mr_get_ptr() gets a memory region pointer from the local memory registration object.
 *
 * RETURN VALUE
 * The rpma_mr_get_ptr() function returns 0 on success or a negative error code on failure.
 * rpma_mr_get_ptr() does not set *ptr value on failure.
 *
 * ERRORS
 * rpma_mr_get_ptr() can fail with the following error:
 *
 * - RPMA_E_INVAL - mr or ptr is NULL
 *
 * SEE ALSO
 * rpma_mr_reg(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_mr_get_ptr(const struct rpma_mr_local *mr, void **ptr);

/** 3
 * rpma_mr_get_size - get the size of the local memory region
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_mr_local;
 *	int rpma_mr_get_size(const struct rpma_mr_local *mr, size_t *size);
 *
 * DESCRIPTION
 * rpma_mr_get_size() gets a memory region size from the local memory registration object.
 *
 * RETURN VALUE
 * The rpma_mr_get_size() function returns 0 on success or a negative error code on failure.
 * rpma_mr_get_size() does not set *size value on failure.
 *
 * ERRORS
 * rpma_mr_get_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - mr or size is NULL
 *
 * SEE ALSO
 * rpma_mr_reg(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_mr_get_size(const struct rpma_mr_local *mr, size_t *size);

/** 3
 * rpma_mr_remote_get_size - get a remote memory region size
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_mr_remote;
 *	int rpma_mr_remote_get_size(const struct rpma_mr_remote *mr,
 *			size_t *size);
 *
 * DESCRIPTION
 * rpma_mr_remote_get_size() gets the size of the remote memory region.
 *
 * RETURN VALUE
 * The rpma_mr_remote_get_size() function returns 0 on success or a negative error code on failure.
 * rpma_mr_remote_get_size() does not set *size value on failure.
 *
 * ERRORS
 * rpma_mr_remote_get_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - mr or size is NULL
 *
 * SEE ALSO
 * rpma_mr_remote_from_descriptor(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_mr_remote_get_size(const struct rpma_mr_remote *mr, size_t *size);

/** 3
 * rpma_mr_remote_get_flush_type - get a remote memory region's flush types
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_mr_remote;
 *	int rpma_mr_remote_get_flush_type(const struct rpma_mr_remote *mr, int *flush_type);
 *
 * DESCRIPTION
 * rpma_mr_remote_get_flush_type() gets flush types supported by the remote memory region.
 *
 * RETURN VALUE
 * The rpma_mr_remote_get_flush_type() function returns 0 on success or a negative error code on
 * failure. rpma_mr_remote_get_flush_type() does not set *flush_type value on failure.
 *
 * ERRORS
 * rpma_mr_remote_get_flush_type() can fail with the following error:
 *
 * - RPMA_E_INVAL - mr or flush_type is NULL
 *
 * SEE ALSO
 * rpma_mr_remote_from_descriptor(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_mr_remote_get_flush_type(const struct rpma_mr_remote *mr, int *flush_type);

/** 3
 * rpma_mr_remote_delete - delete the remote memory region's structure
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_mr_remote;
 *	int rpma_mr_remote_delete(struct rpma_mr_remote **mr_ptr);
 *
 * DESCRIPTION
 * rpma_mr_remote_delete() deletes the remote memory region's structure.
 *
 * RETURN VALUE
 * The rpma_mr_remote_delete() function returns 0 on success or a negative error code on failure.
 * rpma_mr_remote_delete() does not set *mr_ptr value to NULL on failure.
 *
 * ERRORS
 * rpma_mr_remote_delete() can fail with the following error:
 *
 * - RPMA_E_INVAL - mr_ptr is NULL
 *
 * SEE ALSO
 * rpma_mr_remote_from_descriptor(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_mr_remote_delete(struct rpma_mr_remote **mr_ptr);

/** 3
 * rpma_mr_advise - give advice about an address range in a memory registration
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_mr_local *mr;
 *	int rpma_mr_advise(struct rpma_mr_local *mr, size_t offset, size_t len, int advice,
 *		uint32_t flags);
 *
 * DESCRIPTION
 * rpma_mr_advise() gives advice about an address range in a memory registration. The usage
 * parameter specifies the operations that can be performed on the given memory address range.
 * For available advice and flags values please see ibv_advise_mr(3).
 *
 * RETURN VALUE
 * The rpma_mr_advise() function returns 0 on success or a negative error code on failure.
 *
 * ERRORS
 * rpma_mr_advise() can fail with the following errors:
 *
 * - RPMA_E_INVAL - in one of the following:
 *	- the requested range is out of the memory registration bounds
 *	- the memory registration usage does not allow the specific advice
 *	- the flags are invalid
 * - RPMA_E_NOSUPP - the operation is not supported by the system
 * - RPMA_E_PROVIDER - ibv_mr_advise(3) failed for other errors
 *
 * SEE ALSO
 * rpma_mr_reg(3), ibv_mr_advise(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_mr_advise(struct rpma_mr_local *mr, size_t offset, size_t len, int advice,
		uint32_t flags);

/* connection configuration */

struct rpma_conn_cfg;

/** 3
 * rpma_conn_cfg_new - create a new connection configuration object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_cfg;
 *	int rpma_conn_cfg_new(struct rpma_conn_cfg **cfg_ptr);
 *
 * DESCRIPTION
 * rpma_conn_cfg_new() creates a new connection configuration object and fills it with the default
 * values:
 *
 *	.timeout_ms = 1000
 *	.cq_size = 10
 *	.rcq_size = 0
 *	.sq_size = 10
 *	.rq_size = 10
 *	.shared_comp_channel = false
 *
 * RETURN VALUE
 * The rpma_conn_cfg_new() function returns 0 on success or a negative error code on failure.
 * rpma_conn_cfg_new() does not set *cfg_ptr value on failure.
 *
 * ERRORS
 * rpma_conn_cfg_new() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg_ptr is NULL
 * - RPMA_E_NOMEM - out of memory
 *
 * SEE ALSO
 * rpma_conn_cfg_delete(3), rpma_conn_cfg_get_compl_channel(3), rpma_conn_cfg_get_cq_size(3),
 * rpma_conn_cfg_get_rq_size(3), rpma_conn_cfg_get_sq_size(3), rpma_conn_cfg_get_timeout(3),
 * rpma_conn_cfg_set_compl_channel(3), rpma_conn_cfg_set_cq_size(3), rpma_conn_cfg_set_rq_size(3),
 * rpma_conn_cfg_set_sq_size(3), rpma_conn_cfg_set_timeout(3), rpma_conn_req_new(3),
 * rpma_ep_next_conn_req(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_cfg_new(struct rpma_conn_cfg **cfg_ptr);

/** 3
 * rpma_conn_cfg_delete - delete the connection configuration object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_cfg;
 *	int rpma_conn_cfg_delete(struct rpma_conn_cfg **cfg_ptr);
 *
 * DESCRIPTION
 * rpma_conn_cfg_delete() deletes the connection configuration object.
 *
 * RETURN VALUE
 * The rpma_conn_cfg_delete() function returns 0 on success or a negative error code on failure.
 * rpma_conn_cfg_delete() sets *cfg_ptr value to NULL on success and on failure.
 *
 * ERRORS
 * rpma_conn_cfg_delete() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg_ptr is NULL
 *
 * SEE ALSO
 * rpma_conn_cfg_new(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_cfg_delete(struct rpma_conn_cfg **cfg_ptr);

/** 3
 * rpma_conn_cfg_set_timeout - set connection establishment timeout
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_cfg;
 *	int rpma_conn_cfg_set_timeout(struct rpma_conn_cfg *cfg, int timeout_ms);
 *
 * DESCRIPTION
 * rpma_conn_cfg_set_timeout() sets the connection establishment timeout. If this function is not
 * called, the timeout has the default value (1000) set by rpma_conn_cfg_new(3).
 *
 * RETURN VALUE
 * The rpma_conn_cfg_set_timeout() function returns 0 on success or a negative error code on
 * failure.
 *
 * ERRORS
 * rpma_conn_cfg_set_timeout() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg is NULL or timeout_ms < 0
 *
 * SEE ALSO
 * rpma_conn_cfg_new(3), rpma_conn_cfg_get_timeout(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_cfg_set_timeout(struct rpma_conn_cfg *cfg, int timeout_ms);

/** 3
 * rpma_conn_cfg_get_timeout - get connection establishment timeout
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_cfg;
 *	int rpma_conn_cfg_get_timeout(const struct rpma_conn_cfg *cfg, int *timeout_ms);
 *
 * DESCRIPTION
 * rpma_conn_cfg_get_timeout() gets the connection establishment timeout.
 *
 * RETURN VALUE
 * The rpma_conn_cfg_get_timeout() function returns 0 on success or a negative error code on
 * failure. rpma_conn_cfg_get_timeout() does not set *timeout_ms value on failure.
 *
 * ERRORS
 * rpma_conn_cfg_get_timeout() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg or timeout_ms is NULL
 *
 * SEE ALSO
 * rpma_conn_cfg_new(3), rpma_conn_cfg_set_timeout(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_cfg_get_timeout(const struct rpma_conn_cfg *cfg, int *timeout_ms);

/** 3
 * rpma_conn_cfg_set_cq_size - set CQ size for the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_cfg;
 *	int rpma_conn_cfg_set_cq_size(struct rpma_conn_cfg *cfg, uint32_t cq_size);
 *
 * DESCRIPTION
 * rpma_conn_cfg_set_cq_size() sets the CQ size for the connection. If this function is not called,
 * the cq_size has the default value (10) set by rpma_conn_cfg_new(3).
 *
 * RETURN VALUE
 * The rpma_conn_cfg_set_cq_size() function returns 0 on success or a negative error code on
 * failure.
 *
 * ERRORS
 * rpma_conn_cfg_set_cq_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg is NULL
 *
 * SEE ALSO
 * rpma_conn_cfg_new(3), rpma_conn_cfg_get_cq_size(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_cfg_set_cq_size(struct rpma_conn_cfg *cfg, uint32_t cq_size);

/** 3
 * rpma_conn_cfg_get_compl_channel - get if the completion event channel can be shared by CQ and RCQ
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_cfg;
 *	int rpma_conn_cfg_get_compl_channel(const struct rpma_conn_cfg *cfg, bool *shared);
 *
 * DESCRIPTION
 * rpma_conn_cfg_get_compl_channel() gets if the completion event channel can be shared by CQ and
 * RCQ.
 *
 * RETURN VALUE
 * The rpma_conn_cfg_get_compl_channel() function returns 0 on success or a negative error code on
 * failure. rpma_conn_cfg_get_compl_channel() does not set *shared value on failure.
 *
 * ERRORS
 * rpma_conn_cfg_get_compl_channel() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg or shared is NULL
 *
 * SEE ALSO
 * rpma_conn_cfg_new(3), rpma_conn_wait(3), rpma_conn_cfg_set_compl_channel(3), librpma(7) and
 * https://pmem.io/rpma/
 */
int rpma_conn_cfg_get_compl_channel(const struct rpma_conn_cfg *cfg, bool *shared);

/** 3
 * rpma_conn_cfg_set_compl_channel - set if the completion event channel can be shared by CQ and RCQ
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_cfg;
 *	int rpma_conn_cfg_set_compl_channel(struct rpma_conn_cfg *cfg, bool shared);
 *
 * DESCRIPTION
 * rpma_conn_cfg_set_compl_channel() sets if the completion event channel can be shared by CQ and
 * RCQ or not. The completion event channel is not shared by CQ and RCQ by default. See
 * rpma_conn_cfg_new(3) for details.
 *
 * RETURN VALUE
 * The rpma_conn_cfg_set_compl_channel() function returns 0 on success or a negative error code on
 * failure.
 *
 * ERRORS
 * rpma_conn_cfg_set_compl_channel() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg is NULL
 *
 * SEE ALSO
 * rpma_conn_cfg_new(3), rpma_conn_wait(3), rpma_conn_cfg_get_compl_channel(3), librpma(7) and
 * https://pmem.io/rpma/
 */
int rpma_conn_cfg_set_compl_channel(struct rpma_conn_cfg *cfg, bool shared);

/** 3
 * rpma_conn_cfg_get_cq_size - get CQ size for the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_cfg;
 *	int rpma_conn_cfg_get_cq_size(const struct rpma_conn_cfg *cfg, uint32_t *cq_size);
 *
 * DESCRIPTION
 * rpma_conn_cfg_get_cq_size() gets the CQ size for the connection.
 *
 * RETURN VALUE
 * The rpma_conn_cfg_get_cq_size() function returns 0 on success or a negative error code on
 * failure. rpma_conn_cfg_get_cq_size() does not set *cq_size value on failure.
 *
 * ERRORS
 * rpma_conn_cfg_get_cq_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg or cq_size is NULL
 *
 * SEE ALSO
 * rpma_conn_cfg_new(3), rpma_conn_cfg_set_cq_size(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_cfg_get_cq_size(const struct rpma_conn_cfg *cfg, uint32_t *cq_size);

/** 3
 * rpma_conn_cfg_set_rcq_size - set receive CQ size for the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_cfg;
 *	int rpma_conn_cfg_set_rcq_size(struct rpma_conn_cfg *cfg, uint32_t rcq_size);
 *
 * DESCRIPTION
 * rpma_conn_cfg_set_rcq_size() sets the receive CQ size for the connection. Please see
 * the rpma_conn_get_rcq() for details about the receive CQ. If this function is not called,
 * the rcq_size has the default value (0) set by rpma_conn_cfg_new(3).
 *
 * RETURN VALUE
 * The rpma_conn_cfg_set_rcq_size() function returns 0 on success or a negative error code on
 * failure.
 *
 * ERRORS
 * rpma_conn_cfg_set_rcq_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg is NULL
 *
 * SEE ALSO
 * rpma_conn_cfg_get_rcq_size(3), rpma_conn_cfg_new(3), rpma_conn_get_rcq(3), librpma(7) and
 * https://pmem.io/rpma/
 */
int rpma_conn_cfg_set_rcq_size(struct rpma_conn_cfg *cfg, uint32_t rcq_size);

/** 3
 * rpma_conn_cfg_get_rcq_size - get receive CQ size for the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_cfg;
 *	int rpma_conn_cfg_get_rcq_size(const struct rpma_conn_cfg *cfg, uint32_t *rcq_size);
 *
 * DESCRIPTION
 * rpma_conn_cfg_get_rcq_size() gets the receive CQ size for the connection.
 * Please see the rpma_conn_get_rcq() for details about the receive CQ.
 *
 * RETURN VALUE
 * The rpma_conn_cfg_get_rcq_size() function returns 0 on success or a negative error code on
 * failure. rpma_conn_cfg_get_rcq_size() does not set *rcq_size value on failure.
 *
 * ERRORS
 * rpma_conn_cfg_get_rcq_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg or rcq_size is NULL
 *
 * SEE ALSO
 * rpma_conn_cfg_new(3), rpma_conn_cfg_set_rcq_size(3), rpma_conn_get_rcq(3), librpma(7) and
 * https://pmem.io/rpma/
 */
int rpma_conn_cfg_get_rcq_size(const struct rpma_conn_cfg *cfg, uint32_t *rcq_size);

/** 3
 * rpma_conn_cfg_set_sq_size - set SQ size for the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_cfg;
 *	int rpma_conn_cfg_set_sq_size(struct rpma_conn_cfg *cfg, uint32_t sq_size);
 *
 * DESCRIPTION
 * rpma_conn_cfg_set_sq_size() sets the SQ size for the connection. If this function is not called,
 * the sq_size has the default value (10) set by rpma_conn_cfg_new(3).
 *
 * RETURN VALUE
 * The rpma_conn_cfg_set_sq_size() function returns 0 on success or a negative error code on
 * failure.
 *
 * ERRORS
 * rpma_conn_cfg_set_sq_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg is NULL
 *
 * SEE ALSO
 * rpma_conn_cfg_new(3), rpma_conn_cfg_get_sq_size(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_cfg_set_sq_size(struct rpma_conn_cfg *cfg, uint32_t sq_size);

/** 3
 * rpma_conn_cfg_get_sq_size - get SQ size for the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_cfg;
 *	int rpma_conn_cfg_get_sq_size(const struct rpma_conn_cfg *cfg, uint32_t *sq_size);
 *
 * DESCRIPTION
 * rpma_conn_cfg_get_sq_size() gets the SQ size for the connection.
 *
 * RETURN VALUE
 * The rpma_conn_cfg_get_sq_size() function returns 0 on success or a negative error code on
 * failure. rpma_conn_cfg_get_sq_size() does not set *sq_size value on failure.
 *
 * ERRORS
 * rpma_conn_cfg_get_sq_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg or sq_size is NULL
 *
 * SEE ALSO
 * rpma_conn_cfg_new(3), rpma_conn_cfg_set_sq_size(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_cfg_get_sq_size(const struct rpma_conn_cfg *cfg, uint32_t *sq_size);

/** 3
 * rpma_conn_cfg_set_rq_size - set RQ size for the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_cfg;
 *	int rpma_conn_cfg_set_rq_size(struct rpma_conn_cfg *cfg, uint32_t rq_size);
 *
 * DESCRIPTION
 * rpma_conn_cfg_set_rq_size() sets the RQ size for the connection. If this function is not called,
 * the rq_size has the default value (10) set by rpma_conn_cfg_new(3).
 *
 * RETURN VALUE
 * The rpma_conn_cfg_set_rq_size() function returns 0 on success or a negative error code on
 * failure.
 *
 * ERRORS
 * rpma_conn_cfg_set_rq_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg is NULL
 *
 * SEE ALSO
 * rpma_conn_cfg_new(3), rpma_conn_cfg_get_rq_size(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_cfg_set_rq_size(struct rpma_conn_cfg *cfg, uint32_t rq_size);

/** 3
 * rpma_conn_cfg_get_rq_size - get RQ size for the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_cfg;
 *	int rpma_conn_cfg_get_rq_size(const struct rpma_conn_cfg *cfg, uint32_t *rq_size);
 *
 * DESCRIPTION
 * rpma_conn_cfg_get_rq_size() gets the RQ size for the connection.
 *
 * RETURN VALUE
 * The rpma_conn_cfg_get_rq_size() function returns 0 on success or a negative error code on
 * failure. rpma_conn_cfg_get_rq_size() does not set *rq_size value on failure.
 *
 * ERRORS
 * rpma_conn_cfg_get_rq_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg or rq_size is NULL
 *
 * SEE ALSO
 * rpma_conn_cfg_new(3), rpma_conn_cfg_set_rq_size(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_cfg_get_rq_size(const struct rpma_conn_cfg *cfg, uint32_t *rq_size);


/* shared RQ */

struct rpma_srq;

/** 3
 * rpma_conn_cfg_set_srq - set a shared RQ object for the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_cfg;
 *	struct rpma_srq;
 *	int rpma_conn_cfg_set_srq(struct rpma_conn_cfg *cfg, struct rpma_srq *srq);
 *
 * DESCRIPTION
 * rpma_conn_cfg_set_srq() sets a shared RQ object for the connection. If this function is not
 * called, the srq has the default value (NULL) set by rpma_conn_cfg_new(3).
 *
 * RETURN VALUE
 * The rpma_conn_cfg_set_srq() function returns 0 on success or a negative error code on failure.
 *
 * ERRORS
 * rpma_conn_cfg_set_srq() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg is NULL
 *
 * SEE ALSO
 * rpma_conn_cfg_new(3), rpma_conn_cfg_get_srq(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_cfg_set_srq(struct rpma_conn_cfg *cfg, struct rpma_srq *srq);

/** 3
 * rpma_conn_cfg_get_srq - get the shared RQ object from the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_cfg;
 *	struct rpma_srq;
 *	int rpma_conn_cfg_get_srq(const struct rpma_conn_cfg *cfg, struct rpma_srq **srq_ptr);
 *
 * DESCRIPTION
 * rpma_conn_cfg_get_srq() gets the shared RQ object from the connection.
 *
 * RETURN VALUE
 * The rpma_conn_cfg_get_srq() function returns 0 on success or a negative error code on failure.
 * rpma_conn_cfg_get_srq() does not set *srq_ptr value on failure.
 *
 * ERRORS
 * rpma_conn_cfg_get_srq() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg or srq_ptr is NULL
 *
 * SEE ALSO
 * rpma_conn_cfg_new(3), rpma_conn_cfg_set_srq(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_cfg_get_srq(const struct rpma_conn_cfg *cfg, struct rpma_srq **srq_ptr);

/* connection */

struct rpma_conn;

/** 3
 * rpma_conn_get_event_fd - get an event file descriptor of the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn;
 *	int rpma_conn_get_event_fd(const struct rpma_conn *conn, int *fd);
 *
 * DESCRIPTION
 * rpma_conn_get_event_fd() gets an event file descriptor of the connection.
 *
 * RETURN VALUE
 * The rpma_conn_get_event_fd() function returns 0 on success or a negative error code on failure.
 * rpma_conn_get_event_fd() does not set *fd value on failure.
 *
 * ERRORS
 * rpma_conn_get_event_fd() can fail with the following error:
 *
 * - RPMA_E_INVAL - conn or fd is NULL
 *
 * SEE ALSO
 * rpma_conn_next_event(3), rpma_conn_req_connect(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_get_event_fd(const struct rpma_conn *conn, int *fd);

enum rpma_conn_event {
	RPMA_CONN_UNDEFINED = -1,	/* Undefined connection event */
	RPMA_CONN_ESTABLISHED,		/* Connection established */
	RPMA_CONN_CLOSED,		/* Connection closed */
	RPMA_CONN_LOST,			/* Connection lost */
	RPMA_CONN_REJECTED,		/* Connection rejected */
	RPMA_CONN_UNREACHABLE		/* Connection unreachable */
};

/** 3
 * rpma_conn_next_event - obtain a connection status
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn;
 *	enum rpma_conn_event {
 *		RPMA_CONN_UNDEFINED = -1,
 *		RPMA_CONN_ESTABLISHED,
 *		RPMA_CONN_CLOSED,
 *		RPMA_CONN_LOST,
 *		RPMA_CONN_REJECTED,
 *		RPMA_CONN_UNREACHABLE
 *	};
 *
 *	int rpma_conn_next_event(struct rpma_conn *conn,
 *			enum rpma_conn_event *event);
 *
 * DESCRIPTION
 * rpma_conn_next_event() obtains the next event from the connection.
 * Types of events:
 * - RPMA_CONN_UNDEFINED - undefined connection event
 * - RPMA_CONN_ESTABLISHED - connection established
 * - RPMA_CONN_CLOSED - connection closed
 * - RPMA_CONN_LOST - connection lost
 * - RPMA_CONN_REJECTED - connection rejected
 * - RPMA_CONN_UNREACHABLE - connection unreachable
 *
 * RETURN VALUE
 * The rpma_conn_next_event() function returns 0 on success or a negative error code on failure.
 *
 * ERRORS
 * rpma_conn_next_event() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn or event is NULL
 * - RPMA_E_UNKNOWN - unexpected event
 * - RPMA_E_PROVIDER - rdma_get_cm_event() or rdma_ack_cm_event() failed
 * - RPMA_E_NOMEM - out of memory
 *
 * SEE ALSO
 * rpma_conn_req_connect(3), rpma_conn_disconnect(3), librpma(7) and https://pmem.io/rpma/
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
 *	enum rpma_conn_event {
 *		RPMA_CONN_UNDEFINED = -1,
 *		RPMA_CONN_ESTABLISHED,
 *		RPMA_CONN_CLOSED,
 *		RPMA_CONN_LOST,
 *		RPMA_CONN_REJECTED,
 *		RPMA_CONN_UNREACHABLE
 *	};
 *
 * DESCRIPTION
 * rpma_utils_conn_event_2str() converts RPMA_CONN_* enum to the const string representation.
 *
 * RETURN VALUE
 * The rpma_utils_conn_event_2str() function returns the const string representation of
 * RPMA_CONN_* enums.
 *
 * ERRORS
 * rpma_utils_conn_event_2str() can not fail.
 *
 * SEE ALSO
 * rpma_conn_next_event(3), librpma(7) and https://pmem.io/rpma/
 */
const char *rpma_utils_conn_event_2str(enum rpma_conn_event conn_event);

struct rpma_conn_private_data {
	void *ptr;
	uint8_t len;
};

/** 3
 * rpma_conn_get_private_data - get a pointer to the connection's private data
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn;
 *	struct rpma_conn_private_data;
 *	int rpma_conn_get_private_data(const struct rpma_conn *conn,
 *			struct rpma_conn_private_data *pdata);
 *
 * DESCRIPTION
 * rpma_conn_get_private_data() obtains the pointer to the private data given by the other side of
 * the connection.
 *
 * RETURN VALUE
 * The rpma_conn_get_private_data() function returns 0 on success or a negative error code on
 * failure. rpma_conn_get_private_data() does not set *pdata value on failure.
 *
 * ERRORS
 * rpma_conn_get_private_data() can fail with the following error:
 *
 * - RPMA_E_INVAL - conn or pdata is NULL
 *
 * SEE ALSO
 * rpma_conn_req_connect(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_get_private_data(const struct rpma_conn *conn, struct rpma_conn_private_data *pdata);

/** 3
 * rpma_conn_apply_remote_peer_cfg - apply remote peer cfg to the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn;
 *	struct rpma_peer_cfg;
 *	int rpma_conn_apply_remote_peer_cfg(struct rpma_conn *conn,
 *			const struct rpma_peer_cfg *pcfg);
 *
 * DESCRIPTION
 * rpma_conn_apply_remote_peer_cfg() applies the remote peer configuration to the connection.
 *
 * RETURN VALUE
 * The rpma_conn_apply_remote_peer_cfg() function returns 0 on success or a negative error code on
 * failure. rpma_conn_apply_remote_peer_cfg() does not set *pcfg value on failure.
 *
 * ERRORS
 * rpma_conn_apply_remote_peer_cfg() can fail with the following error:
 *
 * - RPMA_E_INVAL - conn or pcfg are NULL
 *
 * SEE ALSO
 * rpma_conn_req_connect(3), rpma_peer_cfg_new(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_apply_remote_peer_cfg(struct rpma_conn *conn, const struct rpma_peer_cfg *pcfg);

/** 3
 * rpma_conn_get_qp_num - get the connection's qp_num
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn;
 *	int rpma_conn_get_qp_num(const struct rpma_conn *conn, uint32_t *qp_num);
 *
 * DESCRIPTION
 * rpma_conn_get_qp_num() obtains the unique identifier of the connection.
 *
 * RETURN VALUE
 * The rpma_conn_get_qp_num() function returns 0 on success or a negative error code on failure.
 * rpma_conn_get_qp_num() does not set *qp_num value on failure.
 *
 * ERRORS
 * rpma_conn_get_qp_num() can fail with the following error:
 *
 * - RPMA_E_INVAL - conn or qp_num is NULL
 *
 * SEE ALSO
 * rpma_conn_req_new(3), rpma_ep_next_conn_req(3), rpma_conn_req_connect(3), librpma(7) and
 * https://pmem.io/rpma/
 */
int rpma_conn_get_qp_num(const struct rpma_conn *conn, uint32_t *qp_num);

struct rpma_cq;

/** 3
 * rpma_conn_get_cq - get the connection's main CQ
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn;
 *	struct rpma_cq;
 *	int rpma_conn_get_cq(const struct rpma_conn *conn, struct rpma_cq **cq_ptr);
 *
 * DESCRIPTION
 * rpma_conn_get_cq() gets the main CQ from the connection. When the receive CQ is not present
 * the main CQ allows handling all completions within the connection. When the receive CQ
 * is present the main CQ allows handling all completions except rpma_recv(3) completions within
 * the connection. Please see rpma_conn_get_rcq(3) for details about the receive CQ.
 *
 * RETURN VALUE
 * The rpma_conn_get_cq() function returns 0 on success or a negative error code on failure.
 * rpma_conn_get_cq() does not set *cq_ptr value on failure.
 *
 * ERRORS
 * rpma_conn_get_cq() can fail with the following error:
 *
 * - RPMA_E_INVAL - conn or cq_ptr is NULL
 *
 * SEE ALSO
 * rpma_conn_req_connect(3), rpma_conn_get_rcq(3), rpma_cq_wait(3), rpma_cq_get_wc(3),
 * rpma_cq_get_fd(3), rpma_recv(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_get_cq(const struct rpma_conn *conn, struct rpma_cq **cq_ptr);

/** 3
 * rpma_conn_get_rcq -- get the connection's receive CQ
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn;
 *	struct rpma_cq;
 *	int rpma_conn_get_rcq(const struct rpma_conn *conn, struct rpma_cq **rcq_ptr);
 *
 * DESCRIPTION
 * rpma_conn_get_rcq() gets the receive CQ from the connection. The receive CQ allows handling all
 * rpma_recv(3) completions within the connection. It allows separating rpma_recv(3) completions
 * processing path from all other completions. The receive CQ is created only if the receive CQ
 * size in the provided connection configuration is greater than 0. When the receive CQ does not
 * exist for the given connection the *rcq_ptr == NULL.
 *
 * RETURN VALUE
 * The rpma_conn_get_rcq() function returns 0 on success or a negative error code on failure.
 * rpma_conn_get_rcq() does not set *rcq_ptr value on failure.
 *
 * ERRORS
 * rpma_conn_get_rcq() can fail with the following error:
 *
 * - RPMA_E_INVAL - conn or rcq_ptr is NULL
 *
 * SEE ALSO
 * rpma_conn_cfg_set_rcq_size(3), rpma_conn_req_connect(3), rpma_conn_get_cq(3), rpma_cq_wait(3),
 * rpma_cq_get_wc(3), rpma_cq_get_fd(3), rpma_recv(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_get_rcq(const struct rpma_conn *conn, struct rpma_cq **rcq_ptr);

/** 3
 * rpma_conn_disconnect - tear the connection down
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn;
 *	int rpma_conn_disconnect(struct rpma_conn *conn);
 *
 * DESCRIPTION
 * rpma_conn_disconnect() tears the connection down.
 *
 * - It may initiate disconnecting the connection. In this case, the end of disconnecting is
 *   signalled by the RPMA_CONN_CLOSED event via rpma_conn_next_event() or
 * - It may be called after receiving the RPMA_CONN_CLOSED event. In this case, the disconnection
 *   is done when rpma_conn_disconnect() returns with success.
 *
 * RETURN VALUE
 * The rpma_conn_disconnect() function returns 0 on success or a negative error code on failure.
 *
 * ERRORS
 * rpma_conn_disconnect() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn is NULL
 * - RPMA_E_PROVIDER - rdma_disconnect() failed
 *
 * SEE ALSO
 * rpma_conn_delete(3), rpma_conn_next_event(3), rpma_conn_req_connect(3), librpma(7) and
 * https://pmem.io/rpma/
 */
int rpma_conn_disconnect(struct rpma_conn *conn);

/** 3
 * rpma_conn_delete - delete already closed connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn;
 *	int rpma_conn_delete(struct rpma_conn **conn_ptr);
 *
 * DESCRIPTION
 * rpma_conn_delete() deletes already closed connection.
 *
 * RETURN VALUE
 * The rpma_conn_delete() function returns 0 on success or a negative error code on failure.
 * rpma_conn_delete() sets *conn_ptr value to NULL on success and on failure.
 *
 * ERRORS
 * rpma_conn_delete() can fail with the following errors:
 * - RPMA_E_INVAL - conn_ptr is NULL or munmap() failed
 * - RPMA_E_PROVIDER - ibv_destroy_cq() or rdma_destroy_id() failed
 *
 * SEE ALSO
 * rpma_conn_disconnect(3), rpma_conn_req_connect(3), librpma(7) and https://pmem.io/rpma/
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
 *	struct rpma_peer;
 *	struct rpma_conn_cfg;
 *	struct rpma_conn_req;
 *	int rpma_conn_req_new(struct rpma_peer *peer, const char *addr, const char *port,
 *			const struct rpma_conn_cfg *cfg, struct rpma_conn_req **req_ptr);
 *
 * DESCRIPTION
 * rpma_conn_req_new() creates a new outgoing connection request object using reliable,
 * connection-oriented and message-based (RDMA_PS_TCP) QP communication.
 *
 * RETURN VALUE
 * The rpma_conn_req_new() function returns 0 on success or a negative error code on failure.
 * rpma_conn_req_new() does not set *req_ptr value on failure. If cfg is NULL, then the default
 * values are used
 * - see rpma_conn_cfg_new(3) for more details.
 *
 * ERRORS
 * rpma_conn_req_new() can fail with the following errors:
 *
 * - RPMA_E_INVAL - peer, addr, port or req_ptr is NULL
 * - RPMA_E_NOMEM - out of memory
 * - RPMA_E_PROVIDER - rdma_create_id(3), rdma_resolve_addr(3), rdma_resolve_route(3) or
 *   ibv_create_cq(3) failed
 *
 * SEE ALSO
 * rpma_conn_cfg_new(3), rpma_conn_req_connect(3), rpma_conn_req_delete(3), rpma_conn_req_recv(3),
 * rpma_ep_next_conn_req(3), rpma_peer_new(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_req_new(struct rpma_peer *peer, const char *addr, const char *port,
		const struct rpma_conn_cfg *cfg, struct rpma_conn_req **req_ptr);

/** 3
 * rpma_conn_req_delete - delete the connection requests
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_req;
 *	int rpma_conn_req_delete(struct rpma_conn_req **req_ptr);
 *
 * DESCRIPTION
 * rpma_conn_req_delete() deletes the connection requests both incoming and outgoing.
 *
 * RETURN VALUE
 * The rpma_conn_req_delete() function returns 0 on success or a negative error code on failure.
 * rpma_conn_req_delete() sets *req_ptr value to NULL on success and on failure.
 *
 * ERRORS
 * rpma_conn_req_delete() can fail with the following errors:
 *
 * - RPMA_E_INVAL - req_ptr is NULL
 * - RPMA_E_PROVIDER
 *	- rdma_destroy_qp(3) or ibv_destroy_cq(3) failed
 *	- rdma_reject(3) or rdma_ack_cm_event(3) failed (passive side only)
 *	- rdma_destroy_id(3) failed (active side only)
 *
 * SEE ALSO
 * rpma_conn_req_new(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_req_delete(struct rpma_conn_req **req_ptr);

/** 3
 * rpma_conn_req_connect - initiate processing the connection request
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_req;
 *	struct rpma_conn_private_data;
 *	struct rpma_conn;
 *	int rpma_conn_req_connect(struct rpma_conn_req **req_ptr,
 *			const struct rpma_conn_private_data *pdata, struct rpma_conn **conn_ptr);
 *
 * DESCRIPTION
 * rpma_conn_req_connect() initiates processing the connection requests both incoming and outgoing.
 * The end of processing is signalled by the RPMA_CONN_ESTABLISHED event via
 * rpma_conn_next_event().
 *
 * RETURN VALUE
 * The rpma_conn_req_connect() function returns 0 on success or a negative error code on failure.
 * On success, the newly created connection object is stored in *conn_ptr whereas *req_ptr is
 * consumed and set to NULL. On failure, rpma_conn_req_connect() does not set *conn_ptr whereas
 * *req_ptr is consumed and set to NULL.
 *
 * ERRORS
 * rpma_conn_req_connect() can fail with the following errors:
 *
 * - RPMA_E_INVAL - req_ptr, *req_ptr or conn_ptr is NULL
 * - RPMA_E_INVAL - pdata is not NULL whereas pdata->len == 0
 * - RPMA_E_NOMEM - out of memory
 * - RPMA_E_PROVIDER - initiating a connection request failed (active side only)
 * - RPMA_E_PROVIDER - accepting the connection request failed (passive side only)
 * - RPMA_E_PROVIDER - freeing a communication event failed (passive side only)
 *
 * SEE ALSO
 * rpma_conn_apply_remote_peer_cfg(3), rpma_conn_delete(3), rpma_conn_disconnect(3),
 * rpma_conn_get_cq(3), rpma_conn_get_event_fd(3), rpma_conn_get_private_data(3),
 * rpma_conn_get_rcq(3), rpma_conn_next_event(3), rpma_conn_req_new(3), rpma_ep_next_conn_req(3),
 * rpma_flush(3), rpma_read(3), rpma_recv(3), rpma_send(3), rpma_write(3), rpma_atomic_write(3),
 * librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_req_connect(struct rpma_conn_req **req_ptr,
		const struct rpma_conn_private_data *pdata, struct rpma_conn **conn_ptr);

/** 3
 * rpma_conn_get_compl_fd - get a file descriptor of the shared completion channel of the connection
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn;
 *	int fd;
 *	int rpma_conn_get_compl_fd(const struct rpma_conn *conn, int *fd);
 *
 * DESCRIPTION
 * rpma_conn_get_compl_fd() gets a file descriptor of the shared completion channel from
 * the connection.
 *
 * RETURN VALUE
 * The rpma_conn_get_compl_fd() function returns 0 on success or a negative error code on failure.
 * rpma_conn_get_compl_fd() does not set *fd value on failure.
 *
 * ERRORS
 * rpma_conn_get_compl_fd() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn or fd is NULL
 * - RPMA_E_NOT_SHARED_CHNL - the completion event channel is not shared
 *
 * SEE ALSO
 * librpma(7), rpma_conn_req_connect(3) and https://pmem.io/rpma/
 */
int rpma_conn_get_compl_fd(const struct rpma_conn *conn, int *fd);

/** 3
 * rpma_conn_wait - wait for a completion event on the shared completion channel from CQ or RCQ
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn;
 *	struct rpma_cq
 *	int rpma_conn_wait(struct rpma_conn *conn, int flags, struct rpma_cq **cq, bool *is_rcq)
 *
 * DESCRIPTION
 * rpma_conn_wait() waits for a completion event on the shared completion channel from CQ or RCQ,
 * acks it and returns a CQ that caused the event in the cq argument and a boolean value saying if
 * it is RCQ or not in the is_rcq argument (if is_rcq is not NULL). If rpma_conn_wait() succeeds,
 * then all available completions should be collected from the returned cq using rpma_cq_get_wc(3).
 *
 * RETURN VALUE
 * The rpma_conn_wait() function returns 0 on success or a negative error code on failure.
 *
 * ERRORS
 * rpma_conn_wait() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn or cq are NULL
 * - RPMA_E_NOT_SHARED_CHNL - the completion event channel is not shared
 * - RPMA_E_NO_COMPLETION - ibv_get_cq_event(3) failed
 * - RPMA_E_UNKNOWN - ibv_get_cq_event(3) returned unknown CQ
 * - RPMA_E_PROVIDER - ibv_req_notify_cq(3) failed
 *
 * SEE ALSO
 * rpma_conn_req_new(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_wait(struct rpma_conn *conn, int flags, struct rpma_cq **cq, bool *is_rcq);

/** 3
 * rpma_conn_req_recv - initiate the receive operation
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_req;
 *	struct rpma_mr_local;
 *	int rpma_conn_req_recv(struct rpma_conn_req *req, struct rpma_mr_local *dst, size_t offset,
 *			size_t len, const void *op_context);
 *
 * DESCRIPTION
 * rpma_conn_req_recv() initiates the receive operation. It prepares a buffer for a message sent
 * from other side of the connection. Please see rpma_send(3). This is a variant of rpma_recv(3)
 * which may be used before the connection is established. op_context is returned in the wr_id
 * field of the completion (struct ibv_wc).
 *
 * RETURN VALUE
 * The rpma_conn_req_recv() function returns 0 on success or a negative error code on failure.
 *
 * ERRORS
 * rpma_conn_req_recv() can fail with the following errors:
 *
 * - RPMA_E_INVAL - req or src or op_context is NULL
 * - RPMA_E_PROVIDER - ibv_post_recv(3) failed
 *
 * SEE ALSO
 * rpma_conn_req_new(3), rpma_mr_reg(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_req_recv(struct rpma_conn_req *req, struct rpma_mr_local *dst, size_t offset,
		size_t len, const void *op_context);

/* server-side setup */

struct rpma_ep;

/** 3
 * rpma_ep_listen - create a listening endpoint
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_peer;
 *	struct rpma_ep;
 *	int rpma_ep_listen(struct rpma_peer *peer, const char *addr, const char *port,
 *			struct rpma_ep **ep_ptr);
 *
 * DESCRIPTION
 * rpma_ep_listen() creates an endpoint and initiates listening for incoming connections using
 * reliable, connection-oriented and message-based (RDMA_PS_TCP) QP communication.
 *
 * RETURN VALUE
 * The rpma_ep_listen() function returns 0 on success or a negative error code on failure.
 * rpma_ep_listen() does not set *ep_ptr value on failure.
 *
 * ERRORS
 * rpma_ep_listen() can fail with the following errors:
 *
 * - RPMA_E_INVAL - peer, addr, port or ep_ptr is NULL
 * - RPMA_E_PROVIDER - rdma_create_event_channel(3), rdma_create_id(3),
 *   rdma_getaddrinfo(3), rdma_listen(3) failed
 * - RPMA_E_NOMEM - out of memory
 *
 * SEE ALSO
 * rpma_ep_get_fd(3), rpma_ep_next_conn_req(3), rpma_ep_shutdown(3), rpma_peer_new(3), librpma(7)
 * and https://pmem.io/rpma/
 */
int rpma_ep_listen(struct rpma_peer *peer, const char *addr, const char *port,
		struct rpma_ep **ep_ptr);

/** 3
 * rpma_ep_shutdown - stop listening and delete the endpoint
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_ep;
 *	int rpma_ep_shutdown(struct rpma_ep **ep_ptr);
 *
 * DESCRIPTION
 * rpma_ep_shutdown() stops listening for incoming connections and deletes the endpoint.
 * The connections established using the endpoint will still exist after deleting the endpoint.
 *
 * RETURN VALUE
 * The rpma_ep_shutdown() function returns 0 on success or a negative error code on failure.
 * rpma_ep_shutdown() does not set *ep_ptr value to NULL on failure.
 *
 * ERRORS
 * rpma_ep_shutdown() can fail with the following errors:
 *
 * - RPMA_E_INVAL - ep_ptr is NULL
 * - RPMA_E_PROVIDER - rdma_destroy_id(3) failed
 *
 * SEE ALSO
 * rpma_ep_listen(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_ep_shutdown(struct rpma_ep **ep_ptr);

/** 3
 * rpma_ep_get_fd - get a file descriptor of the endpoint
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_ep;
 *	int rpma_ep_get_fd(const struct rpma_ep *ep, int *fd);
 *
 * DESCRIPTION
 * rpma_ep_get_fd() gets the file descriptor of the endpoint.
 *
 * RETURN VALUE
 * The rpma_ep_get_fd() function returns 0 on success or a negative error code on failure.
 * rpma_ep_get_fd() does not set *fd value on failure.
 *
 * ERRORS
 * rpma_ep_get_fd() can fail with the following error:
 *
 * - RPMA_E_INVAL - ep or fd is NULL
 *
 * SEE ALSO
 * rpma_ep_listen(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_ep_get_fd(const struct rpma_ep *ep, int *fd);

/** 3
 * rpma_ep_next_conn_req - obtain an incoming connection request
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_ep;
 *	struct rpma_conn_cfg;
 *	struct rpma_conn_req;
 *	int rpma_ep_next_conn_req(struct rpma_ep *ep, const struct rpma_conn_cfg *cfg,
 *			struct rpma_conn_req **req_ptr);
 *
 * DESCRIPTION
 * rpma_ep_next_conn_req() obtains the next connection request from the endpoint.
 *
 * RETURN VALUE
 * The rpma_ep_next_conn_req() function returns 0 on success or a negative error code on failure.
 * rpma_ep_next_conn_req() does not set *req_ptr value on failure.
 *
 * ERRORS
 * rpma_ep_next_conn_req() can fail with the following errors:
 *
 * - RPMA_E_INVAL - ep or req_ptr is NULL
 * - RPMA_E_INVAL - obtained an event different than a connection request
 * - RPMA_E_PROVIDER - rdma_get_cm_event(3) failed
 * - RPMA_E_NOMEM - out of memory
 * - RPMA_E_NO_EVENT - no next connection request available
 *
 * SEE ALSO
 * rpma_conn_cfg_new(3), rpma_conn_req_delete(3), rpma_conn_req_connect(3), rpma_ep_listen(3),
 * librpma(7) and https://pmem.io/rpma/
 */
int rpma_ep_next_conn_req(struct rpma_ep *ep, const struct rpma_conn_cfg *cfg,
		struct rpma_conn_req **req_ptr);

/** 3
 * rpma_conn_req_get_private_data - get a pointer to the request's private data
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn_req;
 *	struct rpma_conn_private_data;
 *	int rpma_conn_req_get_private_data(const struct rpma_conn_req *req,
 *			struct rpma_conn_private_data *pdata);
 *
 * DESCRIPTION
 * rpma_conn_req_get_private_data() obtains the pointer to the connection's private data given by
 * the other side of the connection before the connection is established.
 *
 * RETURN VALUE
 * The rpma_conn_req_get_private_data() function returns 0 on success or a negative error code on
 * failure. rpma_conn_req_get_private_data() does not set *pdata value on failure.
 *
 * ERRORS
 * rpma_conn_req_get_private_data() can fail with the following error:
 *
 * - RPMA_E_INVAL - req or pdata is NULL
 *
 * SEE ALSO
 * rpma_conn_get_private_data(3), rpma_ep_next_conn_req(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_conn_req_get_private_data(const struct rpma_conn_req *req,
		struct rpma_conn_private_data *pdata);

/* shared RQ configuration */

struct rpma_srq_cfg;

/** 3
 * rpma_srq_cfg_new - create a new shared RQ configuration object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_srq_cfg;
 *	int rpma_srq_cfg_new(struct rpma_srq_cfg **cfg_ptr);
 *
 * DESCRIPTION
 * rpma_srq_cfg_new() creates a new shared RQ configuration object and fills it with the default
 * values:
 *
 *	.rcq_size = 100
 *	.rq_size = 100
 *
 * Note that rpma_srq_new(3) with the default rcq_size creates its own receive CQ.
 *
 * RETURN VALUE
 * The rpma_srq_cfg_new() function returns 0 on success or a negative error code on failure.
 * rpma_srq_cfg_new() does not set *cfg_ptr value on failure.
 *
 * ERRORS
 * rpma_srq_cfg_new() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg_ptr is NULL
 * - RPMA_E_NOMEM - out of memory
 *
 * SEE ALSO
 * rpma_srq_cfg_delete(3), rpma_srq_cfg_get_rcq_size(3), rpma_srq_cfg_get_rq_size(3),
 * rpma_srq_cfg_set_rcq_size(3), rpma_srq_cfg_set_rq_size(3), rpma_srq_new(3), librpma(7) and
 * https://pmem.io/rpma/
 */
int rpma_srq_cfg_new(struct rpma_srq_cfg **cfg_ptr);

/** 3
 * rpma_srq_cfg_delete - delete the shared RQ configuration object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_srq_cfg;
 *	int rpma_srq_cfg_delete(struct rpma_srq_cfg **cfg_ptr);
 *
 * DESCRIPTION
 * rpma_srq_cfg_delete() deletes the shared RQ configuration object.
 *
 * RETURN VALUE
 * The rpma_srq_cfg_delete() function returns 0 on success or a negative error code on failure.
 * rpma_srq_cfg_delete() sets *cfg_ptr value to NULL on success and on failure.
 *
 * ERRORS
 * rpma_srq_cfg_delete() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg_ptr is NULL
 *
 * SEE ALSO
 * rpma_srq_cfg_new(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_srq_cfg_delete(struct rpma_srq_cfg **cfg_ptr);

/** 3
 * rpma_srq_cfg_set_rq_size - set RQ size of the shared RQ
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_srq_cfg;
 *	int rpma_srq_cfg_set_rq_size(struct rpma_srq_cfg *cfg, uint32_t rq_size);
 *
 * DESCRIPTION
 * rpma_srq_cfg_set_rq_size() sets the RQ size of the shared RQ. If this function is not called,
 * the rq_size has the default value (100) set by rpma_srq_cfg_new(3).
 *
 * RETURN VALUE
 * The rpma_srq_cfg_set_rq_size() function returns 0 on success or a negative error code on
 * failure.
 *
 * ERRORS
 * rpma_srq_cfg_set_rq_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg is NULL
 *
 * SEE ALSO
 * rpma_srq_cfg_new(3), rpma_srq_cfg_get_rq_size(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_srq_cfg_set_rq_size(struct rpma_srq_cfg *cfg, uint32_t rq_size);

/** 3
 * rpma_srq_cfg_get_rq_size - get RQ size of the shared RQ
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_srq_cfg;
 *	int rpma_srq_cfg_get_rq_size(const struct rpma_srq_cfg *cfg, uint32_t *rq_size);
 *
 * DESCRIPTION
 * rpma_srq_cfg_get_rq_size() gets the RQ size of the shared RQ.
 *
 * RETURN VALUE
 * The rpma_srq_cfg_get_rq_size() function returns 0 on success or a negative error code on
 * failure. rpma_srq_cfg_get_rq_size() does not set *rq_size value on failure.
 *
 * ERRORS
 * rpma_srq_cfg_get_rq_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg or rq_size is NULL
 *
 * SEE ALSO
 * rpma_srq_cfg_new(3), rpma_srq_cfg_set_rq_size(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_srq_cfg_get_rq_size(const struct rpma_srq_cfg *cfg, uint32_t *rq_size);

/** 3
 * rpma_srq_cfg_set_rcq_size - set receive CQ size of the shared RQ
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_srq_cfg;
 *	int rpma_srq_cfg_set_rcq_size(struct rpma_srq_cfg *cfg, uint32_t rcq_size);
 *
 * DESCRIPTION
 * rpma_srq_cfg_set_rcq_size() sets the receive CQ size of the shared RQ. If this function is not
 * called, the rcq_size has the default value (100) set by rpma_srq_cfg_new(3).
 *
 * Note that rpma_srq_new(3) does not create its own receive CQ if the size of the receive CQ
 * equals 0.
 *
 * RETURN VALUE
 * The rpma_srq_cfg_set_rcq_size() function returns 0 on success or a negative error code on
 * failure.
 *
 * ERRORS
 * rpma_srq_cfg_set_rcq_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg is NULL
 *
 * SEE ALSO
 * rpma_srq_cfg_get_rcq_size(3), rpma_srq_cfg_new(3), rpma_srq_new(3), librpma(7) and
 * https://pmem.io/rpma/
 */
int rpma_srq_cfg_set_rcq_size(struct rpma_srq_cfg *cfg, uint32_t rcq_size);

/** 3
 * rpma_srq_cfg_get_rcq_size - get receive CQ size of the shared RQ
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_srq_cfg;
 *	int rpma_srq_cfg_get_rcq_size(const struct rpma_srq_cfg *cfg, uint32_t *rcq_size);
 *
 * DESCRIPTION
 * rpma_srq_cfg_get_rcq_size() gets the receive CQ size of the shared RQ.
 *
 * RETURN VALUE
 * The rpma_srq_cfg_get_rcq_size() function returns 0 on success or a negative error code on
 * failure. rpma_srq_cfg_get_rcq_size() does not set *rcq_size value on failure.
 *
 * ERRORS
 * rpma_srq_cfg_get_rcq_size() can fail with the following error:
 *
 * - RPMA_E_INVAL - cfg or rcq_size is NULL
 *
 * SEE ALSO
 * rpma_srq_cfg_new(3), rpma_srq_cfg_set_rcq_size(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_srq_cfg_get_rcq_size(const struct rpma_srq_cfg *cfg, uint32_t *rcq_size);

/* shared RQ */

/** 3
 * rpma_srq_new - create a new shared RQ object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_peer;
 *	struct rpma_srq_cfg;
 *	struct rpma_srq;
 *	int rpma_srq_new(struct rpma_peer *peer, const struct rpma_srq_cfg *cfg,
 *			struct rpma_srq **srq_ptr);
 *
 * DESCRIPTION
 * rpma_srq_new() creates a new shared RQ object including a new shared RQ and a new shared receive
 * CQ. It does not create the shared receive CQ if the size of the receive CQ in cfg equals 0.
 *
 * RETURN VALUE
 * The rpma_srq_new() function returns 0 on success or a negative error code on failure.
 * rpma_srq_new() does not set *srq_ptr value on failure. If cfg is NULL, then the default values
 * are used
 * - see rpma_srq_cfg_new(3) for more details.
 *
 * ERRORS
 * rpma_srq_new() can fail with the following errors:
 *
 * - RPMA_E_INVAL - peer or srq_ptr is NULL
 * - RPMA_E_NOMEM - out of memory
 * - RPMA_E_PROVIDER - ibv_create_srq(3), ibv_create_comp_channel(3), ibv_create_cq(3) or
 *   ibv_req_notify_cq(3) failed
 *
 * SEE ALSO
 * rpma_srq_delete(3), rpma_srq_get_rcq(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_srq_new(struct rpma_peer *peer, struct rpma_srq_cfg *cfg, struct rpma_srq **srq_ptr);

/** 3
 * rpma_srq_delete - delete the shared RQ object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_srq;
 *	int rpma_conn_req_delete(struct rpma_srq **srq_ptr);
 *
 * DESCRIPTION
 * rpma_srq_delete() deletes the shared RQ object.
 *
 * RETURN VALUE
 * The rpma_srq_delete() function returns 0 on success or a negative error code on failure.
 * rpma_srq_delete() sets *srq_ptr value to NULL on success and on failure.
 *
 * ERRORS
 * rpma_srq_delete() can fail with the following errors:
 *
 * - RPMA_E_INVAL - srq_ptr is NULL
 * - RPMA_E_PROVIDER - ibv_destroy_cq(3), ibv_destroy_comp_channel(3), or ibv_destroy_srq(3) failed
 *
 * SEE ALSO
 * rpma_srq_new(3), rpma_srq_get_rcq(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_srq_delete(struct rpma_srq **srq_ptr);

/** 3
 * rpma_srq_recv - initiate the receive operation in shared RQ
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_srq;
 *	struct rpma_mr_local;
 *	int rpma_srq_recv(struct rpma_srq *srq, struct rpma_mr_local *dst, size_t offset,
 *			size_t len, const void *op_context);
 *
 * DESCRIPTION
 * If multiple local connections use a shared RQ, rpma_srq_recv() initiates the receive operation
 * which prepares a buffer for a message sent from other side of these connections. Please see
 * rpma_send(3).
 *
 * All buffers prepared via rpma_srq_recv(3) form an unordered set. When a message arrives it is
 * placed in one of the buffers awaiting and a completion for the receive operation is generated.
 *
 * A buffer for an incoming message has to be prepared beforehand.
 *
 * The order of buffers in the set does not affect the order of completions of the receive
 * operations got via rpma_cq_get_wc(3).
 *
 * op_context is returned in the wr_id field of the completion (struct ibv_wc).
 *
 * NOTE
 * In the RDMA standard, receive requests form an ordered queue.
 * The librpma library does NOT inherit this guarantee.
 *
 * RETURN VALUE
 * The rpma_srq_recv() function returns 0 on success or a negative error code on failure.
 *
 * ERRORS
 * rpma_srq_recv() can fail with the following errors:
 *
 * - RPMA_E_INVAL - srq == NULL
 * - RPMA_E_INVAL - dst == NULL && (offset != 0 || len != 0)
 * - RPMA_E_PROVIDER - ibv_post_srq_recv(3) failed
 *
 * SEE ALSO
 * rpma_mr_reg(3), rpma_srq_new(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_srq_recv(struct rpma_srq *srq, struct rpma_mr_local *dst, size_t offset, size_t len,
	const void *op_context);

/** 3
 * rpma_srq_get_rcq -- get the receive CQ from the shared RQ object
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_srq;
 *	struct rpma_cq;
 *	int rpma_srq_get_rcq(const struct rpma_srq *srq, struct rpma_cq **rcq_ptr);
 *
 * DESCRIPTION
 * rpma_srq_get_rcq() gets the receive CQ from the shared RQ object. The receive CQ created by
 * rpma_srq_new(3) allows handling all rpma_srq_recv(3) completions within the shared RQ.
 * rpma_srq_cfg_set_rcq_size(3) can change the receive CQ size.
 *
 * RETURN VALUE
 * The rpma_srq_get_rcq() function returns 0 on success or a negative error code on failure.
 * rpma_srq_get_rcq() does not set *rcq_ptr value on failure.
 *
 * ERRORS
 * rpma_srq_get_rcq() can fail with the following error:
 *
 * - RPMA_E_INVAL - srq or rcq_ptr is NULL
 *
 * SEE ALSO
 * rpma_cq_wait(3), rpma_cq_get_wc(3), rpma_cq_get_fd(3), rpma_srq_cfg_set_rcq_size(3),
 * rpma_srq_new(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_srq_get_rcq(const struct rpma_srq *srq, struct rpma_cq **rcq_ptr);

/* remote memory access functions */

/* generate operation completion on error */
#define RPMA_F_COMPLETION_ON_ERROR	(1 << 0)
/* generate operation completion regardless of its result */
#define RPMA_F_COMPLETION_ALWAYS	(1 << 1 | RPMA_F_COMPLETION_ON_ERROR)

/** 3
 * rpma_read - initiate the read operation
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn;
 *	struct rpma_mr_local;
 *	struct rpma_mr_remote;
 *	int rpma_read(struct rpma_conn *conn,
 *			struct rpma_mr_local *dst, size_t dst_offset,
 *			const struct rpma_mr_remote *src, size_t src_offset,
 *			size_t len, int flags, const void *op_context);
 *
 * DESCRIPTION
 * rpma_read() initiates transferring data from the remote memory to the local memory. To read a 0
 * bytes message, set src and dst to NULL and src_offset, dst_offset and len to 0.
 *
 * The attribute flags set the completion notification indicator:
 * - RPMA_F_COMPLETION_ON_ERROR - generate the completion on error
 * - RPMA_F_COMPLETION_ALWAYS - generate the completion regardless of result of the operation.
 *
 * op_context is returned in the wr_id field of the completion (struct ibv_wc).
 *
 * RETURN VALUE
 * The rpma_read() function returns 0 on success or a negative error code on failure.
 *
 * ERRORS
 * rpma_read() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn == NULL || flags == 0
 * - RPMA_E_INVAL - dst == NULL && (src != NULL || src_offset != 0 || dst_offset != 0 || len != 0)
 * - RPMA_E_INVAL - src == NULL && (dst != NULL || src_offset != 0 || dst_offset != 0 || len != 0)
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 *
 * SEE ALSO
 * rpma_conn_req_connect(3), rpma_mr_reg(3), rpma_mr_remote_from_descriptor(3), librpma(7) and
 * https://pmem.io/rpma/
 */
int rpma_read(struct rpma_conn *conn,
		struct rpma_mr_local *dst, size_t dst_offset,
		const struct rpma_mr_remote *src, size_t src_offset,
		size_t len, int flags, const void *op_context);

/** 3
 * rpma_write - initiate the write operation
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn;
 *	struct rpma_mr_local;
 *	struct rpma_mr_remote;
 *	int rpma_write(struct rpma_conn *conn,
 *			struct rpma_mr_remote *dst, size_t dst_offset,
 *			const struct rpma_mr_local *src, size_t src_offset,
 *			size_t len, int flags, const void *op_context);
 *
 * DESCRIPTION
 * rpma_write() initiates transferring data from the local memory to the remote memory. To write
 * a 0 bytes message, set src and dst to NULL and src_offset, dst_offset and len to 0.
 *
 * The attribute flags set the completion notification indicator:
 * - RPMA_F_COMPLETION_ON_ERROR - generate the completion on error
 * - RPMA_F_COMPLETION_ALWAYS - generate the completion regardless of result of the operation.
 *
 * op_context is returned in the wr_id field of the completion (struct ibv_wc).
 *
 * RETURN VALUE
 * The rpma_write() function returns 0 on success or a negative error code on failure.
 *
 * ERRORS
 * rpma_write() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn == NULL || flags == 0
 * - RPMA_E_INVAL - dst == NULL && (src != NULL || src_offset != 0 || dst_offset != 0 || len != 0)
 * - RPMA_E_INVAL - src == NULL && (dst != NULL || src_offset != 0 || dst_offset != 0 || len != 0)
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 *
 * SEE ALSO
 * rpma_conn_req_connect(3), rpma_mr_reg(3), rpma_mr_remote_from_descriptor(3), librpma(7) and
 * https://pmem.io/rpma/
 */
int rpma_write(struct rpma_conn *conn,
		struct rpma_mr_remote *dst, size_t dst_offset,
		const struct rpma_mr_local *src, size_t src_offset,
		size_t len, int flags, const void *op_context);

/** 3
 * rpma_write_with_imm - initiate the write operation with immediate data
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn;
 *	struct rpma_mr_local;
 *	struct rpma_mr_remote;
 *	int rpma_write_with_imm(struct rpma_conn *conn,
 *			struct rpma_mr_remote *dst, size_t dst_offset,
 *			const struct rpma_mr_local *src, size_t src_offset,
 *			size_t len, int flags, uint32_t imm,
 *			const void *op_context);
 *
 * DESCRIPTION
 * rpma_write_with_imm() initiates the write operation with immediate data (transferring data from
 * the local memory to the remote memory. To write a 0 bytes message, set src and dst to NULL and
 * src_offset, dst_offset and len to 0.
 *
 * The attribute flags set the completion notification indicator:
 * - RPMA_F_COMPLETION_ON_ERROR - generate the completion on error
 * - RPMA_F_COMPLETION_ALWAYS - generate the completion regardless of result of the operation.
 *
 * op_context is returned in the wr_id field of the completion (struct ibv_wc).
 *
 * RETURN VALUE
 * The rpma_write_with_imm() function returns 0 on success or a negative error code on failure.
 *
 * ERRORS
 * rpma_write_with_imm() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn == NULL || flags == 0
 * - RPMA_E_INVAL - dst == NULL && (src != NULL || src_offset != 0 || dst_offset != 0 || len != 0)
 * - RPMA_E_INVAL - src == NULL && (dst != NULL || src_offset != 0 || dst_offset != 0 || len != 0)
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 *
 * SEE ALSO
 * rpma_conn_req_connect(3), rpma_mr_reg(3), rpma_mr_remote_from_descriptor(3), librpma(7) and
 * https://pmem.io/rpma/
 */
int rpma_write_with_imm(struct rpma_conn *conn,
		struct rpma_mr_remote *dst, size_t dst_offset,
		const struct rpma_mr_local *src, size_t src_offset,
		size_t len, int flags, uint32_t imm, const void *op_context);

#define RPMA_ATOMIC_WRITE_ALIGNMENT 8

/** 3
 * rpma_atomic_write -- initiate the atomic 8 bytes write operation
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn;
 *	struct rpma_mr_remote;
 *	int rpma_atomic_write(struct rpma_conn *conn,
 *			struct rpma_mr_remote *dst, size_t dst_offset,
 *			const char src[8], int flags, const void *op_context);
 *
 * DESCRIPTION
 * rpma_atomic_write() initiates the atomic 8 bytes write operation (transferring data from
 * the local memory to the remote memory). The atomic write operation allows transferring exactly 8
 * bytes of data and storing them atomically in the remote memory.
 *
 * The attribute flags set the completion notification indicator:
 * - RPMA_F_COMPLETION_ON_ERROR - generate the completion on error
 * - RPMA_F_COMPLETION_ALWAYS - generate the completion regardless of result of the operation.
 *
 * op_context is returned in the wr_id field of the completion (struct ibv_wc).
 *
 * RETURN VALUE
 * The rpma_atomic_write() function returns 0 on success or a negative error code on failure.
 *
 * ERRORS
 * rpma_atomic_write() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn, dst or src is NULL
 * - RPMA_E_INVAL - dst_offset is not aligned to 8 bytes
 * - RPMA_E_INVAL - flags are not set (flags == 0)
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 *
 * SEE ALSO
 * rpma_conn_req_connect(3), rpma_mr_reg(3), rpma_mr_remote_from_descriptor(3), librpma(7) and
 * https://pmem.io/rpma/
 */
int rpma_atomic_write(struct rpma_conn *conn,
	struct rpma_mr_remote *dst, size_t dst_offset,
	const char src[8], int flags, const void *op_context);

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
 * rpma_flush - initiate the flush operation
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn;
 *	struct rpma_mr_remote;
 *	enum rpma_flush_type {
 *		RPMA_FLUSH_TYPE_PERSISTENT,
 *		RPMA_FLUSH_TYPE_VISIBILITY,
 *	};
 *
 *	int rpma_flush(struct rpma_conn *conn,
 *			struct rpma_mr_remote *dst, size_t dst_offset,
 *			size_t len, enum rpma_flush_type type, int flags,
 *			const void *op_context);
 *
 * DESCRIPTION
 * rpma_flush() initiates the flush operation (finalizing a transfer of data to the remote memory).
 * Possible types of rpma_flush() operation:
 * - RPMA_FLUSH_TYPE_PERSISTENT - flush data down to the persistent domain
 * - RPMA_FLUSH_TYPE_VISIBILITY - flush data deep enough to make it visible on the remote node
 *
 * The attribute flags set the completion notification indicator:
 * - RPMA_F_COMPLETION_ON_ERROR - generate the completion on error
 * - RPMA_F_COMPLETION_ALWAYS - generate the completion regardless of result of the operation.
 *
 * op_context is returned in the wr_id field of the completion (struct ibv_wc).
 *
 * RETURN VALUE
 * The rpma_flush() function returns 0 on success or a negative error code on failure.
 *
 * ERRORS
 * rpma_flush() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn or dst is NULL
 * - RPMA_E_INVAL - unknown type value
 * - RPMA_E_INVAL - flags are not set
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 * - RPMA_E_NOSUPP - type is RPMA_FLUSH_TYPE_PERSISTENT and the direct write to pmem is not
 *   supported
 *
 * SEE ALSO
 * rpma_conn_req_connect(3), rpma_mr_remote_from_descriptor(3), librpma(7) and
 * https://pmem.io/rpma/
 */
int rpma_flush(struct rpma_conn *conn,
		struct rpma_mr_remote *dst, size_t dst_offset, size_t len,
		enum rpma_flush_type type, int flags, const void *op_context);

/** 3
 * rpma_send - initiate the send operation
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn;
 *	struct rpma_mr_local;
 *	int rpma_send(struct rpma_conn *conn, const struct rpma_mr_local *src, size_t offset,
 *			size_t len, int flags, const void *op_context);
 *
 * DESCRIPTION
 * rpma_send() initiates the send operation which transfers a message from the local memory to
 * other side of the connection. To send a 0 byte message, set src to NULL and both offset and len
 * to 0.
 *
 * The attribute flags set the completion notification indicator:
 * - RPMA_F_COMPLETION_ON_ERROR - generate the completion on error
 * - RPMA_F_COMPLETION_ALWAYS - generate the completion regardless of result of the operation.
 *
 * op_context is returned in the wr_id field of the completion (struct ibv_wc).
 *
 * RETURN VALUE
 * The rpma_send() function returns 0 on success or a negative error code on failure.
 *
 * ERRORS
 * rpma_send() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn == NULL || flags == 0
 * - RPMA_E_INVAL - src == NULL && (offset != 0 || len != 0)
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 *
 * SEE ALSO
 * rpma_conn_req_connect(3), rpma_mr_reg(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_send(struct rpma_conn *conn, const struct rpma_mr_local *src, size_t offset, size_t len,
		int flags, const void *op_context);

/** 3
 * rpma_send_with_imm - initiate the send operation with immediate data
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn;
 *	struct rpma_mr_local;
 *	int rpma_send_with_imm(struct rpma_conn *conn, const struct rpma_mr_local *src,
 *			size_t offset, size_t len, int flags, uint32_t imm,
 *			const void *op_context);
 *
 * DESCRIPTION
 * rpma_send_with_imm() initiates the send operation with immediate data which transfers a message
 * from the local memory to other side of the connection. To send a 0 byte message, set src to NULL
 * and both offset and len to 0.
 *
 * The attribute flags set the completion notification indicator:
 * - RPMA_F_COMPLETION_ON_ERROR - generate the completion on error
 * - RPMA_F_COMPLETION_ALWAYS - generate the completion regardless of result of the operation.
 *
 * op_context is returned in the wr_id field of the completion (struct ibv_wc).
 *
 * RETURN VALUE
 * The rpma_send_with_imm() function returns 0 on success or a negative error code on failure.
 *
 * ERRORS
 * rpma_send_with_imm() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn == NULL || flags == 0
 * - RPMA_E_INVAL - src == NULL && (offset != 0 || len != 0)
 * - RPMA_E_PROVIDER - ibv_post_send(3) failed
 *
 * SEE ALSO
 * rpma_conn_req_connect(3), rpma_mr_reg(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_send_with_imm(struct rpma_conn *conn, const struct rpma_mr_local *src, size_t offset,
	size_t len, int flags, uint32_t imm, const void *op_context);

/** 3
 * rpma_recv - initiate the receive operation
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_conn;
 *	struct rpma_mr_local;
 *	int rpma_recv(struct rpma_conn *conn, struct rpma_mr_local *dst, size_t offset, size_t len,
 *			const void *op_context);
 *
 * DESCRIPTION
 * rpma_recv() initiates the receive operation which prepares a buffer for a message sent from
 * other side of the connection. Please see rpma_send(3).
 *
 * All buffers prepared via rpma_recv(3) form an unordered set. When a message arrives it is placed
 * in one of the buffers awaitaning and a completion for the receive operation is generated.
 *
 * A buffer for an incoming message have to be prepared beforehand.
 *
 * The order of buffers in the set does not affect the order of completions of receive operations
 * get via rpma_cq_get_wc(3).
 *
 * op_context is returned in the wr_id field of the completion (struct ibv_wc).
 *
 * NOTE
 * In the RDMA standard, receive requests form an ordered queue.
 * The RPMA does NOT inherit this guarantee.
 *
 * RETURN VALUE
 * The rpma_recv() function returns 0 on success or a negative error code on failure.
 *
 * ERRORS
 * rpma_recv() can fail with the following errors:
 *
 * - RPMA_E_INVAL - conn == NULL
 * - RPMA_E_INVAL - dst == NULL && (offset != 0 || len != 0)
 * - RPMA_E_PROVIDER - ibv_post_recv(3) failed
 *
 * SEE ALSO
 * rpma_conn_req_connect(3), rpma_mr_reg(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_recv(struct rpma_conn *conn, struct rpma_mr_local *dst, size_t offset, size_t len,
		const void *op_context);

/* completion handling */

/** 3
 * rpma_cq_get_fd - get the completion queue's file descriptor
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_cq;
 *	int rpma_cq_get_fd(const struct rpma_cq *cq, int *fd);
 *
 * DESCRIPTION
 * rpma_cq_get_fd() gets the file descriptor of the completion queue (CQ in short). When a next
 * completion in the CQ is ready to be consumed by rpma_cq_get_wc(3), the notification is delivered
 * via the file descriptor. The default mode of the file descriptor is blocking but it can be
 * changed to non-blocking mode using fcntl(2). The CQ is either the connection's main CQ or
 * the receive CQ or CQ of shared RQ, please see rpma_conn_get_cq(3), rpma_conn_get_rcq(3) or
 * rpma_srq_get_rcq() for details.
 *
 * Note after spotting the notification using the provided file descriptor you do not have to call
 * rpma_cq_wait(3) before consuming the completion but it may cause that the next call to
 * rpma_cq_wait(3) will notify you of already consumed completion.
 *
 * RETURN VALUE
 * The rpma_cq_get_fd() function returns 0 on success or a negative error code on failure.
 * rpma_cq_get_fd() does not set *fd value on failure.
 *
 * ERRORS
 * rpma_cq_get_fd() can fail with the following error:
 *
 * - RPMA_E_INVAL - cq or fd is NULL
 *
 * SEE ALSO
 * fcntl(2), rpma_conn_get_cq(3), rpma_conn_get_rcq(3), rpma_srq_get_rcq(3), rpma_cq_wait(3),
 * rpma_cq_get_wc(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_cq_get_fd(const struct rpma_cq *cq, int *fd);

/** 3
 * rpma_cq_wait - wait for a completion and ack it
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_cq;
 *	int rpma_cq_wait(struct rpma_cq *cq);
 *
 * DESCRIPTION
 * rpma_cq_wait() waits for an incoming completion event and acks it. If rpma_cq_wait() succeeds,
 * then all available completions should be collected using rpma_cq_get_wc(3) before the next
 * rpma_cq_wait() call.
 *
 * RETURN VALUE
 * The rpma_cq_wait() function returns 0 on success or a negative error code on failure.
 *
 * ERRORS
 * rpma_cq_wait() can fail with the following errors:
 *
 * - RPMA_E_INVAL - cq is NULL
 * - RPMA_E_PROVIDER - ibv_req_notify_cq(3) failed with a provider error
 * - RPMA_E_NO_COMPLETION - no completions available
 * - RPMA_E_SHARED_CHANNEL - the completion event channel is shared and cannot be handled by any
 *   particular CQ
 *
 * SEE ALSO
 * rpma_conn_get_cq(3), rpma_conn_get_rcq(3), rpma_cq_get_wc(3), rpma_cq_get_fd(3), librpma(7) and
 * https://pmem.io/rpma/
 */
int rpma_cq_wait(struct rpma_cq *cq);

/** 3
 * rpma_cq_get_wc - receive one or more completions
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	struct rpma_cq;
 *	struct ibv_wc;
 *
 *	int rpma_cq_get_wc(struct rpma_cq *cq, int num_entries, struct ibv_wc *wc,
 *			int *num_entries_got);
 *
 * DESCRIPTION
 * rpma_cq_get_wc() polls the CQ for completions and returns the first num_entries (or all
 * available completions if the CQ contains fewer than this number) in the wc array exactly like
 * ibv_poll_cq(3) does. The argument wc is a pointer to an array of ibv_wc structs, as defined in
 * <infiniband/verbs.h>. The number of got completions is returned in the num_entries_got argument
 * if it is not NULL. It can be NULL only if num_entries equals 1. All operations generate
 * completions on error. The operations posted with the RPMA_F_COMPLETION_ALWAYS flag also generate
 * completions on success.
 *
 * Note that if the provided cq is the main CQ and the receive CQ is present on the same connection
 * this function won't return IBV_WC_RECV and IBV_WC_RECV_RDMA_WITH_IMM at any time. The receive CQ
 * has to be used instead to collect these completions. Please see the rpma_conn_get_rcq(3) for
 * details about the receive CQ.
 *
 * RETURN VALUE
 * The rpma_cq_get_wc() function returns 0 on success or a negative error code on failure. On
 * success, it saves all got completions and their number into the wc and num_entries_got
 * respectively. If the status of a completion is not equal to IBV_WC_SUCCESS then only
 * the following attributes are valid: wr_id, status, qp_num, and vendor_err.
 *
 * ERRORS
 * rpma_cq_get_wc() can fail with the following errors:
 *
 * - RPMA_E_INVAL - num_entries < 1, cq or wc is NULL, num_entries > 1 and num_entries_got is NULL
 * - RPMA_E_NO_COMPLETION - no completions available
 * - RPMA_E_PROVIDER - ibv_poll_cq(3) failed with a provider error
 * - RPMA_E_UNKNOWN - ibv_poll_cq(3) failed but no provider error is available
 *
 * SEE ALSO
 * rpma_conn_get_cq(3), rpma_conn_get_rcq(3), rpma_conn_req_recv(3), rpma_cq_wait(3),
 * rpma_cq_get_fd(3), rpma_flush(3), rpma_read(3), rpma_recv(3), rpma_send(3),
 * rpma_send_with_imm(3), rpma_write(3), rpma_atomic_write(3), rpma_write_with_imm(3), librpma(7)
 * and https://pmem.io/rpma/
 */
int rpma_cq_get_wc(struct rpma_cq *cq, int num_entries, struct ibv_wc *wc, int *num_entries_got);

/* error handling */

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
 * rpma_err_2str() returns the const string representation of RPMA error codes.
 *
 * ERRORS
 * rpma_err_2str() can not fail.
 *
 * SEE ALSO
 * librpma(7) and https://pmem.io/rpma/
 */
const char *rpma_err_2str(int ret);

/* librpma logging mechanism control */

/*
 * Available log levels in librpma. Log levels (except RPMA_LOG_DISABLED) are used in logging API
 * calls to indicate logging message severity. Log levels are also used to define thresholds for
 * logging.
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
	 * non-massive info mainly related to public API function completions e.g. connection
	 * established
	 */
	RPMA_LOG_LEVEL_NOTICE,
	/* massive info e.g. every write operation indication */
	RPMA_LOG_LEVEL_INFO,
	/* debug info e.g. write operation dump */
	RPMA_LOG_LEVEL_DEBUG,
};

enum rpma_log_threshold {
	/*
	 * the main threshold level - the logging messages above this level won't trigger
	 * the logging functions
	 */
	RPMA_LOG_THRESHOLD,
	/*
	 * the auxiliary threshold level - may or may not be used by the logging function
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
 *	int rpma_log_set_threshold(enum rpma_log_threshold threshold, enum rpma_log_level level);
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
 * - RPMA_LOG_THRESHOLD - the main threshold used to filter out undesired logging messages.
 *   Messages on a higher level than the primary threshold level are ignored. RPMA_LOG_DISABLED
 *   shall be used to suppress logging. The default value is RPMA_LOG_WARNING.
 * - RPMA_LOG_THRESHOLD_AUX - the auxiliary threshold intended for use inside the logging function
 *   (please see rpma_log_get_threshold(3)). The logging function may or may not take this
 *   threshold into consideration. The default value is RPMA_LOG_DISABLED.
 *
 * Available threshold levels are defined by enum rpma_log_level:
 * - RPMA_LOG_DISABLED - all messages will be suppressed
 * - RPMA_LOG_LEVEL_FATAL - an error that causes the library to stop working immediately
 * - RPMA_LOG_LEVEL_ERROR - an error that causes the library to stop working properly
 * - RPMA_LOG_LEVEL_WARNING - an error that could be handled in the upper level
 * - RPMA_LOG_LEVEL_NOTICE - non-massive info mainly related to public API function completions
 *   e.g. connection established
 * - RPMA_LOG_LEVEL_INFO - massive info e.g. every write operation indication
 * - RPMA_LOG_LEVEL_DEBUG - debug info e.g. write operation dump
 *
 * THE DEFAULT LOGGING FUNCTION
 * The default logging function writes messages to syslog(3) and to stderr(3), where syslog(3) is
 * the primary destination (RPMA_LOG_THRESHOLD applies) whereas stderr(3) is the secondary
 * destination (RPMA_LOG_THRESHOLD_AUX applies).
 *
 * RETURN VALUE
 * rpma_log_syslog_set_threshold() function returns 0 on success or a negative error code on
 * failure.
 *
 * ERRORS
 * rpma_log_set_threshold() can fail with the following errors:
 * - RPMA_E_INVAL - threshold is not RPMA_LOG_THRESHOLD nor RPMA_LOG_THRESHOLD_AUX
 * - RPMA_E_INVAL - level is not a value defined by enum rpma_log_level type
 * - RPMA_E_AGAIN - a temporary error occurred, the retry may fix the problem
 *
 * SEE ALSO
 * rpma_log_get_threshold(3), rpma_log_set_function(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_log_set_threshold(enum rpma_log_threshold threshold, enum rpma_log_level level);

/** 3
 * rpma_log_get_threshold - get the logging threshold level
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	int rpma_log_get_threshold(enum rpma_log_threshold threshold, enum rpma_log_level *level);
 *
 * DESCRIPTION
 * rpma_log_get_threshold() gets the current level of the threshold.
 * See rpma_log_set_threshold(3) for available thresholds and levels.
 *
 * RETURN VALUE
 * rpma_log_get_threshold() function returns 0 on success or a negative error code on failure.
 *
 * ERRORS
 * rpma_log_get_threshold() can fail with the following errors:
 * - RPMA_E_INVAL - threshold is not RPMA_LOG_THRESHOLD nor RPMA_LOG_THRESHOLD_AUX
 * - RPMA_E_INVAL - *level is NULL
 *
 * SEE ALSO
 * rpma_log_set_function(3), rpma_log_set_threshold(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_log_get_threshold(enum rpma_log_threshold threshold, enum rpma_log_level *level);

/*
 * the type used for defining logging functions
 */
typedef void rpma_log_function(
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
 * rpma_log_set_function - set the logging function
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	typedef void rpma_log_function(
 *		enum rpma_log_level level,
 *		const char *file_name,
 *		const int line_no,
 *		const char *function_name,
 *		const char *message_format,
 *		...);
 *
 *	int rpma_log_set_function(rpma_log_function *log_function);
 *
 * DESCRIPTION
 * rpma_log_set_function() allows choosing the function which will get all the generated logging
 * messages. The log_function can be either RPMA_LOG_USE_DEFAULT_FUNCTION which will use
 * the default logging function (built into the library) or a pointer to a user-defined function.
 *
 * Parameters of a user-defined log function are as follow:
 * - level - the log level of the message
 * - file_name - name of the source file where the message coming from. It could be set to NULL and
 *   in such case neither line_no nor function_name are provided.
 * - line_no - the source file line where the message coming from
 * - function_name - the function name where the message coming from
 * - message_format - printf(3)-like format string of the message
 * - "..." - additional arguments of the message format string
 *
 * THE DEFAULT LOGGING FUNCTION
 * The initial value of the logging function is RPMA_LOG_USE_DEFAULT_FUNCTION. This function writes
 * messages to syslog(3) (the primary destination) and to stderr(3) (the secondary destination).
 *
 * RETURN VALUE
 * rpma_log_set_function() function returns 0 on success or error code on failure.
 *
 * ERRORS
 * - RPMA_E_AGAIN - a temporary error occurred, the retry may fix the problem
 *
 * NOTE
 * The logging messages on the levels above the RPMA_LOG_THRESHOLD level won't trigger the logging
 * function.
 *
 * The user defined function must be thread-safe.
 *
 * SEE ALSO
 * rpma_log_get_threshold(3), rpma_log_set_threshold(3), librpma(7) and https://pmem.io/rpma/
 */
int rpma_log_set_function(rpma_log_function *log_function);

#ifdef __cplusplus
}
#endif

#endif /* LIBRPMA_H */
