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
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_conn_new(struct rdma_cm_id *id, struct ibv_cq *cq,
		struct rpma_conn **conn_ptr);

#endif /* LIBRPMA_CONN_H */
