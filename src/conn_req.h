/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2022, Intel Corporation */

/*
 * conn_req.h -- librpma connection-request-related internal definitions
 */

#ifndef LIBRPMA_CONN_REQ_H
#define LIBRPMA_CONN_REQ_H

#include "librpma.h"

#include <rdma/rdma_cma.h>

/*
 * ERRORS
 * rpma_conn_req_new_from_cm_event() can fail with the following errors:
 *
 * - RPMA_E_INVAL - peer, event or req_ptr is NULL
 * - RPMA_E_INVAL - event is not RDMA_CM_EVENT_CONNECT_REQUEST
 * - RPMA_E_PROVIDER - ibv_create_cq(3) or rdma_create_qp(3) failed
 * - RPMA_E_NOMEM - out of memory
 */
int rpma_conn_req_new_from_cm_event(struct rpma_peer *peer, struct rdma_cm_event *event,
		const struct rpma_conn_cfg *cfg, struct rpma_conn_req **req_ptr);

#endif /* LIBRPMA_CONN_REQ_H */
