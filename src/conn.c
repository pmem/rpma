/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn.c -- librpma connection-related implementations
 */

#include <stdlib.h>

#include "conn.h"
#include "flush.h"
#include "mr.h"
#include "private_data.h"
#include "rpma_err.h"

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

struct rpma_conn {
	struct rdma_cm_id *id; /* a CM ID of the connection */
	struct rdma_event_channel *evch; /* event channel of the CM ID */
	struct ibv_cq *cq; /* completion queue of the CM ID */

	struct rpma_conn_private_data data; /* private data of the CM ID */
	struct rpma_flush *flush; /* flushing object */
};

/* internal librpma API */

/*
 * rpma_conn_new -- migrate an obtained CM ID into newly created event channel.
 * If succeeded wrap provided entities into a newly created connection object.
 *
 * Note: rdma_migrate_id(3) will block if the previous event channel of the CM
 * ID has any outstanding (unacknowledged) events.
 */
int
rpma_conn_new(struct rdma_cm_id *id, struct ibv_cq *cq,
		struct rpma_conn **conn_ptr)
{
	if (id == NULL || cq == NULL || conn_ptr == NULL)
		return RPMA_E_INVAL;

	int ret = 0;

	struct rdma_event_channel *evch = rdma_create_event_channel();
	if (!evch) {
		Rpma_provider_error = errno;
		return RPMA_E_PROVIDER;
	}

	if (rdma_migrate_id(id, evch)) {
		Rpma_provider_error = errno;
		ret = RPMA_E_PROVIDER;
		goto err_destroy_evch;
	}

	struct rpma_conn *conn = malloc(sizeof(*conn));
	if (!conn) {
		ret = RPMA_E_NOMEM;
		goto err_migrate_id_NULL;
	}

	conn->id = id;
	conn->evch = evch;
	conn->cq = cq;
	conn->data.ptr = NULL;
	conn->data.len = 0;

	/* XXX peer is needed here to call rpma_flush_new() */
	conn->flush = NULL;
	*conn_ptr = conn;

	return 0;

err_migrate_id_NULL:
	(void) rdma_migrate_id(id, NULL);

err_destroy_evch:
	rdma_destroy_event_channel(evch);

	return ret;
}

/*
 * rpma_conn_set_private_data -- allocate a buffer and fill
 * the private data of the CM ID
 */
int
rpma_conn_set_private_data(struct rpma_conn *conn,
		struct rpma_conn_private_data *pdata)
{
	return rpma_private_data_copy(&conn->data, pdata);
}

/* public librpma API */

/*
 * rpma_conn_next_event -- obtain the next event from the connection
 */
int
rpma_conn_next_event(struct rpma_conn *conn, enum rpma_conn_event *event)
{
	int ret;

	if (conn == NULL || event == NULL)
		return RPMA_E_INVAL;

	struct rdma_cm_event *edata = NULL;
	if (rdma_get_cm_event(conn->evch, &edata)) {
		Rpma_provider_error = errno;
		return RPMA_E_PROVIDER;
	}

	if (edata->event == RDMA_CM_EVENT_ESTABLISHED &&
			conn->data.ptr == NULL) {
		ret = rpma_private_data_store(edata, &conn->data);
		if (ret) {
			(void) rdma_ack_cm_event(edata);
			return ret;
		}
	}

	enum rdma_cm_event_type cm_event = edata->event;
	if (rdma_ack_cm_event(edata)) {
		Rpma_provider_error = errno;
		ret = RPMA_E_PROVIDER;
		goto err_private_data_discard;
	}

	switch (cm_event) {
		case RDMA_CM_EVENT_ESTABLISHED:
			*event = RPMA_CONN_ESTABLISHED;
			break;
		case RDMA_CM_EVENT_CONNECT_ERROR:
		case RDMA_CM_EVENT_DEVICE_REMOVAL:
			*event = RPMA_CONN_LOST;
			break;
		case RDMA_CM_EVENT_DISCONNECTED:
		case RDMA_CM_EVENT_TIMEWAIT_EXIT:
			*event = RPMA_CONN_CLOSED;
			break;
		default:
			return RPMA_E_UNKNOWN;
	}

	return 0;

err_private_data_discard:
	rpma_private_data_discard(&conn->data);

	return ret;
}

/*
 * rpma_conn_get_private_data -- hand a pointer to the connection's private data
 */
int
rpma_conn_get_private_data(struct rpma_conn *conn,
		struct rpma_conn_private_data *pdata)
{
	if (conn == NULL || pdata == NULL)
		return RPMA_E_INVAL;

	pdata->ptr = conn->data.ptr;
	pdata->len = conn->data.len;

	return 0;
}

/*
 * rpma_conn_disconnect -- disconnect the connection
 */
int
rpma_conn_disconnect(struct rpma_conn *conn)
{
	if (conn == NULL)
		return RPMA_E_INVAL;

	if (rdma_disconnect(conn->id)) {
		Rpma_provider_error = errno;
		return RPMA_E_PROVIDER;
	}

	return 0;
}

/*
 * rpma_conn_delete -- delete the connection object
 */
int
rpma_conn_delete(struct rpma_conn **conn_ptr)
{
	if (conn_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_conn *conn = *conn_ptr;
	if (conn == NULL)
		return 0;

	int ret = 0;

	/* XXX call rpma_flush_delete() */

	rdma_destroy_qp(conn->id);

	Rpma_provider_error = ibv_destroy_cq(conn->cq);
	if (Rpma_provider_error) {
		ret = RPMA_E_PROVIDER;
		goto err_destroy_id;
	}

	if (rdma_destroy_id(conn->id)) {
		Rpma_provider_error = errno;
		ret = RPMA_E_PROVIDER;
		goto err_destroy_event_channel;
	}

	rdma_destroy_event_channel(conn->evch);
	rpma_private_data_discard(&conn->data);

	free(conn);
	*conn_ptr = NULL;

	return 0;

err_destroy_id:
	(void) rdma_destroy_id(conn->id);
err_destroy_event_channel:
	rdma_destroy_event_channel(conn->evch);

	free(conn);
	*conn_ptr = NULL;

	return ret;
}

/*
 * rpma_read -- initialize the read operation
 */
int
rpma_read(struct rpma_conn *conn,
	struct rpma_mr_local *dst, size_t dst_offset,
	struct rpma_mr_remote *src,  size_t src_offset,
	size_t len, int flags, void *op_context)
{
	if (conn == NULL || dst == NULL || src == NULL || flags == 0)
		return RPMA_E_INVAL;

	return rpma_mr_read(conn->id->qp,
			dst, dst_offset,
			src, src_offset,
			len, flags, op_context);
}

/*
 * rpma_write -- initialize the write operation
 */
int
rpma_write(struct rpma_conn *conn,
	struct rpma_mr_remote *dst, size_t dst_offset,
	struct rpma_mr_local *src,  size_t src_offset,
	size_t len, int flags, void *op_context)
{
	if (conn == NULL || dst == NULL || src == NULL || flags == 0)
		return RPMA_E_INVAL;

	return rpma_mr_write(conn->id->qp,
			dst, dst_offset,
			src, src_offset,
			len, flags, op_context);
}

/*
 * rpma_flush -- initialize the flush operation
 *
 * XXX use conn->flush->func
 */
int
rpma_flush(struct rpma_conn *conn,
	struct rpma_mr_remote *dst, size_t dst_offset, size_t len,
	enum rpma_flush_type type, int flags, void *op_context)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_next_completion -- receive an operation completion
 */
int
rpma_conn_next_completion(struct rpma_conn *conn, struct rpma_completion *cmpl)
{
	if (conn == NULL || cmpl == NULL)
		return RPMA_E_INVAL;

	struct ibv_wc wc = {0};
	int result = ibv_poll_cq(conn->cq, 1 /* num_entries */, &wc);
	if (result == 0) {
		return RPMA_E_NO_COMPLETION;
	} else if (result < 0) {
		Rpma_provider_error = result;
		return RPMA_E_PROVIDER;
	} else if (result > 1) {
		return RPMA_E_UNKNOWN;
	}

	switch (wc.opcode) {
	case IBV_WC_RDMA_READ:
		cmpl->op = RPMA_OP_READ;
		break;
	case IBV_WC_RDMA_WRITE:
		cmpl->op = RPMA_OP_WRITE;
		break;
	default:
		return RPMA_E_NOSUPP;
	}

	cmpl->op_context = (void *)wc.wr_id;
	cmpl->op_status = wc.status;

	return 0;
}
