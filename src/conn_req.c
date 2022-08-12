// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */
/* Copyright 2021-2022, Fujitsu */

/*
 * conn_req.c -- librpma connection-request-related implementations
 */

#include <stdlib.h>
#include <rdma/rdma_cma.h>

#include "common.h"
#include "conn.h"
#include "conn_cfg.h"
#include "conn_req.h"
#include "debug.h"
#include "info.h"
#include "log_internal.h"
#include "mr.h"
#include "peer.h"
#include "private_data.h"

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

struct rpma_conn_req {
	/* it is the passive side */
	int is_passive;
	/* CM ID of the connection request */
	struct rdma_cm_id *id;
	/* main CQ */
	struct rpma_cq *cq;
	/* receive CQ */
	struct rpma_cq *rcq;
	/* shared completion channel */
	struct ibv_comp_channel *channel;

	/* private data of the CM ID (incoming only) */
	struct rpma_conn_private_data data;

	/* a parent RPMA peer of this request - needed for derivative objects */
	struct rpma_peer *peer;
};

/*
 * rpma_snprintf_gid -- snprintf GID address to the given string (helper function)
 */
static inline int
rpma_snprintf_gid(uint8_t *raw, char *gid, size_t size)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_UNKNOWN, {});

	memset(gid, 0, size);
	int ret = snprintf(gid, size,
			"%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
			raw[0], raw[1], raw[2], raw[3], raw[4], raw[5], raw[6], raw[7], raw[8],
			raw[9], raw[10], raw[11], raw[12], raw[13], raw[14], raw[15]);
	if (ret < 0) {
		memset(gid, 0, size);
		return RPMA_E_UNKNOWN;
	}

	return 0;
}

/*
 * rpma_conn_req_new_from_id -- allocate a new conn_req object from CM ID and equip the latter
 * with QP and CQ
 *
 * ASSUMPTIONS
 * - peer != NULL && id != NULL && cfg != NULL && req_ptr != NULL
 */
static int
rpma_conn_req_new_from_id(struct rpma_peer *peer, struct rdma_cm_id *id,
		const struct rpma_conn_cfg *cfg, struct rpma_conn_req **req_ptr)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	int ret = 0;

	int cqe, rcqe;
	bool shared = false;
	struct rpma_srq *srq = NULL;
	struct rpma_cq *cq = NULL;
	struct rpma_cq *rcq = NULL;
	struct rpma_cq *srq_rcq = NULL;
	/* read the main CQ size from the configuration */
	rpma_conn_cfg_get_cqe(cfg, &cqe);
	/* read the receive CQ size from the configuration */
	rpma_conn_cfg_get_rcqe(cfg, &rcqe);
	/* get if the completion channel should be shared by CQ and RCQ */
	(void) rpma_conn_cfg_get_compl_channel(cfg, &shared);
	/* get the shared RQ object from the connection */
	(void) rpma_conn_cfg_get_srq(cfg, &srq);
	if (srq)
		(void) rpma_srq_get_rcq(srq, &srq_rcq);

	if (shared && srq_rcq) {
		RPMA_LOG_ERROR(
				"connection shared completion channel cannot be used when the shared RQ has its own RCQ");
		return RPMA_E_INVAL;
	}

	RPMA_FAULT_INJECTION(RPMA_E_PROVIDER, {});
	struct ibv_comp_channel *channel = NULL;
	if (shared) {
		/* create a completion channel */
		channel = ibv_create_comp_channel(id->verbs);
		if (channel == NULL) {
			RPMA_LOG_ERROR_WITH_ERRNO(errno, "ibv_create_comp_channel()");
			return RPMA_E_PROVIDER;
		}
	}

	ret = rpma_cq_new(id->verbs, cqe, channel, &cq);
	if (ret)
		goto err_comp_channel_destroy;

	if (!srq_rcq && rcqe) {
		ret = rpma_cq_new(id->verbs, rcqe, channel, &rcq);
		if (ret)
			goto err_rpma_cq_delete;
	}

	/* setup a QP */
	ret = rpma_peer_setup_qp(peer, id, cq, srq_rcq ? srq_rcq : rcq, cfg);
	if (ret)
		goto err_rpma_rcq_delete;

	*req_ptr = (struct rpma_conn_req *)malloc(sizeof(struct rpma_conn_req));
	if (*req_ptr == NULL) {
		ret = RPMA_E_NOMEM;
		goto err_destroy_qp;
	}

/*
 * Maximum length of GID address in the following format:
 * 0000:0000:0000:0000:0000:ffff:c0a8:6604
 */
#define GID_STR_LEN 40
	/* log GID addresses if log level >= RPMA_LOG_LEVEL_NOTICE */
	enum rpma_log_level level;
	ret = rpma_log_get_threshold(RPMA_LOG_THRESHOLD, &level);
	if (ret == 0 && level >= RPMA_LOG_LEVEL_NOTICE) {
		struct ibv_sa_path_rec *path_rec = id->route.path_rec;
		char gid[GID_STR_LEN];
		if (path_rec && !rpma_snprintf_gid(path_rec->sgid.raw, gid, GID_STR_LEN)) {
			RPMA_LOG_NOTICE("src GID = %s", gid);
		} else {
			RPMA_LOG_NOTICE("src GID is not available");
		}

		if (path_rec && !rpma_snprintf_gid(path_rec->dgid.raw, gid, GID_STR_LEN)) {
			RPMA_LOG_NOTICE("dst GID = %s", gid);
		} else {
			RPMA_LOG_NOTICE("dst GID is not available");
		}
	}
#undef GID_STR_LEN

	(*req_ptr)->is_passive = 0;
	(*req_ptr)->id = id;
	(*req_ptr)->cq = cq;
	(*req_ptr)->rcq = rcq;
	(*req_ptr)->channel = channel;
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

err_comp_channel_destroy:
	if (channel)
		(void) ibv_destroy_comp_channel(channel);

	return ret;
}

/*
 * rpma_conn_new_accept -- call rdma_accept()+rdma_ack_cm_event(). If succeeds
 * request re-packing the connection request to a connection object. Otherwise,
 * rdma_disconnect()+rdma_destroy_qp()+rpma_cq_delete() to destroy
 * the unsuccessful connection request.
 *
 * ASSUMPTIONS
 * - req != NULL && conn_param != NULL && conn_ptr != NULL
 */
static int
rpma_conn_new_accept(struct rpma_conn_req *req,
	struct rdma_conn_param *conn_param, struct rpma_conn **conn_ptr)
{
	int ret = 0;

	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION_GOTO(RPMA_E_PROVIDER, err_conn_req_delete);

	if (rdma_accept(req->id, conn_param)) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_accept()");
		ret = RPMA_E_PROVIDER;
		goto err_conn_req_delete;
	}

	struct rpma_conn *conn = NULL;
	ret = rpma_conn_new(req->peer, req->id, req->cq, req->rcq, req->channel, &conn);
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
	(void) rpma_private_data_delete(&req->data);
	if (req->channel)
		(void) ibv_destroy_comp_channel(req->channel);

	return ret;
}

/*
 * rpma_conn_new_connect -- call rdma_connect(). If succeeds request
 * re-packing the connection request to a connection object. Otherwise,
 * rdma_destroy_qp()+rpma_cq_delete()+rdma_destroy_id() to destroy
 * the unsuccessful connection request.
 *
 * ASSUMPTIONS
 * - req != NULL && conn_param != NULL && conn_ptr != NULL
 */
static int
rpma_conn_new_connect(struct rpma_conn_req *req, struct rdma_conn_param *conn_param,
	struct rpma_conn **conn_ptr)
{
	int ret = 0;

	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION_GOTO(RPMA_E_PROVIDER, err_conn_new);

	struct rpma_conn *conn = NULL;
	ret = rpma_conn_new(req->peer, req->id, req->cq, req->rcq, req->channel, &conn);
	if (ret)
		goto err_conn_new;

	RPMA_FAULT_INJECTION(RPMA_E_PROVIDER,
	{
		(void) rpma_conn_delete(&conn);
	});

	if (rdma_connect(req->id, conn_param)) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_connect()");
		(void) rpma_conn_delete(&conn);
		return RPMA_E_PROVIDER;
	}

	*conn_ptr = conn;
	return 0;

err_conn_new:
	rdma_destroy_qp(req->id);
	(void) rpma_cq_delete(&req->rcq);
	(void) rpma_cq_delete(&req->cq);
	(void) rdma_destroy_id(req->id);
	if (req->channel)
		(void) ibv_destroy_comp_channel(req->channel);

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
	RPMA_DEBUG_TRACE;

	if (rdma_reject(req->id, NULL /* private data */, 0 /* private data len */)) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_reject()");
		return RPMA_E_PROVIDER;
	}

	RPMA_FAULT_INJECTION(RPMA_E_PROVIDER, {});
	return 0;
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
	RPMA_DEBUG_TRACE;

	if (rdma_destroy_id(req->id)) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_destroy_id()");
		return RPMA_E_PROVIDER;
	}

	RPMA_FAULT_INJECTION(RPMA_E_PROVIDER, {});
	return 0;
}

/* internal librpma API */

/*
 * rpma_conn_req_new_from_cm_event -- feeds an ID from cm event into
 * rpma_conn_req_new_from_id and add the event to conn_req
 *
 * ASSUMPTIONS
 * cfg != NULL
 */
int
rpma_conn_req_new_from_cm_event(struct rpma_peer *peer, struct rdma_cm_event *event,
		const struct rpma_conn_cfg *cfg, struct rpma_conn_req **req_ptr)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (peer == NULL || event == NULL || event->event != RDMA_CM_EVENT_CONNECT_REQUEST ||
	    req_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new_from_id(peer, event->id, cfg, &req);
	if (ret)
		return ret;

	ret = rpma_private_data_store(event, &req->data);
	if (ret)
		goto err_conn_req_delete;

	req->is_passive = 1;
	*req_ptr = req;

	return 0;

err_conn_req_delete:
	(void) rpma_conn_req_delete(&req);

	return ret;
}

/* public librpma API */

/*
 * rpma_conn_req_new -- create a new outgoing connection request object. It uses
 * rdma_create_id, rpma_info_resolve_addr and rdma_resolve_route and feeds
 * the prepared ID into rpma_conn_req_new_from_id.
 */
int
rpma_conn_req_new(struct rpma_peer *peer, const char *addr, const char *port,
		const struct rpma_conn_cfg *cfg, struct rpma_conn_req **req_ptr)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (peer == NULL || addr == NULL || port == NULL || req_ptr == NULL)
		return RPMA_E_INVAL;

	if (cfg == NULL)
		cfg = rpma_conn_cfg_default();

	int timeout_ms = 0;
	(void) rpma_conn_cfg_get_timeout(cfg, &timeout_ms);

	struct rpma_info *info;
	int ret = rpma_info_new(addr, port, RPMA_INFO_ACTIVE, &info);
	if (ret)
		return ret;

	struct rdma_cm_id *id;
	RPMA_FAULT_INJECTION_GOTO(RPMA_E_PROVIDER, err_info_delete);
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
	RPMA_FAULT_INJECTION_GOTO(RPMA_E_PROVIDER, err_destroy_id);
	if (rdma_resolve_route(id, timeout_ms)) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_resolve_route(timeout_ms=%i)", timeout_ms);
		ret = RPMA_E_PROVIDER;
		goto err_destroy_id;
	}

	struct rpma_conn_req *req;
	ret = rpma_conn_req_new_from_id(peer, id, cfg, &req);
	if (ret)
		goto err_destroy_id;

	*req_ptr = req;

	(void) rpma_info_delete(&info);

	RPMA_LOG_NOTICE("Requesting a connection to %s:%s", addr, port);

	return 0;

err_destroy_id:
	(void) rdma_destroy_id(id);

err_info_delete:
	(void) rpma_info_delete(&info);
	return ret;
}

/*
 * rpma_conn_req_connect -- prepare connection parameters and request connecting
 * a connection request (either active or passive). When done release (delete)
 * the connection request object (regardless of the result).
 */
int
rpma_conn_req_connect(struct rpma_conn_req **req_ptr, const struct rpma_conn_private_data *pdata,
	struct rpma_conn **conn_ptr)
{
	RPMA_DEBUG_TRACE;

	if (req_ptr == NULL || *req_ptr == NULL)
		return RPMA_E_INVAL;

	RPMA_FAULT_INJECTION(RPMA_E_INVAL,
	{
		(void) rpma_conn_req_delete(req_ptr);
	});

	if (conn_ptr == NULL || (pdata != NULL && (pdata->ptr == NULL || pdata->len == 0))) {
		(void) rpma_conn_req_delete(req_ptr);
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
	if ((*req_ptr)->is_passive)
		ret = rpma_conn_new_accept(*req_ptr, &conn_param, conn_ptr);
	else
		ret = rpma_conn_new_connect(*req_ptr, &conn_param, conn_ptr);

	free(*req_ptr);
	*req_ptr = NULL;

	return ret;
}

/*
 * rpma_conn_req_delete -- destroy QP and either reject (for incoming connection requests)
 * or destroy the connection request (for the outgoing one). At last release the connection
 * request object.
 */
int
rpma_conn_req_delete(struct rpma_conn_req **req_ptr)
{
	RPMA_DEBUG_TRACE;

	if (req_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_conn_req *req = *req_ptr;
	if (req == NULL)
		return 0;

	rdma_destroy_qp(req->id);

	int ret = rpma_cq_delete(&req->rcq);

	int ret2 = rpma_cq_delete(&req->cq);
	if (!ret && ret2)
		ret = ret2;

	if (req->is_passive)
		ret2 = rpma_conn_req_reject(req);
	else
		ret2 = rpma_conn_req_destroy(req);
	if (!ret && ret2)
		ret = ret2;

	if (req->channel) {
		errno = ibv_destroy_comp_channel(req->channel);
		if (errno) {
			RPMA_LOG_ERROR_WITH_ERRNO(errno,
				"ibv_destroy_comp_channel()");
			if (!ret)
				ret = RPMA_E_PROVIDER;
		}
	}

	rpma_private_data_delete(&req->data);

	free(req);
	*req_ptr = NULL;

	RPMA_FAULT_INJECTION(RPMA_E_PROVIDER, {});
	return ret;
}

/*
 * rpma_conn_req_recv -- initiate the receive operation
 */
int
rpma_conn_req_recv(struct rpma_conn_req *req, struct rpma_mr_local *dst, size_t offset, size_t len,
    const void *op_context)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (req == NULL || dst == NULL)
		return RPMA_E_INVAL;

	return rpma_mr_recv(req->id->qp, dst, offset, len, op_context);
}

/*
 * rpma_conn_req_get_private_data -- get a pointer to the incoming connection's private data
 */
int
rpma_conn_req_get_private_data(const struct rpma_conn_req *req,
    struct rpma_conn_private_data *pdata)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (req == NULL || pdata == NULL)
		return RPMA_E_INVAL;

	pdata->ptr = req->data.ptr;
	pdata->len = req->data.len;

	return 0;
}
