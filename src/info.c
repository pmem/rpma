// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * info.c -- librpma info-related implementations
 */

#include <errno.h>
#include <stdlib.h>
#include <netdb.h>

#include "conn_req.h"
#include "debug.h"
#include "info.h"
#include "log_internal.h"

#include "librpma.h"

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

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
rpma_info_new(const char *addr, const char *port, enum rpma_info_side side,
		struct rpma_info **info_ptr)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_PROVIDER, {});

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
#ifdef RDMA_GETADDRINFO_OLD_SIGNATURE
	int ret = rdma_getaddrinfo((char *)addr, (char *)port, &hints, &rai);
#else
	int ret = rdma_getaddrinfo(addr, port, &hints, &rai);
#endif
	if (ret) {
		const char *err = (ret == -1 || ret == EAI_SYSTEM) ?
				strerror(errno) : gai_strerror(ret);
		RPMA_LOG_ERROR(
			"rdma_getaddrinfo(node=%s, service=%s, ai_flags=%s, ai_qp_type=IBV_QPT_RC, ai_port_space=RDMA_PS_TCP): %s",
			addr, port, (hints.ai_flags & RAI_PASSIVE) ? "passive" : "active", err);
		return RPMA_E_PROVIDER;
	}

	struct rpma_info *info = malloc(sizeof(*info));
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
 * rpma_info_delete -- release the address translation cache and delete the info object
 */
int
rpma_info_delete(struct rpma_info **info_ptr)
{
	RPMA_DEBUG_TRACE;

	if (info_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_info *info = *info_ptr;
	if (info == NULL)
		return 0;

	rdma_freeaddrinfo(info->rai);
	free(info);
	*info_ptr = NULL;

	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});
	return 0;
}

/*
 * rpma_info_resolve_addr -- resolve the CM ID's destination address
 *
 * ASSUMPTIONS
 * - info != NULL
 * - id != NULL
 * - timeout_ms > 0
 */
int
rpma_info_resolve_addr(const struct rpma_info *info, struct rdma_cm_id *id, int timeout_ms)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_PROVIDER, {});

	int ret = rdma_resolve_addr(id, info->rai->ai_src_addr, info->rai->ai_dst_addr,
			timeout_ms);
	if (ret) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno,
			"rdma_resolve_addr(src_addr=%s, dst_addr=%s, timeout_ms=%d)",
			info->rai->ai_src_canonname,
			info->rai->ai_dst_canonname,
			timeout_ms);
		return RPMA_E_PROVIDER;
	}

	return 0;
}

/*
 * rpma_info_bind_addr -- bind the CM ID to the local address
 */
int
rpma_info_bind_addr(const struct rpma_info *info, struct rdma_cm_id *id)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_PROVIDER, {});

	if (id == NULL || info == NULL)
		return RPMA_E_INVAL;

	int ret = rdma_bind_addr(id, info->rai->ai_src_addr);
	if (ret) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_bind_addr(addr=%s)",
				info->rai->ai_src_canonname);
		return RPMA_E_PROVIDER;
	}

	return 0;
}
