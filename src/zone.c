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
 * zone.c -- entry points for librpma zone
 */

#include <arpa/inet.h>
#include <infiniband/verbs.h>
#include <netinet/in.h>
#include <rdma/rdma_cma.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <librpma.h>

#include "alloc.h"
#include "config.h"
#include "connection.h"
#include "ravl.h"
#include "rpma_utils.h"
#include "valgrind_internal.h"
#include "zone.h"

// #define RX_TX_SIZE 256 /* XXX */

static int
info_new(struct rpma_config *cfg, struct rdma_addrinfo **rai)
{
	/* prepare hints */
	struct rdma_addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	if (cfg->flags & RPMA_CONFIG_IS_SERVER)
		hints.ai_flags |= RAI_PASSIVE;
	hints.ai_qp_type = IBV_QPT_RC;
	hints.ai_port_space = RDMA_PS_TCP;

	/* query */
	int ret = rdma_getaddrinfo(cfg->addr, cfg->service, &hints, rai);
	if (ret)
		return RPMA_E_ERRNO;

	return 0;
}

static void
info_delete(struct rdma_addrinfo **rai)
{
	rdma_freeaddrinfo(*rai);
	*rai = NULL;
}

static int
device_get(struct rdma_addrinfo *rai, struct ibv_context **device)
{
	struct rdma_cm_id *temp_id;
	int ret = rdma_create_id(NULL, &temp_id, NULL, RDMA_PS_TCP);
	if (ret)
		return RPMA_E_ERRNO;

	if (rai->ai_flags & RAI_PASSIVE) {
		ret = rdma_bind_addr(temp_id, rai->ai_src_addr);
		if (ret) {
			ret = RPMA_E_ERRNO;
			goto err_bind_addr;
		}
	} else {
		ret = rdma_resolve_addr(temp_id, rai->ai_src_addr,
					rai->ai_dst_addr, RPMA_DEFAULT_TIMEOUT);
		if (ret) {
			ret = RPMA_E_ERRNO;
			goto err_resolve_addr;
		}
	}

	*device = temp_id->verbs;

err_bind_addr:
err_resolve_addr:
	(void)rdma_destroy_id(temp_id);
	return ret;
}

static int
epoll_init(struct rpma_zone *zone)
{
	int ret = 0;

	ret = rpma_utils_fd_set_nonblock(zone->ec->fd);
	if (ret)
		return ret;

	zone->ec_epoll = epoll_create1(EPOLL_CLOEXEC);
	if (zone->ec_epoll < 0) {
		ret = RPMA_E_ERRNO;
		ERR_STR(ret, "epoll_create1");
		return ret;
	}

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.ptr = NULL;

	ret = epoll_ctl(zone->ec_epoll, EPOLL_CTL_ADD, zone->ec->fd, &event);
	if (ret < 0) {
		ret = RPMA_E_ERRNO;
		ERR_STR(ret, "epoll_ctl(EPOLL_CTL_ADD)");
		goto err_add;
	}

	return 0;

err_add:
	close(zone->ec_epoll);
	zone->ec_epoll = RPMA_FD_INVALID;
	return ret;
}

static int
epoll_fini(struct rpma_zone *zone)
{
	int ret = close(zone->ec_epoll);
	if (ret)
		return RPMA_E_ERRNO;

	zone->ec_epoll = RPMA_FD_INVALID;

	return 0;
}

static int
zone_init(struct rpma_config *cfg, struct rpma_zone *zone)
{
	int ret = info_new(cfg, &zone->rai);
	if (ret)
		return ret;

	ret = device_get(zone->rai, &zone->device);
	if (ret)
		goto err_device_get;

	/* protection domain */
	zone->pd = ibv_alloc_pd(zone->device);
	if (!zone->pd) {
		ret = RPMA_E_UNKNOWN; /* XXX */
		goto err_alloc_pd;
	}

	/* event channel */
	zone->ec = rdma_create_event_channel();
	if (!zone->ec) {
		ret = RPMA_E_ERRNO;
		goto err_create_event_channel;
	}

	ret = epoll_init(zone);
	if (ret)
		goto err_epoll_init;

	return 0;

err_epoll_init:
	(void)rdma_destroy_event_channel(zone->ec);
	zone->ec = NULL;
err_create_event_channel:
	(void)ibv_dealloc_pd(zone->pd);
err_alloc_pd:
err_device_get:
	info_delete(&zone->rai);
	return ret;
}

static void
zone_fini(struct rpma_zone *zone)
{
	if (zone->ec_epoll != RPMA_FD_INVALID)
		epoll_fini(zone);
	if (zone->listen_id)
		rdma_destroy_id(zone->listen_id);
	if (zone->ec)
		rdma_destroy_event_channel(zone->ec);
	if (zone->pd)
		ibv_dealloc_pd(zone->pd);
}

struct id_conn_pair {
	struct rdma_cm_id *id;
	struct rpma_connection *conn;
};

static int
id_conn_pair_compare(const void *lhs, const void *rhs)
{
	const struct id_conn_pair *l = lhs;
	const struct id_conn_pair *r = rhs;

	intptr_t diff = (intptr_t)l->id - (intptr_t)r->id;
	if (diff != 0)
		return diff > 0 ? 1 : -1;

	return 0;
}

int
rpma_zone_new(struct rpma_config *cfg, struct rpma_zone **zone)
{
	struct rpma_zone *ptr = Malloc(sizeof(struct rpma_zone));
	if (!ptr)
		return RPMA_E_ERRNO;

	ptr->ec = NULL;
	ptr->ec_epoll = RPMA_FD_INVALID;
	ptr->device = NULL;
	ptr->pd = NULL;
	ptr->listen_id = NULL;
	ptr->uarg = NULL;
	ptr->active_connections = 0;
	ptr->connections = ravl_new(id_conn_pair_compare);

	ptr->waiting = 0;

	ptr->on_connection_event_func = NULL;
	ptr->on_timeout_func = NULL;
	ptr->timeout = RPMA_DEFAULT_TIMEOUT;

	ptr->msg_size = cfg->msg_size;
	ptr->send_queue_length = cfg->send_queue_length;
	ptr->recv_queue_length = cfg->recv_queue_length;
	ptr->flags = cfg->flags;

	int ret = zone_init(cfg, ptr);

	if (ret)
		goto err_free;

	*zone = ptr;

	return ret;

err_free:
	Free(ptr);
	return ret;
}

static void
listen_dump(struct rpma_zone *zone)
{
	struct sockaddr_in *addr_in;
	const char *addr;
	unsigned short port;

	if (zone->rai->ai_family == AF_INET) {
		addr_in = (struct sockaddr_in *)zone->rai->ai_src_addr;

		if (!addr_in->sin_port) {
			ERR("addr_in->sin_por == 0");
			return;
		}

		addr = inet_ntoa(addr_in->sin_addr);
		port = htons(addr_in->sin_port);

		fprintf(stderr, "Started listening on %s:%u\n", addr, port);
	} else {
		ASSERT(0);
	}
}

static int
zone_listen(struct rpma_zone *zone)
{
	ASSERT(zone->flags & RPMA_CONFIG_IS_SERVER);

	int ret = rdma_create_id(zone->ec, &zone->listen_id, NULL, RDMA_PS_TCP);
	if (ret)
		return RPMA_E_ERRNO;

	ret = rdma_bind_addr(zone->listen_id, zone->rai->ai_src_addr);
	if (ret) {
		ret = RPMA_E_ERRNO;
		goto err_bind_addr;
	}

	ret = rdma_listen(zone->listen_id, 0 /* backlog */);
	if (ret) {
		ret = RPMA_E_ERRNO;
		goto err_listen;
	}

	listen_dump(zone);

	return 0;

err_listen:
err_bind_addr:
	rdma_destroy_id(zone->listen_id);
	zone->listen_id = NULL;
	return ret;
}

int
rpma_zone_delete(struct rpma_zone **zone)
{
	struct rpma_zone *ptr = *zone;
	if (!ptr)
		return 0;

	zone_fini(ptr);

	Free(ptr);
	*zone = NULL;

	return 0;
}

int
rpma_zone_register_on_connection_event(struct rpma_zone *zone,
				       rpma_on_connection_event_func func)
{
	zone->on_connection_event_func = func;
	return 0;
}

int
rpma_zone_register_on_timeout(struct rpma_zone *zone, rpma_on_timeout_func func,
			      int timeout)
{
	if (timeout < 0)
		return RPMA_E_NEGATIVE_TIMEOUT;

	zone->on_timeout_func = func;
	zone->timeout = timeout;
	return 0;
}

int
rpma_zone_unregister_on_timeout(struct rpma_zone *zone)
{
	zone->on_timeout_func = NULL;
	zone->timeout = RPMA_DEFAULT_TIMEOUT;
	return 0;
}

#define EC_TIMEOUT 1
#define EC_ERR 2

#define MAX_EVENTS 2

static int
event_read(struct rpma_zone *zone, enum rdma_cm_event_type *event, int timeout)
{
	struct epoll_event events[MAX_EVENTS];
	int ret;

	/* if epoll indicates an event is ready it has to be ready */
	int event_is_ready = 0;

	while (1) {
		ret = rdma_get_cm_event(zone->ec, &zone->edata);

		/* a valid event obtained */
		if (ret == 0) {
			*event = zone->edata->event;
			break;
		}

		ASSERTeq(event_is_ready, 0);

		ret = RPMA_E_ERRNO;
		/* an unexpected error occurred */
		if (ret != -EAGAIN)
			return ret;

		/* wait for incoming events */
		ret = epoll_wait(zone->ec_epoll, events, MAX_EVENTS, timeout);
		if (ret == 0)
			return EC_TIMEOUT;
		else if (ret < 0)
			return RPMA_E_ERRNO;

		event_is_ready = 1;
	}

	return 0;
}

int
rpma_zone_event_ack(struct rpma_zone *zone)
{
	ASSERTne(zone->edata, NULL);

	int ret = rdma_ack_cm_event(zone->edata);
	if (ret) {
		ret = RPMA_E_ERRNO;
		ERR_STR(ret, "rdma_ack_cm_event");
		return ret;
	}

	zone->edata = NULL;
	return 0;
}

static int
zone_on_timeout(struct rpma_zone *zone, void *uarg)
{
	rpma_on_timeout_func func;
	util_atomic_load_explicit64(&zone->on_timeout_func, &func,
				    memory_order_acquire);

	if (!func)
		return 0;

	return func(zone, uarg);
}

static void
conn_store(struct ravl *store, struct rpma_connection *conn)
{
	struct id_conn_pair *pair = Malloc(sizeof(*pair));
	pair->id = conn->id;
	pair->conn = conn;

	ravl_insert(store, pair);
}

static struct rpma_connection *
conn_restore(struct ravl *store, struct rdma_cm_id *id)
{
	struct id_conn_pair to_find;
	to_find.id = id;
	to_find.conn = NULL;

	struct ravl_node *node =
		ravl_find(store, &to_find, RAVL_PREDICATE_EQUAL);
	if (!node)
		return NULL;

	struct id_conn_pair *found = ravl_data(node);
	if (!found)
		return NULL;

	struct rpma_connection *ret = found->conn;
	Free(found);
	ravl_remove(store, node);

	return ret;
}

int
rpma_zone_wait_connections(struct rpma_zone *zone, void *uarg)
{
	zone->uarg = uarg;

	struct rpma_connection *conn;

	enum rdma_cm_event_type event = RPMA_CM_EVENT_TYPE_INVALID;
	int ret;

	uint64_t *waiting = &zone->waiting;
	rpma_utils_wait_start(waiting);

	if (zone->flags & RPMA_CONFIG_IS_SERVER) {
		if (!zone->listen_id)
			zone_listen(zone);
	} else {
		ret = zone->on_connection_event_func(
			zone, RPMA_CONNECTION_EVENT_OUTGOING, NULL, uarg);
		if (ret)
			return ret;
	}

	while (rpma_utils_is_waiting(waiting)) {
		ret = event_read(zone, &event, zone->timeout);
		if (ret == EC_TIMEOUT) {
			if (zone_on_timeout(zone, uarg))
				break;
			continue;
		} else if (ret == EC_ERR) {
			ret = RPMA_E_EC_READ;
			break;
		}

		switch (event) {
			case RDMA_CM_EVENT_CONNECT_REQUEST:
				ret = zone->on_connection_event_func(
					zone, RPMA_CONNECTION_EVENT_INCOMING,
					NULL, uarg);
				if (ret)
					return ret;
				++zone->active_connections;
				break;
			case RDMA_CM_EVENT_DISCONNECTED:
				conn = conn_restore(zone->connections,
						    zone->edata->id);
				ret = zone->on_connection_event_func(
					zone, RPMA_CONNECTION_EVENT_DISCONNECT,
					conn, uarg);
				if (ret)
					return ret;
				--zone->active_connections;
				break;
			default:
				ERR("unexpected event received (%u)", event);
				ret = RPMA_E_EC_EVENT;
				break;
		}
	}

	return 0;
}

int
rpma_zone_wait_connected(struct rpma_zone *zone, struct rpma_connection *conn)
{
	enum rdma_cm_event_type event = RPMA_CM_EVENT_TYPE_INVALID;
	int ret = 0;

	while (rpma_utils_is_waiting(&zone->waiting)) {
		ret = event_read(zone, &event, zone->timeout);
		if (ret == EC_TIMEOUT) {
			if (zone_on_timeout(zone, zone->uarg))
				break;
			continue;
		} else if (ret == EC_ERR) {
			ret = RPMA_E_EC_READ;
			break;
		}

		if (event == RDMA_CM_EVENT_ESTABLISHED) {
			if (zone->edata->id != conn->id) {
				ERR("unexpected id received (%p)",
				    zone->edata->id);
				ret = RPMA_E_EC_EVENT_DATA;
			}
			conn_store(zone->connections, conn);
			ret = rpma_zone_event_ack(zone);
			break;
		} else {
			ERR("unexpected event received (%u)", event);
			ret = RPMA_E_EC_EVENT;
			break;
		}
	}

	return ret;
}

int
rpma_zone_wait_break(struct rpma_zone *zone)
{
	rpma_utils_wait_break(&zone->waiting);
	return 0;
}
