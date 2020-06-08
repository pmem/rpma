/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn_req.c -- librpma connection-request-related implementations
 */

#include <rdma/rdma_cma.h>

#include "cmocka_alloc.h"
#include "conn.h"
#include "conn_req.h"
#include "info.h"
#include "out.h"
#include "peer.h"
#include "rpma_err.h"

struct rpma_conn_req {
	/* RDMA_CM_EVENT_CONNECT_REQUEST event (if applicable) */
	struct rdma_cm_event *edata;
	/* CM ID of the connection request */
	struct rdma_cm_id *id;
	/* completion queue of the CM ID */
	struct ibv_cq *cq;
};

/*
 * rpma_conn_req_from_id -- allocate a new conn_req object from CM ID and equip
 * the latter with QP and CQ
 */
static int
rpma_conn_req_from_id(struct rpma_peer *peer, struct rdma_cm_id *id,
		struct rpma_conn_req **req)
{
	ASSERTne(peer, NULL);
	ASSERTne(id, NULL);
	ASSERTne(req, NULL);

	/* create a CQ */
	struct ibv_cq *cq = ibv_create_cq(id->verbs, RPMA_DEFAULT_Q_SIZE,
				NULL /* cq_context */,
				NULL /* channel */,
				0 /* comp_vector */);
	if (cq == NULL) {
		Rpma_provider_error = errno;
		return RPMA_E_PROVIDER;
	}

	/* create a QP */
	int ret = rpma_peer_create_qp(peer, id, cq);
	if (ret)
		goto err_destroy_cq;

	*req = (struct rpma_conn_req *)Malloc(sizeof(struct rpma_conn_req));
	if (*req == NULL) {
		ASSERTeq(errno, ENOMEM);
		ret = RPMA_E_NOMEM;
		goto err_destroy_qp;
	}

	(*req)->edata = NULL;
	(*req)->id = id;
	(*req)->cq = cq;

	return 0;

err_destroy_qp:
	rdma_destroy_qp(id);

err_destroy_cq:
	(void) ibv_destroy_cq(cq);

	return ret;
}

/*
 * rpma_conn_req_accept -- call rdma_accept()+rdma_ack_cm_event(). If succeeds
 * request re-packing the connection request to a connection object.
 */
static int
rpma_conn_req_accept(struct rpma_conn_req *req,
	struct rdma_conn_param *conn_param, struct rpma_conn **conn_ptr)
{
	ASSERTne(req, NULL);
	ASSERTne(conn_param, NULL);
	ASSERTne(conn_ptr, NULL);

	int ret = 0;

	if (rdma_accept(req->id, conn_param)) {
		Rpma_provider_error = errno;
		ret = RPMA_E_PROVIDER;
		(void) rdma_ack_cm_event(req->edata);
		goto err_conn_req_delete;
	}

	/* ACK the connection request event */
	if (rdma_ack_cm_event(req->edata)) {
		Rpma_provider_error = errno;
		ret = RPMA_E_PROVIDER;
		goto err_conn_disconnect;
	}

	struct rpma_conn *conn = NULL;
	ret = rpma_conn_new(req->id, req->cq, &conn);
	if (ret)
		goto err_conn_disconnect;

	*conn_ptr = conn;
	return 0;

err_conn_disconnect:
	(void) rdma_disconnect(req->id);

err_conn_req_delete:
	rdma_destroy_qp(req->id);
	(void) ibv_destroy_cq(req->cq);

	return ret;
}

/*
 * rpma_conn_req_connect_active -- call rdma_connect(). If succeeds request
 * re-packing the connection request to a connection object.
 */
static int
rpma_conn_req_connect_active(struct rpma_conn_req *req,
	struct rdma_conn_param *conn_param, struct rpma_conn **conn_ptr)
{
	ASSERTne(req, NULL);
	ASSERTne(conn_param, NULL);
	ASSERTne(conn_ptr, NULL);

	int ret = 0;

	if (rdma_connect(req->id, conn_param)) {
		Rpma_provider_error = errno;
		ret = RPMA_E_PROVIDER;
		goto err_conn_req_delete;
	}

	struct rpma_conn *conn = NULL;
	ret = rpma_conn_new(req->id, req->cq, &conn);
	if (ret)
		goto err_conn_disconnect;

	*conn_ptr = conn;
	return 0;

err_conn_disconnect:
	(void) rdma_disconnect(req->id);

err_conn_req_delete:
	rdma_destroy_qp(req->id);
	(void) ibv_destroy_cq(req->cq);
	(void) rdma_destroy_id(req->id);

	return ret;
}

/*
 * rpma_conn_req_reject -- destroy CQ of the CM ID and reject the connection.
 */
static int
rpma_conn_req_reject(struct rpma_conn_req *req)
{
	ASSERTne(req, NULL);

	int ret = 0;

	Rpma_provider_error = ibv_destroy_cq(req->cq);
	if (Rpma_provider_error) {
		ret = RPMA_E_PROVIDER;
		goto err_reject;
	}

	if (rdma_reject(req->id,
			NULL /* private data */,
			0 /* private data len */)) {
		Rpma_provider_error = errno;
		ret = RPMA_E_PROVIDER;
		goto err_ack;
	}

	if (rdma_ack_cm_event(req->edata)) {
		Rpma_provider_error = errno;
		ret = RPMA_E_PROVIDER;
	}

	return ret;

err_reject:
	(void) rdma_reject(req->id, NULL, 0);

err_ack:
	(void) rdma_ack_cm_event(req->edata);

	return ret;
}

/*
 * rpma_conn_req_destroy -- destroy CQ of the CM ID and destroy the CM ID.
 */
static int
rpma_conn_req_destroy(struct rpma_conn_req *req)
{
	ASSERTne(req, NULL);

	Rpma_provider_error = ibv_destroy_cq(req->cq);
	if (Rpma_provider_error) {
		(void) rdma_destroy_id(req->id);
		return RPMA_E_PROVIDER;
	}

	if (rdma_destroy_id(req->id)) {
		Rpma_provider_error = errno;
		return RPMA_E_PROVIDER;
	}

	return 0;
}

/* internal librpma API */

/*
 * rpma_conn_req_from_cm_event -- XXX feeds an ID from cm event into
 * rpma_conn_req_from_id and add the event to conn_req
 */
int
rpma_conn_req_from_cm_event(struct rpma_peer *peer, struct rdma_cm_event *edata,
		struct rpma_conn_req **req_ptr)
{
	if (peer == NULL || edata == NULL || req_ptr == NULL)
		return RPMA_E_INVAL;

	if (edata->event != RDMA_CM_EVENT_CONNECT_REQUEST)
		return RPMA_E_INVAL;

	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_id(peer, edata->id, &req);
	if (ret)
		return ret;

	req->edata = edata;
	*req_ptr = req;

	return 0;
}

/* public librpma API */

/*
 * rpma_conn_req_new -- create a new outgoing connection request object. It uses
 * rdma_create_id, rpma_info_resolve_addr and rdma_resolve_route and feeds the
 * prepared ID into rpma_conn_req_from_id.
 */
int
rpma_conn_req_new(struct rpma_peer *peer, const char *addr, const char *service,
		struct rpma_conn_req **req_ptr)
{
	if (peer == NULL || addr == NULL || service == NULL || req_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_info *info;
	int ret = rpma_info_new(addr, service, RPMA_INFO_ACTIVE, &info);
	if (ret)
		return ret;

	struct rdma_cm_id *id;
	if (rdma_create_id(NULL, &id, NULL, RDMA_PS_TCP)) {
		Rpma_provider_error = errno;
		ret = RPMA_E_PROVIDER;
		goto err_info_delete;
	}

	/* resolve address */
	ret = rpma_info_resolve_addr(info, id);
	if (ret)
		goto err_destroy_id;

	/* resolve route */
	if (rdma_resolve_route(id, RPMA_DEFAULT_TIMEOUT)) {
		Rpma_provider_error = errno;
		ret = RPMA_E_PROVIDER;
		goto err_destroy_id;
	}

	struct rpma_conn_req *req;
	ret = rpma_conn_req_from_id(peer, id, &req);
	if (ret)
		goto err_destroy_id;

	*req_ptr = req;

	(void) rpma_info_delete(&info);

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
 * release the connection request object.
 */
int
rpma_conn_req_connect(struct rpma_conn_req **req_ptr,
	struct rpma_conn_private_data *pdata, struct rpma_conn **conn_ptr)
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

	Free(req);
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

	Free(req);
	*req_ptr = NULL;

	return ret;
}
