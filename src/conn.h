/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn.h -- librpma connection-related internal definitions
 */

#ifndef LIBRPMA_CONN_H
#define LIBRPMA_CONN_H

#include "librpma.h"

#include <rdma/rdma_cma.h>

/*
 * ERRORS
 * rpma_conn_new() can fail with the following errors:
 *
 * - RPMA_E_INVAL - id, cq or conn_ptr is NULL
 * - RPMA_E_PROVIDER - if rdma_create_event_channel(3) or rdma_migrate_id(3)
 *                     fail
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_conn_new(struct rpma_peer *peer, struct rdma_cm_id *id,
		struct ibv_cq *cq, struct rpma_conn **conn_ptr);

/*
 * rpma_conn_set_private_data -- allocate a buffer and fill
 * the private data of the CM ID
 *
 * ASSUMPTIONS
 * - conn != NULL && pdata != NULL
 *
 * ERRORS
 * rpma_conn_set_private_data() can fail with the following error:
 *
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_conn_set_private_data(struct rpma_conn *conn,
		struct rpma_conn_private_data *pdata);

#endif /* LIBRPMA_CONN_H */
