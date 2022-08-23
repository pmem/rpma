// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * utils.c -- generic helper functions for librpma
 */

#include <rdma/rdma_cma.h>

#include "librpma.h"
#include "debug.h"
#include "log_internal.h"
#include "info.h"

/* public librpma API */

/*
 * rpma_utils_get_ibv_context -- obtain an RDMA device context by IP address
 */
int
rpma_utils_get_ibv_context(const char *addr, enum rpma_util_ibv_context_type type,
		struct ibv_context **ibv_ctx_ptr)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_INVAL, {});

	if (addr == NULL || ibv_ctx_ptr == NULL)
		return RPMA_E_INVAL;

	enum rpma_info_side side;
	switch (type) {
	case RPMA_UTIL_IBV_CONTEXT_LOCAL:
		side = RPMA_INFO_PASSIVE;
		break;
	case RPMA_UTIL_IBV_CONTEXT_REMOTE:
		side = RPMA_INFO_ACTIVE;
		break;
	default:
		return RPMA_E_INVAL;
	}

	struct rpma_info *info;
	int ret = rpma_info_new(addr, NULL /* port */, side, &info);
	if (ret)
		return ret;

	struct rdma_cm_id *temp_id;
	RPMA_FAULT_INJECTION_GOTO(RPMA_E_PROVIDER, err_info_delete);
	ret = rdma_create_id(NULL, &temp_id, NULL, RDMA_PS_TCP);
	if (ret) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "rdma_create_id()");
		ret = RPMA_E_PROVIDER;
		goto err_info_delete;
	}

	if (side == RPMA_INFO_PASSIVE) {
		ret = rpma_info_bind_addr(info, temp_id);
		if (ret)
			goto err_destroy_id;
	} else {
		ret = rpma_info_resolve_addr(info, temp_id,
				RPMA_DEFAULT_TIMEOUT_MS);
		if (ret)
			goto err_destroy_id;
	}

	/* obtain the device */
	*ibv_ctx_ptr = temp_id->verbs;

err_destroy_id:
	(void) rdma_destroy_id(temp_id);

err_info_delete:
	(void) rpma_info_delete(&info);
	return ret;
}

/*
 * rpma_utils_ibv_context_is_odp_capable -- query the extended device context's capabilities and
 * check if it supports On-Demand Paging
 */
int
rpma_utils_ibv_context_is_odp_capable(struct ibv_context *ibv_ctx, int *is_odp_capable)
{
	RPMA_DEBUG_TRACE;
	RPMA_FAULT_INJECTION(RPMA_E_PROVIDER, {});

	if (ibv_ctx == NULL || is_odp_capable == NULL)
		return RPMA_E_INVAL;

	*is_odp_capable = 0;

#ifdef ON_DEMAND_PAGING_SUPPORTED
	/* query an RDMA device's attributes */
	struct ibv_device_attr_ex attr = {{{0}}};
	errno = ibv_query_device_ex(ibv_ctx, NULL /* input */, &attr);
	if (errno) {
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "ibv_query_device_ex(attr={0})");
		return RPMA_E_PROVIDER;
	}

	/*
	 * Check whether On-Demand Paging is supported for all required types of operations.
	 */
	struct ibv_odp_caps *odp_caps = &attr.odp_caps;
	if (odp_caps->general_caps & IBV_ODP_SUPPORT) {
		/* flags for the Reliable Connected transport type */
		uint32_t rc_odp_caps = odp_caps->per_transport_caps.rc_odp_caps;
		if ((rc_odp_caps & IBV_ODP_SUPPORT_WRITE) &&
				(rc_odp_caps & IBV_ODP_SUPPORT_READ)) {
			*is_odp_capable = 1;
		}
	}
#endif
	return 0;
}

/*
 * rpma_utils_conn_event_2str -- return const string representation of RPMA_CONN_* enums
 */
const char *
rpma_utils_conn_event_2str(enum rpma_conn_event conn_event)
{
	switch (conn_event) {
	case RPMA_CONN_UNDEFINED:
		return "Undefined connection event";
	case RPMA_CONN_ESTABLISHED:
		return "Connection established";
	case RPMA_CONN_CLOSED:
		return "Connection closed";
	case RPMA_CONN_LOST:
		return "Connection lost";
	case RPMA_CONN_REJECTED:
		return "Connection rejected";
	case RPMA_CONN_UNREACHABLE:
		return "Connection unreachable";
	default:
		return "Unsupported connection event";
	}
}
