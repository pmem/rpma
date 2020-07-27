/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn_req.c -- librpma connection-request-related implementations
 */

#include <stdlib.h>
#include <rdma/rdma_cma.h>

#include "conn.h"
#include "conn_req.h"
#include "info.h"
#include "peer.h"
#include "private_data.h"
#include "rpma_err.h"

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

struct rpma_conn_req {
	/* RDMA_CM_EVENT_CONNECT_REQUEST event (if applicable) */
	struct rdma_cm_event *edata;
	/* CM ID of the connection request */
	struct rdma_cm_id *id;
	/* completion queue of the CM ID */
	struct ibv_cq *cq;

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
 * - peer != NULL && id != NULL && req != NULL
 */
static int
rpma_conn_req_from_id(struct rpma_peer *peer, struct rdma_cm_id *id,
		struct rpma_conn_req **req)
{
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

	*req = (struct rpma_conn_req *)malloc(sizeof(struct rpma_conn_req));
	if (*req == NULL) {
		ret = RPMA_E_NOMEM;
		goto err_destroy_qp;
	}

	(*req)->edata = NULL;
	(*req)->id = id;
	(*req)->cq = cq;
	(*req)->data.ptr = NULL;
	(*req)->data.len = 0;
	(*req)->peer = peer;

	return 0;

err_destroy_qp:
	rdma_destroy_qp(id);

err_destroy_cq:
	(void) ibv_destroy_cq(cq);

	return ret;
}

/*
 * rpma_conn_req_accept -- call rdma_accept()+rdma_ack_cm_event(). If succeeds
 * request re-packing the connection request to a connection object. Otherwise,
 * rdma_disconnect()+rdma_destroy_qp()+ibv_destroy_cq() to destroy
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
	ret = rpma_conn_new(req->peer, req->id, req->cq, &conn);
	if (ret)
		goto err_conn_disconnect;

	ret = rpma_conn_set_private_data(conn, &req->data);
	if (ret)
		goto err_conn_delete;

	*conn_ptr = conn;
	return 0;

err_conn_delete:
	(void) rpma_conn_delete(&conn);

err_conn_disconnect:
	(void) rdma_disconnect(req->id);

err_conn_req_delete:
	rdma_destroy_qp(req->id);
	(void) ibv_destroy_cq(req->cq);

	return ret;
}

/*
 * rpma_conn_req_connect_active -- call rdma_connect(). If succeeds request
 * re-packing the connection request to a connection object. Otherwise,
 * rdma_destroy_qp()+ibv_destroy_cq()+rdma_destroy_id() to destroy
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
	int provider_error = 0;

	struct rpma_conn *conn = NULL;
	ret = rpma_conn_new(req->peer, req->id, req->cq, &conn);
	if (ret)
		goto err_conn_req_delete;

	if (rdma_connect(req->id, conn_param)) {
		provider_error = errno;
		ret = RPMA_E_PROVIDER;
		goto err_conn_delete;
	}

	*conn_ptr = conn;
	return 0;

err_conn_delete:
	(void) rpma_conn_delete(&conn);
	Rpma_provider_error = provider_error;

err_conn_req_delete:
	rdma_destroy_qp(req->id);
	(void) ibv_destroy_cq(req->cq);
	(void) rdma_destroy_id(req->id);

	return ret;
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
	int ret = 0;

	Rpma_provider_error = ibv_destroy_cq(req->cq);
	if (Rpma_provider_error) {
		ret = RPMA_E_PROVIDER;
	}

	if (rdma_reject(req->id,
			NULL /* private data */,
			0 /* private data len */)) {
		if (!ret) {
			Rpma_provider_error = errno;
			ret = RPMA_E_PROVIDER;
		}
	}

	if (rdma_ack_cm_event(req->edata)) {
		if (!ret) {
			Rpma_provider_error = errno;
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
 * release the connection request object (regardless of the result).
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
