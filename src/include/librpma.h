/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2019-2020, Intel Corporation
 */

/*
 * librpma.h -- definitions of librpma entry points (EXPERIMENTAL)
 *
 * This library provides low-level support for remote access to persistent
 * memory utilizing RDMA-capable RNICs.
 *
 * See librpma(7) for details.
 */

#ifndef LIBRPMA_H
#define LIBRPMA_H 1

#include <stddef.h>
#include <stdint.h>


/* XXX rework in pmem2-style */
#define RPMA_E_UNKNOWN			(-100000)
#define RPMA_E_NOSUPP			(-100001)
#define RPMA_E_NEGATIVE_TIMEOUT		(-100002)
#define RPMA_E_NOT_LISTENING		(-100003)
#define RPMA_E_EC_READ			(-100004)
#define RPMA_E_EC_EVENT			(-100005)
#define RPMA_E_EC_EVENT_DATA		(-100006)
#define RPMA_E_UNHANDLED_EVENT		(-100007)
#define RPMA_E_UNKNOWN_CONNECTION	(-100008)
#define RPMA_E_TIMEOUT			(-100009)


/* picking up an RDMA-capable device */
struct rpma_device;

int rpma_device_by_src_address(const char *addr, struct rpma_device **rdev);

int rpma_device_by_dst_address(const char *addr, struct rpma_device **rdev);

int rpma_device_delete(struct rpma_device **rdev);


/* peer config */
struct rpma_peer_cfg;

int rpma_peer_cfg_new(struct rpma_peer_cfg **zcfg);

typedef void *(*rpma_malloc_func)(size_t size);

typedef void (*rpma_free_func)(void *ptr);

int rpma_peer_cfg_set_msg_buffer_alloc_funcs(struct rpma_peer_cfg *zcfg,
		rpma_malloc_func malloc_func, rpma_free_func free_func);

int rpma_peer_cfg_delete(struct rpma_peer_cfg **zcfg);


/* peer */
struct rpma_peer;

int rpma_peer_new(struct rpma_peer_cfg *zcfg, struct rpma_device *rdev,
		struct rpma_peer **peer);

int rpma_peer_delete(struct rpma_peer **peer);


/* connection config */
struct rpma_conn_cfg;

int rpma_conn_cfg_new(struct rpma_conn_cfg **cfg);

int rpma_conn_cfg_set_setup_timeout(struct rpma_conn_cfg *cfg, int timeout);

int rpma_conn_cfg_set_op_timeout(struct rpma_conn_cfg *cfg, int timeout);

int rpma_conn_cfg_delete(struct rpma_conn_cfg **cfg);


/* connection */
struct rpma_conn;

int rpma_connect(struct rpma_peer *peer, struct rpma_conn_cfg *ccfg,
		const char *addr, const char *service, struct rpma_conn **conn);

int rpma_accept(struct rpma_peer *peer, struct rpma_conn_cfg *ccfg,
		const char *addr, const char *service, struct rpma_conn **conn);

int rpma_conn_set_app_context(struct rpma_conn *conn, void *data);

int rpma_conn_get_app_context(struct rpma_conn *conn, void **data);

#define RPMA_DISCONNECT_NOW		(0)
#define RPMA_DISCONNECT_WHEN_DONE	(1 << 0)

int rpma_disconnect(struct rpma_conn **conn, int flags);


/* completion handling */
#define RPMA_OP_READ	(1 << 0)
#define RPMA_OP_WRITE	(1 << 1)
#define RPMA_OP_COMMIT	(1 << 2)
#define RPMA_OP_RECV	(1 << 3)

int rpma_complete(struct rpma_conn *conn, int op_flags,
		void **op_context);


/* local memory region */
struct rpma_memory;

#define RPMA_MR_READ_SRC	(1 << 0)
#define RPMA_MR_READ_DST	(1 << 1)
#define RPMA_MR_WRITE_SRC	(1 << 2)
#define RPMA_MR_WRITE_DST	(1 << 3)

int rpma_memory_new(struct rpma_peer *peer, void *ptr, size_t size,
		int usage, struct rpma_memory **mem);

int rpma_memory_get_ptr(struct rpma_memory *mem, void **ptr);

int rpma_memory_get_size(struct rpma_memory *mem, size_t *size);

struct rpma_memory_id {
	uint64_t data[4];
};

int rpma_memory_get_id(struct rpma_memory *mem,
		struct rpma_memory_id *id);

int rpma_memory_delete(struct rpma_memory **mem);


/* remote memory region */
struct rpma_memory_remote;

int rpma_memory_remote_new(struct rpma_peer *peer, struct rpma_memory_id *id,
		struct rpma_memory_remote **rmem);

int rpma_memory_remote_get_size(struct rpma_memory_remote *rmem, size_t *size);

int rpma_memory_remote_delete(struct rpma_memory_remote **rmem);


/* remote memory access commands */
#define RPMA_NO_COMPLETION	(0)
#define RPMA_WITH_COMPLETION	(1 << 0)

int rpma_read(struct rpma_conn *conn,
		struct rpma_memory *dst, size_t dst_off,
		struct rpma_memory_remote *src, size_t src_off, size_t length,
		void *op_context, int op_flags);

int rpma_write(struct rpma_conn *conn,
		struct rpma_memory_remote *dst, size_t dst_off,
		struct rpma_memory *src, size_t src_off, size_t length,
		void *op_context, int op_flags);

int rpma_commit(struct rpma_conn *conn,
		void *op_context, int op_flags);


/** 3
 * rpma_errormsg - returns the last error message
 *
 * SYNOPSIS
 *
 *	#include <librpma.h>
 *
 *	const char *rpma_errormsg(void);
 *
 * DESCRIPTION
 * If an error is detected during the call to a librpma(7) function, the
 * application may retrieve an error message describing the reason of the
 * failure from rpma_errormsg(). The error message buffer is thread-local;
 * errors encountered in one thread do not affect its value in
 * other threads. The buffer is never cleared by any library function; its
 * content is significant only when the return value of the immediately
 * preceding call to a librpma(7) function indicated an error.
 * The application must not modify or free the error message string.
 * Subsequent calls to other library functions may modify the previous message.
 *
 * RETURN VALUE
 * The rpma_errormsg() function returns a pointer to a static buffer
 * containing the last error message logged for the current thread. If
 * .I errno
 * was set, the error message may include a description of the
 * corresponding error code as returned by strerror(3).
 *
 * SEE ALSO
 * strerror(3), librpma(7) and
 * .B <https://pmem.io>
 */
const char *rpma_errormsg(void);

#endif /* LIBRPMA_H */
