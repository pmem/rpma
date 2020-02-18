/*
 * Copyright 2019-2020, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * base.h -- base definitions of librpma entry points (EXPERIMENTAL)
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

#define RPMA_E_UNKNOWN (-100000)
#define RPMA_E_NOSUPP (-100001)
#define RPMA_E_NEGATIVE_TIMEOUT (-100002)
#define RPMA_E_NOT_LISTENING (-100003)
#define RPMA_E_EC_READ (-100004)
#define RPMA_E_EC_EVENT (-100005)
#define RPMA_E_EC_EVENT_DATA (-100006)
#define RPMA_E_UNHANDLED_EVENT (-100007)
#define RPMA_E_UNKNOWN_CONNECTION (-100008)

/* config setup */

struct rpma_config;

int rpma_config_new(struct rpma_config **cfg);

int rpma_config_set_addr(struct rpma_config *cfg, const char *addr);

int rpma_config_set_service(struct rpma_config *cfg, const char *service);

int rpma_config_set_msg_size(struct rpma_config *cfg, size_t msg_size);

int rpma_config_set_send_queue_length(struct rpma_config *cfg,
				      uint64_t queue_len);

int rpma_config_set_recv_queue_length(struct rpma_config *cfg,
				      uint64_t queue_len);

typedef void *(*rpma_malloc_func)(size_t size);

typedef void (*rpma_free_func)(void *ptr);

int rpma_config_set_queue_alloc_funcs(struct rpma_config *cfg,
				      rpma_malloc_func malloc_func,
				      rpma_free_func free_func);

#define RPMA_CONFIG_IS_SERVER (1 << 0)

int rpma_config_set_flags(struct rpma_config *cfg, unsigned flags);

int rpma_config_delete(struct rpma_config **cfg);

/* zone */

struct rpma_zone;

int rpma_zone_new(struct rpma_config *cfg, struct rpma_zone **zone);

int rpma_zone_delete(struct rpma_zone **zone);

/* dispatcher */

struct rpma_dispatcher;

int rpma_dispatcher_new(struct rpma_zone *zone, struct rpma_dispatcher **disp);

int rpma_dispatch(struct rpma_dispatcher *disp);

int rpma_dispatcher_delete(struct rpma_dispatcher **disp);

/* zone connection loop setup */

#define RPMA_CONNECTION_EVENT_INCOMING 0
#define RPMA_CONNECTION_EVENT_OUTGOING 1
#define RPMA_CONNECTION_EVENT_DISCONNECT 2

struct rpma_connection;

typedef int (*rpma_on_connection_event_func)(struct rpma_zone *zone,
					     uint64_t event,
					     struct rpma_connection *conn,
					     void *uarg);

int rpma_zone_register_on_connection_event(struct rpma_zone *zone,
					   rpma_on_connection_event_func func);

typedef int (*rpma_on_timeout_func)(struct rpma_zone *zone, void *uarg);

int rpma_zone_register_on_timeout(struct rpma_zone *zone,
				  rpma_on_timeout_func func, int timeout);

int rpma_zone_unregister_on_timeout(struct rpma_zone *zone);

int rpma_zone_wait_connections(struct rpma_zone *zone, void *uarg);

int rpma_zone_wait_break(struct rpma_zone *zone);

/* connection */

int rpma_connection_new(struct rpma_zone *zone, struct rpma_connection **conn);

int rpma_connection_accept(struct rpma_connection *conn);

int rpma_connection_reject(struct rpma_zone *zone);

int rpma_connection_establish(struct rpma_connection *conn);

int rpma_connection_disconnect(struct rpma_connection *conn);

int rpma_connection_delete(struct rpma_connection **conn);

int rpma_connection_set_custom_data(struct rpma_connection *conn, void *data);

int rpma_connection_get_custom_data(struct rpma_connection *conn, void **data);

int rpma_connection_get_zone(struct rpma_connection *conn,
			     struct rpma_zone **zone);

int rpma_connection_attach(struct rpma_connection *conn,
			   struct rpma_dispatcher *disp);

int rpma_connection_detach(struct rpma_connection *conn);

int rpma_connection_dispatch_break(struct rpma_connection *conn);

typedef int (*rpma_queue_func)(struct rpma_connection *conn, void *arg);

int rpma_connection_enqueue(struct rpma_connection *conn, rpma_queue_func func,
			    void *arg);

typedef int (*rpma_on_transmission_notify_func)(struct rpma_connection *conn,
						void *addr, size_t len,
						void *uarg);

int rpma_connection_register_on_notify(struct rpma_connection *conn,
				       rpma_on_transmission_notify_func func);

typedef int (*rpma_on_connection_recv_func)(struct rpma_connection *conn,
					    void *ptr, size_t length);

int rpma_connection_register_on_recv(struct rpma_connection *conn,
				     rpma_on_connection_recv_func func);

/* connection group */

struct rpma_connection_group;

int rpma_connection_group_new(struct rpma_connection_group **group);

int rpma_connection_group_add(struct rpma_connection_group *group,
			      struct rpma_connection *conn);

int rpma_connection_group_remove(struct rpma_connection_group *group,
				 struct rpma_connection *conn);

int rpma_connection_group_enqueue(struct rpma_connection_group *group,
				  rpma_queue_func func, void *arg);

int rpma_connection_group_delete(struct rpma_connection_group **group);

/* error handling */

const char *rpma_errormsg(void);

#ifdef __cplusplus
}
#endif
#endif /* base.h */
