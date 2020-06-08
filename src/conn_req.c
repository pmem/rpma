/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn_req.c -- librpma connection-request-related implementations
 */

#include "cmocka_alloc.h"
#include "conn.h"
#include "conn_req.h"
#include "rpma_err.h"

struct rpma_conn_req {
	/* RDMA_CM_EVENT_CONNECT_REQUEST event (if applicable) */
	struct rdma_cm_event *edata;
	/* CM ID of the connection request */
	struct rdma_cm_id *id;
	/* event channel of the CM ID */
	struct rdma_event_channel *evch;
	/* completion queue of the CM ID */
	struct ibv_cq *cq;
};

/*
 * rpma_conn_req_from_id -- XXX uses ibv_create_cq, rpma_peer_create_qp and
 * allocate the conn_req object
 *
 * XXX open: is it possible to create QP for not accepted nor rejected
 * connection request?
 */
static int
rpma_conn_req_from_id(struct rpma_peer *peer, struct rdma_cm_id *id,
		struct rpma_conn_req **req)
{
	return RPMA_E_NOSUPP;
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

	if (edata->event == RDMA_CM_EVENT_CONNECT_REQUEST)
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
 * rpma_conn_req_new -- XXX uses rdma_create_id, rpma_info_resolve_addr and
 * rdma_resolve_route and feed the prepared ID into rpma_conn_req_from_id
 */
int
rpma_conn_req_new(struct rpma_peer *peer, const char *addr, const char *service,
		struct rpma_conn_req **req)
{
	(void) rpma_conn_req_from_id;

	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_req_connect -- request re-packing the connection request to
 * a connection object. If succeeds call either rdma_accept() or rdma_connect().
 * If all succeeds release the connection request object.
 */
int
rpma_conn_req_connect(struct rpma_conn_req **req_ptr, const void *private_data,
		uint8_t private_data_len, struct rpma_conn **conn_ptr)
{
	if (req_ptr == NULL || conn_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_conn_req *req = *req_ptr;
	if (req == NULL)
		return RPMA_E_INVAL;

	if (private_data == NULL && private_data_len != 0)
		return RPMA_E_INVAL;

	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(req->id, req->evch, req->cq, &conn);
	if (ret)
		return ret;

	struct rdma_conn_param conn_param;
	memset(&conn_param, 0, sizeof(conn_param));
	conn_param.private_data = private_data;
	conn_param.private_data_len = private_data_len;
	conn_param.responder_resources = RDMA_MAX_RESP_RES;
	conn_param.initiator_depth = RDMA_MAX_INIT_DEPTH;
	conn_param.flow_control = 1;
	conn_param.retry_count = 7; /* max 3-bit value */
	conn_param.rnr_retry_count = 7; /* max 3-bit value */

	if (req->edata) {
		/* passive side of the connection */
		if (rdma_accept(req->id, &conn_param)) {
			Rpma_provider_error = errno;
			ret = RPMA_E_PROVIDER;
			goto err_conn_delete;
		}

		/* ACK the connection request event */
		if (rdma_ack_cm_event(req->edata)) {
			Rpma_provider_error = errno;
			ret = RPMA_E_PROVIDER;
			goto err_conn_disconnect;
		}

		req->edata = NULL;
	} else {
		/* active side of the connection */
		if (rdma_connect(req->id, &conn_param)) {
			Rpma_provider_error = errno;
			ret = RPMA_E_PROVIDER;
			goto err_conn_delete;
		}
	}

	Free(req);
	*req_ptr = NULL;
	*conn_ptr = conn;

	return 0;

err_conn_disconnect:
	(void) rdma_disconnect(req->id);
err_conn_delete:
	(void) rpma_conn_delete(&conn);
	return ret;
}

/*
 * rpma_conn_req_delete -- XXX teardown of the conn_req object
 *
 * XXX if req->edate != NULL -> rdma_reject + rdma_ack_cm_event
 */
int
rpma_conn_req_delete(struct rpma_conn_req **req_ptr)
{
	return RPMA_E_NOSUPP;
}
