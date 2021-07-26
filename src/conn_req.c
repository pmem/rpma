// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn_req.c -- librpma connection-request-related implementations
 */

#include <stdlib.h>
#include <rdma/rdma_cma.h>

#include "common.h"
#include "conn.h"
#include "conn_cfg.h"
#include "conn_req.h"
#include "info.h"
#include "log_internal.h"
#include "mr.h"
#include "peer.h"
#include "private_data.h"

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

struct rpma_conn_req {
	/* RDMA_CM_EVENT_CONNECT_REQUEST event (if applicable) */
	struct rdma_cm_event *edata;
	/* CM ID of the connection request */
	struct rdma_cm_id *id;
	/* main CQ */
	struct rpma_cq *cq;
	/* receive CQ */
	struct rpma_cq *rcq;

	/* private data of the CM ID (incoming only) */
	struct rpma_conn_private_data data;

	/* a parent RPMA peer of this request - needed for derivative objects */
	struct rpma_peer *peer;
};

/*
 * rpma_conn_req_from_id -- allocate a new conn_req object from CM ID and equip
 * the latter with QP and CQ
 *
 * ASSUMPTIONS
 * - peer != NULL && id != NULL && cfg != NULL && req_ptr != NULL
 */
static int
rpma_conn_req_from_id(struct rpma_peer *peer, struct rdma_cm_id *id,
		const struct rpma_conn_cfg *cfg, struct rpma_conn_req **req_ptr)
{
	int ret = 0;

	int cqe, rcqe;
	/* read the main CQ size from the configuration */
	(void) rpma_conn_cfg_get_cqe(cfg, &cqe);
	/* read the receive CQ size from the configuration */
	(void) rpma_conn_cfg_get_rcqe(cfg, &rcqe);

	struct rpma_cq *cq = NULL;
	ret = rpma_cq_new(id->verbs, cqe, &cq);
	if (ret)
		return ret;

	struct rpma_cq *rcq = NULL;
	if (rcqe) {
		ret = rpma_cq_new(id->verbs, rcqe, &rcq);
		if (ret)
			goto err_rpma_cq_delete;
	}

	/* create a QP */
	ret = rpma_peer_create_qp(peer, id, cq, rcq, cfg);
	if (ret)
		goto err_rpma_rcq_delete;

	*req_ptr = (struct rpma_conn_req *)malloc(sizeof(struct rpma_conn_req));
	if (*req_ptr == NULL) {
		ret = RPMA_E_NOMEM;
		goto err_destroy_qp;
	}

	(*req_ptr)->edata = NULL;
	(*req_ptr)->id = id;
	(*req_ptr)->cq = cq;
	(*req_ptr)->rcq = rcq;
	(*req_ptr)->data.ptr = NULL;
	(*req_ptr)->data.len = 0;
	(*req_ptr)->peer = peer;

	return 0;

err_destroy_qp:
	rdma_destroy_qp(id);

err_rpma_rcq_delete:
	(void) rpma_cq_delete(&rcq);

err_rpma_cq_delete:
	(void) rpma_cq_delete(&cq);

	return ret;
}

/*
 * rpma_conn_req_accept -- call rdma_accept()+rdma_ack_cm_event(). If succeeds
 * request re-packing the connection request to a connection object. Otherwise,
 * rdma_disconnect()+rdma_destroy_qp()+rpma_cq_delete() to destroy
 * the unsuccessful connection request.
 *
 * ASSUMPTIONS
 * - req != NULL && conn_param != NULL && conn_ptr != NULL
 */
static int
rpma_conn_req_accept(struct rpma_conn_req *req,
	struct rdma_conn_param *conn_param, struct rpma_conn **conn_ptr)
{
	int ret = 0;

	if (rdma_accept(req->id, conn_param)) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_accept()");
		ret = RPMA_E_PROVIDER;
		(void) rdma_ack_cm_event(req->edata);
		goto err_conn_req_delete;
	}

	/* ACK the connection request event */
	if (rdma_ack_cm_event(req->edata)) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_ack_cm_event()");
		ret = RPMA_E_PROVIDER;
		goto err_conn_disconnect;
	}

	struct rpma_conn *conn = NULL;
	ret = rpma_conn_new(req->peer, req->id, req->cq, req->rcq, &conn);
	if (ret)
		goto err_conn_disconnect;

	rpma_conn_transfer_private_data(conn, &req->data);

	*conn_ptr = conn;
	return 0;

err_conn_disconnect:
	(void) rdma_disconnect(req->id);

err_conn_req_delete:
	rdma_destroy_qp(req->id);
	(void) rpma_cq_delete(&req->rcq);
	(void) rpma_cq_delete(&req->cq);

	return ret;
}

/*
 * rpma_conn_req_connect_active -- call rdma_connect(). If succeeds request
 * re-packing the connection request to a connection object. Otherwise,
 * rdma_destroy_qp()+rpma_cq_delete()+rdma_destroy_id() to destroy
 * the unsuccessful connection request.
 *
 * ASSUMPTIONS
 * - req != NULL && conn_param != NULL && conn_ptr != NULL
 */
static int
rpma_conn_req_connect_active(struct rpma_conn_req *req,
	struct rdma_conn_param *conn_param, struct rpma_conn **conn_ptr)
{
	int ret = 0;

	struct rpma_conn *conn = NULL;
	ret = rpma_conn_new(req->peer, req->id, req->cq, req->rcq, &conn);
	if (ret) {
		rdma_destroy_qp(req->id);
		(void) rpma_cq_delete(&req->rcq);
		(void) rpma_cq_delete(&req->cq);
		(void) rdma_destroy_id(req->id);
		return ret;
	}

	if (rdma_connect(req->id, conn_param)) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_connect()");
		(void) rpma_conn_delete(&conn);
		return RPMA_E_PROVIDER;
	}

	*conn_ptr = conn;
	return 0;
}

/*
 * rpma_conn_req_reject -- destroy CQ of the CM ID and reject the connection.
 *
 * ASSUMPTIONS
 * - req != NULL
 */
static int
rpma_conn_req_reject(struct rpma_conn_req *req)
{
	int ret = rpma_cq_delete(&req->rcq);

	int ret2 = rpma_cq_delete(&req->cq);
	if (!ret && ret2)
		ret = ret2;

	if (rdma_reject(req->id,
			NULL /* private data */,
			0 /* private data len */)) {
		if (!ret) {
			RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_reject()");
			ret = RPMA_E_PROVIDER;
		}
	}

	if (rdma_ack_cm_event(req->edata)) {
		if (!ret) {
			RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_ack_cm_event()");
			ret = RPMA_E_PROVIDER;
		}
	}

	return ret;
}

/*
 * rpma_conn_req_destroy -- destroy CQ of the CM ID and destroy the CM ID.
 *
 * ASSUMPTIONS
 * - req != NULL
 */
static int
rpma_conn_req_destroy(struct rpma_conn_req *req)
{
	int ret = rpma_cq_delete(&req->rcq);

	int ret2 = rpma_cq_delete(&req->cq);
	if (!ret && ret2)
		ret = ret2;

	if (rdma_destroy_id(req->id)) {
		if (!ret) {
			RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_destroy_id()");
			ret = RPMA_E_PROVIDER;
		}
	}

	return ret;
}

/* internal librpma API */

/*
 * rpma_conn_req_from_cm_event -- feeds an ID from cm event into
 * rpma_conn_req_from_id and add the event to conn_req
 *
 * ASSUMPTIONS
 * cfg != NULL
 */
int
rpma_conn_req_from_cm_event(struct rpma_peer *peer,
		struct rdma_cm_event *edata, const struct rpma_conn_cfg *cfg,
		struct rpma_conn_req **req_ptr)
{
	if (peer == NULL || edata == NULL || req_ptr == NULL)
		return RPMA_E_INVAL;

	if (edata->event != RDMA_CM_EVENT_CONNECT_REQUEST)
		return RPMA_E_INVAL;

	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_id(peer, edata->id, cfg, &req);
	if (ret)
		return ret;

	ret = rpma_private_data_store(edata, &req->data);
	if (ret) {
		(void) rpma_conn_req_delete(&req);
		return ret;
	}
	req->edata = edata;
	*req_ptr = req;

	return 0;
}

/* public librpma API */

/*
 * rpma_conn_req_new -- create a new outgoing connection request object. It uses
 * rdma_create_id, rpma_info_resolve_addr and rdma_resolve_route and feeds
 * the prepared ID into rpma_conn_req_from_id.
 */
int
rpma_conn_req_new(struct rpma_peer *peer, const char *addr,
		const char *port, const struct rpma_conn_cfg *cfg,
		struct rpma_conn_req **req_ptr)
{
	if (peer == NULL || addr == NULL || port == NULL || req_ptr == NULL)
		return RPMA_E_INVAL;

	if (cfg == NULL)
		cfg = rpma_conn_cfg_default();

	int timeout_ms;
	(void) rpma_conn_cfg_get_timeout(cfg, &timeout_ms);

	struct rpma_info *info;
	int ret = rpma_info_new(addr, port, RPMA_INFO_ACTIVE, &info);
	if (ret)
		return ret;

	struct rdma_cm_id *id;
	if (rdma_create_id(NULL, &id, NULL, RDMA_PS_TCP)) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_create_id()");
		ret = RPMA_E_PROVIDER;
		goto err_info_delete;
	}

	/* resolve address */
	ret = rpma_info_resolve_addr(info, id, timeout_ms);
	if (ret)
		goto err_destroy_id;

	/* resolve route */
	if (rdma_resolve_route(id, timeout_ms)) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno,
			"rdma_resolve_route(timeout_ms=%i)", timeout_ms);
		ret = RPMA_E_PROVIDER;
		goto err_destroy_id;
	}

	struct rpma_conn_req *req;
	ret = rpma_conn_req_from_id(peer, id, cfg, &req);
	if (ret)
		goto err_destroy_id;

	*req_ptr = req;

	(void) rpma_info_delete(&info);

	RPMA_LOG_NOTICE("Requesting a connection to %s:%s", addr,
				port);

	return 0;

err_destroy_id:
	(void) rdma_destroy_id(id);

err_info_delete:
	(void) rpma_info_delete(&info);
	return ret;
}

/*
 * rpma_conn_req_connect -- prepare connection parameters and request
 * connecting a connection request (either active or passive). When done
 * release the connection request object (regardless of the result).
 */
int
rpma_conn_req_connect(struct rpma_conn_req **req_ptr,
	const struct rpma_conn_private_data *pdata, struct rpma_conn **conn_ptr)
{
	if (req_ptr == NULL || conn_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_conn_req *req = *req_ptr;
	if (req == NULL)
		return RPMA_E_INVAL;

	if (pdata != NULL) {
		if (pdata->ptr == NULL || pdata->len == 0)
			return RPMA_E_INVAL;
	}

	struct rdma_conn_param conn_param = {0};
	conn_param.private_data = pdata ? pdata->ptr : NULL;
	conn_param.private_data_len = pdata ? pdata->len : 0;
	conn_param.responder_resources = RDMA_MAX_RESP_RES;
	conn_param.initiator_depth = RDMA_MAX_INIT_DEPTH;
	conn_param.flow_control = 1;
	conn_param.retry_count = 7; /* max 3-bit value */
	conn_param.rnr_retry_count = 7; /* max 3-bit value */

	int ret = 0;

	if (req->edata)
		ret = rpma_conn_req_accept(req, &conn_param, conn_ptr);
	else
		ret = rpma_conn_req_connect_active(req, &conn_param, conn_ptr);

	free(req);
	*req_ptr = NULL;

	return ret;
}

/*
 * rpma_conn_req_delete -- destroy QP and either reject (for incoming connection
 * requests) or destroy the connection request (for the outgoing one). At last
 * release the connection request object.
 */
int
rpma_conn_req_delete(struct rpma_conn_req **req_ptr)
{
	if (req_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_conn_req *req = *req_ptr;
	if (req == NULL)
		return 0;

	rdma_destroy_qp(req->id);

	int ret = 0;

	if (req->edata)
		ret = rpma_conn_req_reject(req);
	else
		ret = rpma_conn_req_destroy(req);

	rpma_private_data_discard(&req->data);

	free(req);
	*req_ptr = NULL;

	return ret;
}

/*
 * rpma_conn_req_recv -- initiate the receive operation
 */
int
rpma_conn_req_recv(struct rpma_conn_req *req,
    struct rpma_mr_local *dst, size_t offset, size_t len,
    const void *op_context)
{
	if (req == NULL || dst == NULL)
		return RPMA_E_INVAL;

	return rpma_mr_recv(req->id->qp,
			dst, offset, len,
			op_context);
}

/*
 * rpma_conn_req_get_private_data -- get a pointer to the incoming connection's
 * private data
 */
int
rpma_conn_req_get_private_data(const struct rpma_conn_req *req,
    struct rpma_conn_private_data *pdata)
{
	if (req == NULL || pdata == NULL)
		return RPMA_E_INVAL;

	pdata->ptr = req->data.ptr;
	pdata->len = req->data.len;

	return 0;
}
