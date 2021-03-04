/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2021, Intel Corporation */

/*
 * conn.h -- librpma connection-related internal definitions
 */

#ifndef LIBRPMA_CONN_H
#define LIBRPMA_CONN_H

#include "librpma.h"

#include <rdma/rdma_cma.h>

struct rpma_conn {
	struct rdma_cm_id *id; /* a CM ID of the connection */
	struct rdma_event_channel *evch; /* event channel of the CM ID */
	struct ibv_comp_channel *channel; /* completion event channel */
	struct ibv_cq *cq; /* completion queue of the CM ID */

	struct rpma_conn_private_data data; /* private data of the CM ID */
	struct rpma_flush *flush; /* flushing object */

	bool direct_write_to_pmem; /* direct write to pmem is supported */
};

/*
 * ERRORS
 * rpma_conn_new() can fail with the following errors:
 *
 * - RPMA_E_INVAL - peer, id, cq or conn_ptr is NULL
 * - RPMA_E_PROVIDER - if rdma_create_event_channel(3) or rdma_migrate_id(3)
 *                     fail
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_conn_new(struct rpma_peer *peer, struct rdma_cm_id *id,
		struct ibv_cq *cq, struct rpma_conn **conn_ptr);

/*
 * rpma_conn_transfer_private_data -- transfer the private data to
 * the connection (a take over).
 *
 * ASSUMPTIONS
 * - conn != NULL && pdata != NULL
 */
void rpma_conn_transfer_private_data(struct rpma_conn *conn,
		struct rpma_conn_private_data *pdata);

#endif /* LIBRPMA_CONN_H */
