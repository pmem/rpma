/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn.c -- librpma connection-related implementations
 */

#include "cmocka_alloc.h"
#include "conn.h"
#include "rpma_err.h"
#include "out.h"

struct rpma_conn {
	struct rdma_cm_id *id; /* a CM ID of the connection */
	struct rdma_event_channel *evch; /* event channel of the CM ID */
	struct ibv_cq *cq; /* completion queue of the CM ID */
};

/* internal librpma API */

/*
 * rpma_conn_new -- wrap provided entities into a newly created connection
 * object
 */
int
rpma_conn_new(struct rdma_cm_id *id, struct rdma_event_channel *evch,
		struct ibv_cq *cq, struct rpma_conn **conn_ptr)
{
	if (id == NULL || evch == NULL || cq == NULL || conn_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_conn *conn = Malloc(sizeof(*conn));
	if (!conn) {
		ASSERTeq(errno, ENOMEM);
		return RPMA_E_NOMEM;
	}

	conn->id = id;
	conn->evch = evch;
	conn->cq = cq;
	*conn_ptr = conn;

	return 0;
}

/* public librpma API */

/*
 * rpma_conn_next_event -- XXX uses rdma_get_cm_event
 */
int
rpma_conn_next_event(struct rpma_conn *conn, enum rpma_conn_event *event)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_get_private_data -- XXX
 */
int
rpma_conn_get_private_data(struct rpma_conn *conn, void **private_data,
		uint8_t *private_data_len)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_disconnect -- XXX uses rdma_disconnect
 */
int
rpma_conn_disconnect(struct rpma_conn *conn)
{
	return RPMA_E_NOSUPP;
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

	Free(conn);
	*conn_ptr = NULL;

	return 0;

err_destroy_id:
	(void) rdma_destroy_id(conn->id);
err_destroy_event_channel:
	rdma_destroy_event_channel(conn->evch);

	Free(conn);
	*conn_ptr = NULL;

	return ret;
}
