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
 * connection.c -- entry points for librpma connection
 */

#include <rdma/rdma_cma.h>

#include <librpma.h>

#include "alloc.h"
#include "connection.h"
#include "dispatcher.h"
#include "memory.h"
#include "rpma_utils.h"
#include "zone.h"

#define CQ_SIZE 10 /* XXX */

int
rpma_connection_new(struct rpma_zone *zone, struct rpma_connection **conn)
{
	struct rpma_connection *ptr = Malloc(sizeof(struct rpma_connection));
	if (!ptr)
		return RPMA_E_ERRNO;

	ptr->zone = zone;
	ptr->id = NULL;
	ptr->cq = NULL;
	ptr->disconnected = 0;
	ptr->disp = NULL;

	ptr->on_connection_recv_func = NULL;
	ptr->on_transmission_notify_func = NULL;

	ptr->custom_data = NULL;

	int ret = rpma_connection_rma_init(ptr);
	if (ret)
		goto err_rma_init;

	ret = rpma_connection_msg_init(ptr);
	if (ret)
		goto err_msg_init;

	*conn = ptr;

	return 0;

err_msg_init:
	(void)rpma_connection_rma_fini(ptr);
err_rma_init:
	Free(ptr);
	return ret;
}

static int
id_init(struct rpma_connection *conn, struct rdma_cm_id *id)
{
	struct rpma_zone *zone = conn->zone;
	int ret = 0;

	int cqe = CQ_SIZE;
	conn->cq = ibv_create_cq(id->verbs, cqe, (void *)conn, 0, 0);
	if (!conn->cq)
		return RPMA_E_ERRNO;

	struct ibv_qp_init_attr init_qp_attr;

	init_qp_attr.qp_context = conn;
	init_qp_attr.send_cq = conn->cq;
	init_qp_attr.recv_cq = conn->cq;
	init_qp_attr.srq = NULL;
	init_qp_attr.cap.max_send_wr = CQ_SIZE; /* XXX */
	init_qp_attr.cap.max_recv_wr = CQ_SIZE; /* XXX */
	init_qp_attr.cap.max_send_sge = 1;
	init_qp_attr.cap.max_recv_sge = 1;
	init_qp_attr.cap.max_inline_data = 0; /* XXX */
	init_qp_attr.qp_type = IBV_QPT_RC;
	init_qp_attr.sq_sig_all = 0;

	ret = rdma_create_qp(id, zone->pd, &init_qp_attr);
	if (ret) {
		ret = RPMA_E_ERRNO;
		goto err_create_qp;
	}

	conn->id = id;

	return 0;

err_create_qp:
	ibv_destroy_cq(conn->cq);
	conn->cq = NULL;
	return ret;
}

static int
id_fini(struct rpma_connection *conn)
{
	int ret = 0;

	if (!conn->id)
		return 0;

	if (conn->id->qp) {
		int ret = ibv_destroy_qp(conn->id->qp);
		if (ret) {
			ERR_STR(ret, "ibv_destroy_qp");
			return -ret; /* XXX macro? */
		}
	}

	if (conn->cq) {
		ret = ibv_destroy_cq(conn->cq);
		if (ret) {
			ERR_STR(ret, "ibv_destroy_cq");
			return -ret; /* XXX macro? */
		}
	}

	return 0;
}

static int
recv_post_all(struct rpma_connection *conn)
{
	int ret;
	void *ptr = conn->recv.buff->ptr;

	for (uint64_t i = 0; i < conn->zone->recv_queue_length; ++i) {
		ret = rpma_connection_recv_post(conn, ptr);
		if (ret)
			return ret;
		ptr = (void *)((uintptr_t)ptr + conn->zone->msg_size);
	}

	return 0;
}

int
rpma_connection_accept(struct rpma_connection *conn)
{
	int ret = id_init(conn, conn->zone->edata->id);
	if (ret)
		return ret;

	ret = recv_post_all(conn);
	if (ret)
		goto err_recv_post_all;

	struct rdma_conn_param conn_param;
	conn_param.private_data = NULL; /* XXX very interesting */
	conn_param.private_data_len = 0;
	conn_param.responder_resources = CQ_SIZE; /* XXX ? */
	conn_param.initiator_depth = CQ_SIZE;	  /* XXX ? */
	conn_param.flow_control = 1;		  /* XXX */
	conn_param.retry_count = 0;		  /* ignored */
	conn_param.rnr_retry_count = 7;		  /* max for 3-bit value */
	/* since QP is created on this connection id srq and qp_num are ignored
	 */

	ret = rdma_accept(conn->id, &conn_param);
	if (ret) {
		ERR_STR(ret, "rdma_accept");
		goto err_accept;
	}

	ret = rpma_zone_event_ack(conn->zone);
	if (ret)
		goto err_event_ack;

	ret = rpma_zone_wait_connected(conn->zone, conn);
	if (ret)
		goto err_connected;

	return 0;

err_connected:
err_event_ack:
err_accept:
err_recv_post_all:
	id_fini(conn);
	return ret;
}

int
rpma_connection_reject(struct rpma_zone *zone)
{
	/* XXX use private_data? */
	int ret = rdma_reject(zone->edata->id, NULL, 0);
	if (ret) {
		ret = RPMA_E_ERRNO;
		ERR_STR(ret, "rdma_reject");
		return ret;
	}

	ret = rpma_zone_event_ack(zone);
	if (ret)
		return ret;

	return 0;
}

int
rpma_connection_establish(struct rpma_connection *conn)
{
	struct rdma_addrinfo *rai = conn->zone->rai;

	int ret = rdma_create_id(NULL, &conn->id, NULL, RDMA_PS_TCP);
	if (ret)
		return RPMA_E_ERRNO;

	ret = rdma_resolve_addr(conn->id, rai->ai_src_addr, rai->ai_dst_addr,
				RPMA_DEFAULT_TIMEOUT);
	if (ret) {
		ret = RPMA_E_ERRNO;
		ERR_STR(ret, "rdma_resolve_addr");
		goto err_resolve_addr;
	}

	ret = rdma_resolve_route(conn->id, RPMA_DEFAULT_TIMEOUT);
	if (ret) {
		ret = RPMA_E_ERRNO;
		ERR_STR(ret, "rdma_resolve_route");
		goto err_resolve_route;
	}

	ret = id_init(conn, conn->id);
	if (ret)
		goto err_id_init;

	ret = recv_post_all(conn);
	if (ret)
		goto err_recv_post_all;

	struct rdma_conn_param conn_param;
	memset(&conn_param, 0, sizeof conn_param);
	conn_param.responder_resources = RDMA_MAX_RESP_RES;
	conn_param.initiator_depth = RDMA_MAX_INIT_DEPTH;
	conn_param.flow_control = 1;
	conn_param.retry_count = 7;	/* max 3-bit value */
	conn_param.rnr_retry_count = 7; /* max 3-bit value */
	ret = rdma_connect(conn->id, &conn_param);
	if (ret) {
		ret = RPMA_E_ERRNO;
		ERR_STR(ret, "rdma_connect");
		goto err_connect;
	}

	ret = rdma_migrate_id(conn->id, conn->zone->ec);
	if (ret) {
		ret = RPMA_E_ERRNO;
		ERR_STR(ret, "rdma_migrate_id");
		goto err_migrate_id;
	}

	return 0;

err_migrate_id:
	(void)rdma_disconnect(conn->id);
err_connect:
err_recv_post_all:
	id_fini(conn);
err_id_init:
err_resolve_route:
err_resolve_addr:
	rdma_destroy_id(conn->id);
	conn->id = NULL;
	return ret;
}

int
rpma_connection_disconnect(struct rpma_connection *conn)
{
	/* XXX any prior messaging? */
	int ret = rdma_disconnect(conn->id);
	if (ret) {
		ret = RPMA_E_ERRNO;
		ERR_STR(ret, "rdma_disconnect");
		return ret;
	}

	conn->disconnected = 1;

	return 0;
}

int
rpma_connection_delete(struct rpma_connection **conn)
{
	struct rpma_connection *ptr = *conn;
	int ret;

	ASSERTeq(ptr->disp, NULL);

	if (ptr->id && !ptr->disconnected) {
		ret = rpma_connection_disconnect(ptr);
		if (ret)
			return ret;
	}

	id_fini(ptr);

	ret = rpma_connection_rma_fini(ptr);
	if (ret)
		goto err_rma_fini;

	ret = rpma_connection_msg_fini(ptr);
	if (ret)
		goto err_msg_fini;

	Free(ptr);
	*conn = NULL;

	return 0;

err_rma_fini:
	(void)rpma_connection_msg_fini(ptr);
err_msg_fini:
	return ret;
}

int
rpma_connection_set_custom_data(struct rpma_connection *conn, void *data)
{
	conn->custom_data = data;

	return 0;
}

int
rpma_connection_get_custom_data(struct rpma_connection *conn, void **data)
{
	*data = conn->custom_data;

	return 0;
}

int
rpma_connection_get_zone(struct rpma_connection *conn, struct rpma_zone **zone)
{
	*zone = conn->zone;

	return 0;
}

int
rpma_connection_attach(struct rpma_connection *conn,
		       struct rpma_dispatcher *disp)
{
	int ret = rpma_dispatcher_attach_connection(disp, conn);
	if (ret)
		return ret;

	conn->disp = disp;

	return 0;
}

int
rpma_connection_detach(struct rpma_connection *conn)
{
	int ret = rpma_dispatcher_detach_connection(conn->disp, conn);
	if (ret)
		return ret;

	conn->disp = NULL;

	return 0;
}

int
rpma_connection_dispatch_break(struct rpma_connection *conn)
{
	ASSERTne(conn->disp, NULL);

	rpma_dispatch_break(conn->disp);
	return 0;
}

int
rpma_connection_enqueue(struct rpma_connection *conn, rpma_queue_func func,
			void *arg)
{
	ASSERTne(conn->disp, NULL); /* XXX ? */

	return rpma_dispatcher_enqueue_func(conn->disp, conn, func, arg);
}

int
rpma_connection_register_on_notify(struct rpma_connection *conn,
				   rpma_on_transmission_notify_func func)
{
	conn->on_transmission_notify_func = func;

	return 0;
}

int
rpma_connection_register_on_recv(struct rpma_connection *conn,
				 rpma_on_connection_recv_func func)
{
	conn->on_connection_recv_func = func;

	return 0;
}

int
rpma_connection_cq_entry_process(struct rpma_connection *conn,
				 struct ibv_wc *wc)
{
	int ret = 0;
	if (wc->opcode & IBV_WC_RECV) {
		/* XXX uarg is still necesarry here? */
		void *ptr = (void *)wc->wr_id;
		ret = conn->on_connection_recv_func(conn, ptr,
						    conn->zone->msg_size);
	} else {
		ASSERT(0);
	}
	/* XXX IBV_WC_RDMA_WRITE, IBV_WC_RDMA_READ */

	return ret;
}

static int
cq_entry_process_or_enqueue(struct rpma_connection *conn, struct ibv_wc *wc)
{
	if (conn->disp)
		return rpma_dispatcher_enqueue_cq_entry(conn->disp, conn, wc);

	return rpma_connection_cq_entry_process(conn, wc);
}

static inline int
cq_read(struct rpma_connection *conn, struct ibv_wc *wc)
{
	int ret = ibv_poll_cq(conn->cq, 1 /* num_entries */, wc);
	if (ret == 0)
		return 0;
	if (ret < 0) {
		ERR_STR(ret, "ibv_poll_cq");
		return ret;
	}

	ASSERTeq(ret, 1);
	ASSERTeq(wc->status, IBV_WC_SUCCESS); /* XXX */

	return ret;
}

int
rpma_connection_cq_wait(struct rpma_connection *conn, enum ibv_wc_opcode opcode,
			uint64_t wr_id)
{
	struct ibv_wc wc;
	int ret;
	int mismatch;

	/* XXX additional stop condition? */
	while (1) {
		ret = cq_read(conn, &wc);
		if (ret == 0)
			continue;
		else if (ret < 0)
			return ret;

		mismatch = (wc.opcode != opcode);
		mismatch |= (wc.wr_id != wr_id);

		if (mismatch) {
			ret = cq_entry_process_or_enqueue(conn, &wc);
			if (ret) {
				/* XXX */
				ASSERT(0);
			}

			continue;
		}

		break;
	}

	return 0;
}

int
rpma_connection_cq_process(struct rpma_connection *conn)
{
	struct ibv_wc wc;
	int ret;

	while (1) {
		ret = cq_read(conn, &wc);
		if (ret == 0)
			break;
		else if (ret < 0)
			return ret;

		ret = rpma_connection_cq_entry_process(conn, &wc);
		if (ret)
			return ret;
	}

	return 0;
}
