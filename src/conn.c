// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */
/* Copyright 2021, Fujitsu */

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

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

struct rpma_conn {
	struct rdma_cm_id *id; /* a CM ID of the connection */
	struct rdma_event_channel *evch; /* event channel of the CM ID */
	struct rpma_cq *cq; /* main CQ */
	struct rpma_cq *rcq; /* receive CQ */

	struct rpma_conn_private_data data; /* private data of the CM ID */
	struct rpma_flush *flush; /* flushing object */

	bool direct_write_to_pmem; /* direct write to pmem is supported */
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
		struct rpma_cq *cq, struct rpma_cq *rcq,
		struct rpma_conn **conn_ptr)
{
	if (peer == NULL || id == NULL || cq == NULL || conn_ptr == NULL)
		return RPMA_E_INVAL;

	int ret = 0;

	struct rdma_event_channel *evch = rdma_create_event_channel();
	if (!evch) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_create_event_channel()");
		return RPMA_E_PROVIDER;
	}

	if (rdma_migrate_id(id, evch)) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_migrate_id()");
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
	conn->cq = cq;
	conn->rcq = rcq;
	conn->data.ptr = NULL;
	conn->data.len = 0;
	conn->flush = flush;
	conn->direct_write_to_pmem = false;

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
 * rpma_conn_transfer_private_data -- transfer the private data to
 * the connection (a take over).
 */
void
rpma_conn_transfer_private_data(struct rpma_conn *conn,
		struct rpma_conn_private_data *pdata)
{
	conn->data.ptr = pdata->ptr;
	conn->data.len = pdata->len;

	pdata->ptr = NULL;
	pdata->len = 0;
}

/* public librpma API */

/*
 * rpma_conn_get_event_fd -- get a file descriptor of the event channel
 * associated with the connection
 */
int
rpma_conn_get_event_fd(const struct rpma_conn *conn, int *fd)
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
			return RPMA_E_NO_EVENT;

		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_get_cm_event()");
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
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_ack_cm_event()");
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
		case RDMA_CM_EVENT_REJECTED:
			*event = RPMA_CONN_REJECTED;
			break;
		default:
			RPMA_LOG_WARNING("%s: %s",
					rpma_utils_conn_event_2str(*event),
					rdma_event_str(cm_event));
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
rpma_conn_get_private_data(const struct rpma_conn *conn,
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
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_disconnect()");
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
		goto err_destroy_qp;

	rdma_destroy_qp(conn->id);

	ret = rpma_cq_delete(&conn->rcq);
	if (ret)
		goto err_rpma_cq_delete;

	ret = rpma_cq_delete(&conn->cq);
	if (ret)
		goto err_destroy_id;

	if (rdma_destroy_id(conn->id)) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_destroy_id()");
		ret = RPMA_E_PROVIDER;
		goto err_destroy_event_channel;
	}

	rdma_destroy_event_channel(conn->evch);
	rpma_private_data_discard(&conn->data);

	free(conn);
	*conn_ptr = NULL;

	return 0;

err_destroy_qp:
	rdma_destroy_qp(conn->id);
	(void) rpma_cq_delete(&conn->rcq);
err_rpma_cq_delete:
	(void) rpma_cq_delete(&conn->cq);
err_destroy_id:
	(void) rdma_destroy_id(conn->id);
err_destroy_event_channel:
	rdma_destroy_event_channel(conn->evch);

	free(conn);
	*conn_ptr = NULL;

	return ret;
}

/*
 * rpma_read -- initiate the read operation
 */
int
rpma_read(struct rpma_conn *conn,
	struct rpma_mr_local *dst, size_t dst_offset,
	const struct rpma_mr_remote *src,  size_t src_offset,
	size_t len, int flags, const void *op_context)
{
	if (conn == NULL || flags == 0 ||
	    ((src == NULL || dst == NULL) &&
	    (src != NULL || dst != NULL || dst_offset != 0 || src_offset != 0 ||
	    len != 0)))
		return RPMA_E_INVAL;

	return rpma_mr_read(conn->id->qp,
			dst, dst_offset,
			src, src_offset,
			len, flags, op_context);
}

/*
 * rpma_write -- initiate the write operation
 */
int
rpma_write(struct rpma_conn *conn,
	struct rpma_mr_remote *dst, size_t dst_offset,
	const struct rpma_mr_local *src,  size_t src_offset,
	size_t len, int flags, const void *op_context)
{
	if (conn == NULL || flags == 0 ||
	    ((src == NULL || dst == NULL) &&
	    (src != NULL || dst != NULL || dst_offset != 0 || src_offset != 0 ||
	    len != 0)))
		return RPMA_E_INVAL;

	return rpma_mr_write(conn->id->qp,
			dst, dst_offset,
			src, src_offset,
			len, flags,
			IBV_WR_RDMA_WRITE, 0,
			op_context, false);
}

/*
 * rpma_write_with_imm -- initiate the write operation with immediate data
 */
int
rpma_write_with_imm(struct rpma_conn *conn,
	struct rpma_mr_remote *dst, size_t dst_offset,
	const struct rpma_mr_local *src,  size_t src_offset,
	size_t len, int flags, uint32_t imm, const void *op_context)
{
	if (conn == NULL || flags == 0 ||
	    ((src == NULL || dst == NULL) &&
	    (src != NULL || dst != NULL || dst_offset != 0 || src_offset != 0 ||
	    len != 0)))
		return RPMA_E_INVAL;

	return rpma_mr_write(conn->id->qp,
			dst, dst_offset,
			src, src_offset,
			len, flags,
			IBV_WR_RDMA_WRITE_WITH_IMM, imm,
			op_context, false);
}

/*
 * rpma_write_atomic -- initiate the atomic write operation
 */
int
rpma_write_atomic(struct rpma_conn *conn,
	struct rpma_mr_remote *dst, size_t dst_offset,
	const struct rpma_mr_local *src,  size_t src_offset,
	int flags, const void *op_context)
{
	if (conn == NULL || dst == NULL || src == NULL || flags == 0)
		return RPMA_E_INVAL;

	if (dst_offset % RPMA_ATOMIC_WRITE_ALIGNMENT != 0)
		return RPMA_E_INVAL;

	return rpma_mr_write(conn->id->qp,
			dst, dst_offset,
			src, src_offset,
			RPMA_ATOMIC_WRITE_ALIGNMENT, flags,
			IBV_WR_RDMA_WRITE, 0,
			op_context, true);
}

/*
 * rpma_atomic_write -- initiate the atomic 8 bytes write operation
 */
int
rpma_atomic_write(struct rpma_conn *conn,
	struct rpma_mr_remote *dst, size_t dst_offset,
	const char src[8],
	int flags, const void *op_context)
{
	if (conn == NULL || dst == NULL || src == NULL || flags == 0)
		return RPMA_E_INVAL;

	if (dst_offset % RPMA_ATOMIC_WRITE_ALIGNMENT != 0)
		return RPMA_E_INVAL;

	return rpma_mr_atomic_write(conn->id->qp,
			dst, dst_offset, src,
			flags, op_context);
}

/*
 * rpma_flush -- initiate the flush operation
 */
int
rpma_flush(struct rpma_conn *conn,
	struct rpma_mr_remote *dst, size_t dst_offset, size_t len,
	enum rpma_flush_type type, int flags, const void *op_context)
{
	if (conn == NULL || dst == NULL || flags == 0)
		return RPMA_E_INVAL;

	if (type == RPMA_FLUSH_TYPE_PERSISTENT && !conn->direct_write_to_pmem) {
		RPMA_LOG_ERROR(
			"Connection does not support flush to persistency. "
			"Check if the remote node supports direct write to persistent memory.");
		return RPMA_E_NOSUPP;
	}

	int flush_type;
	/* it cannot fail because: mr != NULL && flush_type != NULL */
	(void) rpma_mr_remote_get_flush_type(dst, &flush_type);

	if (type == RPMA_FLUSH_TYPE_PERSISTENT &&
	    0 == (flush_type & RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT)) {
		RPMA_LOG_ERROR(
			"The remote memory region does not support flushing to persistency");
		return RPMA_E_NOSUPP;
	}

	if (type == RPMA_FLUSH_TYPE_VISIBILITY &&
	    0 == (flush_type & RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY)) {
		RPMA_LOG_ERROR(
			"The remote memory region does not support flushing to global visibility");
		return RPMA_E_NOSUPP;
	}

	rpma_flush_func flush = conn->flush->func;
	return flush(conn->id->qp, conn->flush, dst, dst_offset,
			len, type, flags, op_context);
}

/*
 * rpma_send -- initiate the send operation
 */
int
rpma_send(struct rpma_conn *conn,
    const struct rpma_mr_local *src, size_t offset, size_t len,
    int flags, const void *op_context)
{
	if (conn == NULL || flags == 0 ||
	    (src == NULL && (offset != 0 || len != 0)))
		return RPMA_E_INVAL;

	return rpma_mr_send(conn->id->qp,
			src, offset, len,
			flags, IBV_WR_SEND,
			0, op_context);
}

/*
 * rpma_send_with_imm -- initiate the send operation with immediate data
 */
int
rpma_send_with_imm(struct rpma_conn *conn,
	const struct rpma_mr_local *src, size_t offset, size_t len,
	int flags, uint32_t imm, const void *op_context)
{
	if (conn == NULL || flags == 0 ||
	    (src == NULL && (offset != 0 || len != 0)))
		return RPMA_E_INVAL;

	return rpma_mr_send(conn->id->qp,
			src, offset, len,
			flags, IBV_WR_SEND_WITH_IMM,
			imm, op_context);
}

/*
 * rpma_recv -- initiate the receive operation
 */
int
rpma_recv(struct rpma_conn *conn,
    struct rpma_mr_local *dst, size_t offset, size_t len,
    const void *op_context)
{
	if (conn == NULL || (dst == NULL && (offset != 0 || len != 0)))
		return RPMA_E_INVAL;

	return rpma_mr_recv(conn->id->qp,
			dst, offset, len,
			op_context);
}

/*
 * rpma_conn_get_qp_num -- get the connection's qp_num
 */
int
rpma_conn_get_qp_num(const struct rpma_conn *conn, uint32_t *qp_num)
{
	if (conn == NULL || qp_num == NULL)
		return RPMA_E_INVAL;

	*qp_num = conn->id->qp->qp_num;

	return 0;
}

/*
 * rpma_conn_get_cq -- get the connection's main CQ
 */
int
rpma_conn_get_cq(const struct rpma_conn *conn, struct rpma_cq **cq_ptr)
{
	if (conn == NULL || cq_ptr == NULL)
		return RPMA_E_INVAL;

	*cq_ptr = conn->cq;

	return 0;
}

/*
 * rpma_conn_get_rcq -- get the connection's receive CQ
 */
int
rpma_conn_get_rcq(const struct rpma_conn *conn, struct rpma_cq **rcq_ptr)
{
	if (conn == NULL || rcq_ptr == NULL)
		return RPMA_E_INVAL;

	*rcq_ptr = conn->rcq;

	return 0;
}

/*
 * rpma_conn_apply_remote_peer_cfg -- apply remote peer cfg for the connection
 */
int
rpma_conn_apply_remote_peer_cfg(struct rpma_conn *conn,
		const struct rpma_peer_cfg *pcfg)
{
	if (conn == NULL || pcfg == NULL)
		return RPMA_E_INVAL;

	return rpma_peer_cfg_get_direct_write_to_pmem(pcfg,
			&conn->direct_write_to_pmem);
}
