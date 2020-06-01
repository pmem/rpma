/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * ep.c -- librpma endpoint-related implementations
 */

#include "librpma.h"

struct rpma_ep {
	/* entities dedicated to listening for incoming connections */
	struct rdma_cm_id *id;
	struct rdma_event_channel *evch;
};

/*
 * rpma_ep_listen -- XXX
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
 * rpma_ep_next_conn_req -- XXX
 */
int
rpma_ep_next_conn_req(struct rpma_ep *ep, struct rpma_conn_req **req)
{
	return RPMA_E_NOSUPP;
}
