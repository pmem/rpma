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
 * rpma_conn_req_connect -- XXX uses either rdma_accept + rdma_ack_cm_event
 * or rdma_accept to transform rpma_conn_req into rpma_conn
 */
int
rpma_conn_req_connect(struct rpma_conn_req **req_ptr, const void *private_data,
		uint8_t private_data_len, struct rpma_conn **conn)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_req_delete -- destroy QP and CQ of the CM ID and reject the
 * connection or XXX. Finally, free the connection request object.
 */
int
rpma_conn_req_delete(struct rpma_conn_req **req_ptr)
{
	if (req_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_conn_req *req = *req_ptr;
	if (req == NULL)
		return 0;

	int ret = 0;

	rdma_destroy_qp(req->id);

	Rpma_provider_error = ibv_destroy_cq(req->cq);
	if (Rpma_provider_error) {
		ret = RPMA_E_PROVIDER;
		goto err_reject_or_XXX;
	}

	if (req->edata) {
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
			goto err_free;
		}
	} else {
		/*
		 * XXX to be implemented when rpma_conn_req_new() will be
		 * ready
		 */
		ASSERT(0);
	}

	Free(req);
	*req_ptr = NULL;

	return 0;

err_reject_or_XXX:
	if (req->edata == NULL) {
		/*
		 * XXX to be implemented when rpma_conn_req_new() will be
		 * ready
		 */
		ASSERT(0);
	}

	/* error path when req->edata != NULL */
	(void) rdma_reject(req->id, NULL, 0);

err_ack:
	(void) rdma_ack_cm_event(req->edata);

err_free:
	Free(req);
	*req_ptr = NULL;

	return ret;
}
