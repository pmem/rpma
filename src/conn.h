/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn.h -- librpma connection-related internal definitions
 */

#ifndef LIBRPMA_CONN_H
#define LIBRPMA_CONN_H

#include <rdma/rdma_cma.h>

int rpma_conn_new(struct rdma_cm_id *id, struct rdma_event_channel *evch,
		struct ibv_cq *cq, struct rpma_conn **conn);

#endif /* LIBRPMA_CONN_H */
