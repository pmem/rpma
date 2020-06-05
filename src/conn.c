/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn.c -- librpma connection-related implementations
 */

#include "conn.h"

struct rpma_conn {
	struct rdma_cm_id *id; /* a CM ID of the connection */
	struct rdma_event_channel *evch; /* event channel of the CM ID */
	struct ibv_cq *cq; /* completion queue of the CM ID */
};

/* internal librpma API */

/*
 * rpma_conn_new -- XXX wrap entities into a new rpma_conn object
 */
int
rpma_conn_new(struct rdma_cm_id *id, struct rdma_event_channel *evch,
		struct ibv_cq *cq, struct rpma_conn **conn)
{
	return RPMA_E_NOSUPP;
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
 * rpma_conn_delete -- XXX deleted rpma_conn object; make sure the connection
 * is after RPMA_CONN_CLOSED event
 */
int
rpma_conn_delete(struct rpma_conn **conn)
{
	return RPMA_E_NOSUPP;
}
