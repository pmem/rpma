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

int rpma_conn_req_from_cm_event(struct rpma_peer *peer,
		struct rdma_cm_event *event, struct rpma_conn_req **req);

#endif /* LIBRPMA_CONN_REQ_H */
