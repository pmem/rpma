/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn_req.h -- librpma connection-request-related internal definitions
 */

#ifndef LIBRPMA_CONN_REQ_H
#define LIBRPMA_CONN_REQ_H

#include <rdma/rdma_cma.h>

#define RPMA_DEFAULT_TIMEOUT 1000 /* ms */

struct rpma_conn_req;
struct rpma_peer;

int rpma_conn_req_from_cm_event(struct rpma_peer *peer,
		struct rdma_cm_event *event, struct rpma_conn_req **req);

#endif /* LIBRPMA_CONN_REQ_H */
