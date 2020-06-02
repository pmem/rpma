/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * ep.c -- librpma endpoint-related implementations
 */

#include "librpma.h"

struct rpma_ep {
	/* CM ID dedicated to listening for incoming connections */
	struct rdma_cm_id *id;
	/* event channel of the CM ID */
	struct rdma_event_channel *evch;
};

/* public librpma API */

/*
 * rpma_ep_listen -- XXX uses rdma_create_id, rpma_info_bind_addr,
 * rdma_create_event_channel, rdma_migrate_id and rdma_listen
 */
int
rpma_ep_listen(struct rpma_peer *peer, const char *addr, const char *service,
		struct rpma_ep **ep)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_ep_shutdown -- XXX
 */
int
rpma_ep_shutdown(struct rpma_ep **ep)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_ep_next_conn_req -- XXX uses rdma_get_cm_event and
 * rpma_conn_req_from_cm_event
 */
int
rpma_ep_next_conn_req(struct rpma_ep *ep, struct rpma_conn_req **req)
{
	return RPMA_E_NOSUPP;
}
