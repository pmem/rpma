// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn.c -- librpma connection-related implementations
 */

#include <inttypes.h>
#include <stdlib.h>

#include "common.h"
#include "conn.h"
#include "flush.h"
#include "log_internal.h"
#include "mr.h"
#include "private_data.h"
#include "rpma_err.h"

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

struct rpma_conn {
	struct rdma_cm_id *id; /* a CM ID of the connection */
	struct rdma_event_channel *evch; /* event channel of the CM ID */
	struct ibv_comp_channel *channel; /* completion event channel */
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
rpma_conn_new(struct rpma_peer *peer, struct rdma_cm_id *id,
		struct ibv_cq *cq, struct rpma_conn **conn_ptr)
{
	if (peer == NULL || id == NULL || cq == NULL || conn_ptr == NULL)
		return RPMA_E_INVAL;

	int ret = 0;

	struct rdma_event_channel *evch = rdma_create_event_channel();
	if (!evch) {
		Rpma_provider_error = errno;
		RPMA_LOG_ERROR_WITH_ERRNO(Rpma_provider_error,
				"rdma_create_event_channel()");
		return RPMA_E_PROVIDER;
	}

	if (rdma_migrate_id(id, evch)) {
		Rpma_provider_error = errno;
		RPMA_LOG_ERROR_WITH_ERRNO(Rpma_provider_error,
				"rdma_migrate_id()");
		ret = RPMA_E_PROVIDER;
		goto err_destroy_evch;
	}

	struct rpma_flush *flush;
	ret = rpma_flush_new(peer, &flush);
	if (ret)
		goto err_migrate_id_NULL;

	struct rpma_conn *conn = malloc(sizeof(*conn));
	if (!conn) {
		ret = RPMA_E_NOMEM;
		goto err_flush_delete;
	}

	conn->id = id;
	conn->evch = evch;
	conn->channel = cq->channel;
	conn->cq = cq;
	conn->data.ptr = NULL;
	conn->data.len = 0;
	conn->flush = flush;

	*conn_ptr = conn;

	return 0;

err_flush_delete:
	(void) rpma_flush_delete(&flush);

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
 * rpma_conn_get_event_fd -- get a file descriptor of the event channel
 * associated with the connection
 */
int
rpma_conn_get_event_fd(struct rpma_conn *conn, int *fd)
{
	if (conn == NULL || fd == NULL)
		return RPMA_E_INVAL;

	*fd = conn->evch->fd;

	return 0;
}

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
		if (errno == ENODATA)
			return RPMA_E_NO_NEXT;

		Rpma_provider_error = errno;
		RPMA_LOG_ERROR_WITH_ERRNO(Rpma_provider_error,
				"rdma_get_cm_event()");
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
		RPMA_LOG_ERROR_WITH_ERRNO(Rpma_provider_error,
				"rdma_ack_cm_event()");
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

	RPMA_LOG_NOTICE("%s", rpma_utils_conn_event_2str(*event));

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
		RPMA_LOG_ERROR_WITH_ERRNO(Rpma_provider_error,
				"rdma_disconnect()");
		return RPMA_E_PROVIDER;
	}

	RPMA_LOG_NOTICE("Requesting for disconnection");

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

	ret = rpma_flush_delete(&conn->flush);
	if (ret)
		goto err_destroy_cq;

	rdma_destroy_qp(conn->id);

	Rpma_provider_error = ibv_destroy_cq(conn->cq);
	if (Rpma_provider_error) {
		RPMA_LOG_ERROR_WITH_ERRNO(Rpma_provider_error,
				"ibv_destroy_cq()");
		ret = RPMA_E_PROVIDER;
		goto err_destroy_comp_channel;
	}

	Rpma_provider_error = ibv_destroy_comp_channel(conn->channel);
	if (Rpma_provider_error) {
		RPMA_LOG_ERROR_WITH_ERRNO(Rpma_provider_error,
				"ibv_destroy_comp_channel()");
		ret = RPMA_E_PROVIDER;
		goto err_destroy_id;
	}

	if (rdma_destroy_id(conn->id)) {
		Rpma_provider_error = errno;
		RPMA_LOG_ERROR_WITH_ERRNO(Rpma_provider_error,
				"rdma_destroy_id()");
		ret = RPMA_E_PROVIDER;
		goto err_destroy_event_channel;
	}

	rdma_destroy_event_channel(conn->evch);
	rpma_private_data_discard(&conn->data);

	free(conn);
	*conn_ptr = NULL;

	return 0;

err_destroy_cq:
	(void) ibv_destroy_cq(conn->cq);
err_destroy_comp_channel:
	(void) ibv_destroy_comp_channel(conn->channel);
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
 * rpma_write_atomic -- initialize the atomic write operation
 */
int
rpma_write_atomic(struct rpma_conn *conn,
	struct rpma_mr_remote *dst, size_t dst_offset,
	struct rpma_mr_local *src,  size_t src_offset,
	int flags, void *op_context)
{
	if (conn == NULL || dst == NULL || src == NULL || flags == 0)
		return RPMA_E_INVAL;

	if (dst_offset % RPMA_ATOMIC_WRITE_ALIGNMENT != 0)
		return RPMA_E_INVAL;

	return rpma_mr_write(conn->id->qp,
			dst, dst_offset,
			src, src_offset,
			RPMA_ATOMIC_WRITE_ALIGNMENT, flags, op_context);
}

/*
 * rpma_flush -- initialize the flush operation
 */
int
rpma_flush(struct rpma_conn *conn,
	struct rpma_mr_remote *dst, size_t dst_offset, size_t len,
	enum rpma_flush_type type, int flags, void *op_context)
{
	if (conn == NULL || dst == NULL || flags == 0)
		return RPMA_E_INVAL;

	rpma_flush_func flush = conn->flush->func;
	return flush(conn->id->qp, conn->flush, dst, dst_offset,
			len, type, flags, op_context);
}

/*
 * rpma_send -- initialize the send operation
 */
int
rpma_send(struct rpma_conn *conn,
    struct rpma_mr_local *src, size_t offset, size_t len,
    int flags, void *op_context)
{
	if (conn == NULL || src == NULL || flags == 0)
		return RPMA_E_INVAL;

	return rpma_mr_send(conn->id->qp,
			src, offset, len,
			flags, op_context);
}

/*
 * rpma_recv -- initialize the receive operation
 */
int
rpma_recv(struct rpma_conn *conn,
    struct rpma_mr_local *dst, size_t offset, size_t len,
    void *op_context)
{
	if (conn == NULL || dst == NULL)
		return RPMA_E_INVAL;

	return rpma_mr_recv(conn->id->qp,
			dst, offset, len,
			op_context);
}

/*
 * rpma_conn_get_completion_fd -- get a file descriptor of the completion event
 * channel associated with the connection
 */
int
rpma_conn_get_completion_fd(struct rpma_conn *conn, int *fd)
{
	if (conn == NULL || fd == NULL)
		return RPMA_E_INVAL;

	*fd = conn->channel->fd;

	return 0;
}

/*
 * rpma_conn_prepare_completions -- wait for completions
 */
int
rpma_conn_prepare_completions(struct rpma_conn *conn)
{
	if (conn == NULL)
		return RPMA_E_INVAL;

	/* wait for the completion event */
	struct ibv_cq *ev_cq;	/* unused */
	void *ev_ctx;		/* unused */
	if (ibv_get_cq_event(conn->channel, &ev_cq, &ev_ctx))
		return RPMA_E_NO_COMPLETION;

	/*
	 * ACK the collected CQ event.
	 *
	 * XXX for performance reasons, it may be beneficial to ACK more than
	 * one CQ event at the same time.
	 */
	ibv_ack_cq_events(conn->cq, 1 /* # of CQ events */);

	/* request for the next event on the CQ channel */
	Rpma_provider_error = ibv_req_notify_cq(conn->cq,
			0 /* all completions */);
	if (Rpma_provider_error) {
		RPMA_LOG_ERROR_WITH_ERRNO(Rpma_provider_error,
				"ibv_req_notify_cq()");
		return RPMA_E_PROVIDER;
	}

	return 0;
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
		/*
		 * There may be an extra CQ event with no completion in the CQ.
		 */
		RPMA_LOG_DEBUG("No completion in the CQ");
		return RPMA_E_NO_COMPLETION;
	} else if (result < 0) {
		/* XXX ibv_poll_cq() may return only -1; no errno provided */
		Rpma_provider_error = result;
		RPMA_LOG_ERROR_WITH_ERRNO(Rpma_provider_error,
				"ibv_poll_cq()");
		return RPMA_E_PROVIDER;
	} else if (result > 1) {
		RPMA_LOG_ERROR(
				"ibv_poll_cq() returned %d where 0 or 1 is expected",
				result);
		return RPMA_E_UNKNOWN;
	}

	switch (wc.opcode) {
	case IBV_WC_RDMA_READ:
		cmpl->op = RPMA_OP_READ;
		break;
	case IBV_WC_RDMA_WRITE:
		cmpl->op = RPMA_OP_WRITE;
		break;
	case IBV_WC_SEND:
		cmpl->op = RPMA_OP_SEND;
		break;
	case IBV_WC_RECV:
		cmpl->op = RPMA_OP_RECV;
		break;
	default:
		RPMA_LOG_ERROR("unsupported wc.opcode == %d", wc.opcode);
		return RPMA_E_NOSUPP;
	}

	cmpl->op_context = (void *)wc.wr_id;
	cmpl->byte_len = wc.byte_len;
	cmpl->op_status = wc.status;

	if (unlikely(wc.status != IBV_WC_SUCCESS)) {
		RPMA_LOG_WARNING("failed rpma_completion(op_context=0x%" PRIx64
				", op_status=%s)",
				cmpl->op_context,
				ibv_wc_status_str(cmpl->op_status));
	}

	return 0;
}
