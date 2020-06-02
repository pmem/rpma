/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * info.c -- librpma info-related implementations
 */

#include "librpma.h"
#include "info.h"

struct rpma_info {
	/* either active or passive side of the connection */
	enum rpma_info_side side;
	struct rdma_addrinfo *rai; /* a cache of the translated address */
};

/* internal librpma API */

/*
 * rpma_info_new -- XXX use addr, service and side for resolving the rpma_info
 * instance; uses rdma_getaddrinfo
 */
int
rpma_info_new(const char *addr, const char *service, enum rpma_info_side side,
		struct rpma_info **info)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_info_delete -- XXX relase cached info and delete the rpma_info object;
 * uses rdma_freeaddrinfo
 */
int
rpma_info_delete(struct rpma_info **info)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_info_resolve_addr -- XXX uses dma_resolve_addr
 */
int
rpma_info_resolve_addr(struct rpma_info *info, struct rdma_cm_id *id)
{
	return RPMA_E_NOSUPP;
}

/*
 * rdma_info_bind_addr -- XXX uses rdma_bind_addr
 */
int
rdma_info_bind_addr(struct rpma_info *info)
{
	return RPMA_E_NOSUPP;
}
