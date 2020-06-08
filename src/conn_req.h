/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn_req.h -- librpma connection-request-related internal definitions
 */

#ifndef LIBRPMA_CONN_REQ_H
#define LIBRPMA_CONN_REQ_H

#include "librpma.h"

#include <rdma/rdma_cma.h>

#define RPMA_DEFAULT_TIMEOUT 1000 /* ms */

/* for the simplicity sake, it is assumed all CQ/SQ/RQ sizes are equal */
#define RPMA_DEFAULT_Q_SIZE 10

/* the maximum number of scatter/gather elements in any Work Request */
#define RPMA_MAX_SGE 1

/* the maximum message size (in bytes) that can be posted inline */
#define RPMA_MAX_INLINE_DATA 0

/*
 * ERRORS
 * rpma_conn_req_from_cm_event() can fail with the following errors:
 *
 * - RPMA_E_INVAL - peer, event or req_ptr is NULL
 * - RPMA_E_INVAL - event is not RDMA_CM_EVENT_CONNECT_REQUEST
 * - RPMA_E_PROVIDER - ibv_create_cq(3) or rdma_create_qp(3) failed
 */
int rpma_conn_req_from_cm_event(struct rpma_peer *peer,
		struct rdma_cm_event *event, struct rpma_conn_req **req_ptr);

#endif /* LIBRPMA_CONN_REQ_H */
