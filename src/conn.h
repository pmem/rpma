/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2022, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn.h -- librpma connection-related internal definitions
 */

#ifndef LIBRPMA_CONN_H
#define LIBRPMA_CONN_H

#include "librpma.h"
#include "cq.h"

#include <rdma/rdma_cma.h>

/*
 * ERRORS
 * rpma_conn_new() can fail with the following errors:
 *
 * - RPMA_E_INVAL - peer, id, cq or conn_ptr is NULL
 * - RPMA_E_PROVIDER - if rdma_create_event_channel(3) or rdma_migrate_id(3) fail
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_conn_new(struct rpma_peer *peer, struct rdma_cm_id *id, struct rpma_cq *cq,
		struct rpma_cq *rcq, struct ibv_comp_channel *channel,
		struct rpma_conn **conn_ptr);

/*
 * rpma_conn_transfer_private_data -- transfer the private data to the connection (a take over).
 *
 * ASSUMPTIONS
 * - conn != NULL && pdata != NULL
 */
void rpma_conn_transfer_private_data(struct rpma_conn *conn, struct rpma_conn_private_data *pdata);

#endif /* LIBRPMA_CONN_H */
