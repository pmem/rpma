/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2019-2020, Intel Corporation
 */

/*
 * librpma/base.h -- base definitions of librpma entry points (EXPERIMENTAL)
 *
 * This library provides low-level support for remote access to persistent
 * memory utilizing RDMA-capable RNICs.
 *
 * See librpma(7) for details.
 */

#ifndef LIBRPMA_BASE_H
#define LIBRPMA_BASE_H 1

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* XXX rework in pmem2-style */
#define RPMA_E_UNKNOWN				(-100000)
#define RPMA_E_NOSUPP				(-100001)
#define RPMA_E_NEGATIVE_TIMEOUT		(-100002)
#define RPMA_E_NOT_LISTENING		(-100003)
#define RPMA_E_EC_READ				(-100004)
#define RPMA_E_EC_EVENT				(-100005)
#define RPMA_E_EC_EVENT_DATA		(-100006)
#define RPMA_E_UNHANDLED_EVENT		(-100007)
#define RPMA_E_UNKNOWN_CONNECTION	(-100008)
#define RPMA_E_TIMEOUT				(-100009)


/* picking up an RDMA-capable device */
struct rpma_device;

int rpma_device_by_src_address(const char *addr, struct rpma_device **rdev);

int rpma_device_by_dst_address(const char *addr, struct rpma_device **rdev);

int rpma_device_delete(struct rpma_device **rdev);


/* an RPMA peer config */
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


/* connection config setup */
struct rpma_conn_cfg;

int rpma_conn_cfg_new(struct rpma_conn_cfg **cfg);

int rpma_conn_cfg_set_max_msg_size(struct rpma_conn_cfg *cfg, size_t msg_size);

int rpma_conn_cfg_set_send_buffers_num(struct rpma_conn_cfg *cfg,
		uint64_t buff_num);

int rpma_conn_cfg_set_recv_buffers_num(struct rpma_conn_cfg *cfg,
		uint64_t buff_num);

int rpma_conn_cfg_set_setup_timeout(struct rpma_conn_cfg *cfg, int timeout);

int rpma_conn_cfg_set_op_timeout(struct rpma_conn_cfg *cfg, int timeout);

int rpma_conn_cfg_delete(struct rpma_conn_cfg **cfg);


/* establishing a connection */
struct rpma_conn;

int rpma_connect(struct rpma_peer *peer, struct rpma_conn_cfg *ccfg,
		const char *addr, const char *service, struct rpma_conn **conn);

int rpma_accept(struct rpma_peer *peer, struct rpma_conn_cfg *ccfg,
		const char *addr, const char *service, struct rpma_conn **conn);

int rpma_conn_set_app_context(struct rpma_conn *conn, void *data);

int rpma_conn_get_app_context(struct rpma_conn *conn, void **data);

#define RPMA_DISCONNECT_NOW			(0)
#define RPMA_DISCONNECT_WHEN_DONE	(1 << 0)

int rpma_disconnect(struct rpma_conn **conn, int flags);


/* completion handling */
#define RPMA_OP_READ			(1 << 0)
#define RPMA_OP_WRITE			(1 << 1)
#define RPMA_OP_COMMIT			(1 << 2)
#define RPMA_OP_RECV			(1 << 3)

int rpma_complete(struct rpma_conn *conn, int op_flags,
		void **op_context);

/* error handling */
const char *rpma_errormsg(void);

#ifdef __cplusplus
}
#endif
#endif /* LIBRPMA_BASE_H */
