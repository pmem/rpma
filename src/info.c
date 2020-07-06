/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * info.c -- librpma info-related implementations
 */

#include <errno.h>

#include "cmocka_alloc.h"
#include "conn_req.h"
#include "info.h"
#include "rpma_err.h"

#include "librpma.h"

struct rpma_info {
	/* either active or passive side of the connection */
	enum rpma_info_side side;
	/* a cache of the translated address */
	struct rdma_addrinfo *rai;
};

/* internal librpma API */

/*
 * rpma_info_new -- create an address translation cache aka the info object
 */
int
rpma_info_new(const char *addr, const char *service, enum rpma_info_side side,
		struct rpma_info **info_ptr)
{
	if (addr == NULL || info_ptr == NULL)
		return RPMA_E_INVAL;

	/* prepare hints */
	struct rdma_addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	if (side == RPMA_INFO_PASSIVE)
		hints.ai_flags |= RAI_PASSIVE;
	hints.ai_qp_type = IBV_QPT_RC;
	hints.ai_port_space = RDMA_PS_TCP;

	/* query */
	struct rdma_addrinfo *rai = NULL;
	int ret = rdma_getaddrinfo(addr, service, &hints, &rai);
	if (ret) {
		Rpma_provider_error = errno;
		return RPMA_E_PROVIDER;
	}

	//sanity check for compatibility between rai and hints ai_flags
	if (hints.ai_flags & RAI_PASSIVE) {
		if ( !(rai->ai_flags & RAI_PASSIVE)) {
			ret = RPMA_E_UNKNOWN;
			goto err_freeaddrinfo;
		}

	}

	struct rpma_info *info = Malloc(sizeof(*info));
	if (info == NULL) {
		ret = RPMA_E_NOMEM;
		goto err_freeaddrinfo;
	}

	info->side = side;
	info->rai = rai;
	*info_ptr = info;

	return 0;

err_freeaddrinfo:
	rdma_freeaddrinfo(rai);
	return ret;
}

/*
 * rpma_info_delete -- release the address translation cache and delete the
 * info object
 */
int
rpma_info_delete(struct rpma_info **info_ptr)
{
	if (info_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_info *info = *info_ptr;
	if (info == NULL)
		return 0;

	rdma_freeaddrinfo(info->rai);
	Free(info);
	*info_ptr = NULL;

	return 0;
}

/*
 * rpma_info_assign_addr -- assign address to the ID
 * either  resolve the ID destination address
 * or bind the ID to address
 */
int
rpma_info_assign_addr(const struct rpma_info *info, struct rdma_cm_id *id)
{
	int ret = 0;
	if (id == NULL || info == NULL)
		return RPMA_E_INVAL;

	if (info->side == RPMA_INFO_ACTIVE)
		ret = rdma_resolve_addr(id, info->rai->ai_src_addr,
				info->rai->ai_dst_addr, RPMA_DEFAULT_TIMEOUT);
	else
		ret = rdma_bind_addr(id, info->rai->ai_src_addr);

	if (ret) {
		Rpma_provider_error = errno;
		return RPMA_E_PROVIDER;
	}
	return 0;
}
