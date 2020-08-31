// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * rpma.c -- entry points for librpma
 */

#include <rdma/rdma_cma.h>

#include "librpma.h"
#include "log_internal.h"
#include "rpma_err.h"
#include "info.h"

/* public librpma API */

/*
 * rpma_utils_get_ibv_context -- obtain an RDMA device context by IP address
 */
int
rpma_utils_get_ibv_context(const char *addr,
		enum rpma_util_ibv_context_type type, struct ibv_context **dev)
{
	if (addr == NULL || dev == NULL)
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

	struct rdma_cm_id *temp_id;
	int ret = rdma_create_id(NULL, &temp_id, NULL, RDMA_PS_TCP);
	if (ret) {
		Rpma_provider_error = errno;
		RPMA_LOG_ERROR_WITH_ERRNO(Rpma_provider_error,
				"rdma_create_id");
		return RPMA_E_PROVIDER;
	}

	struct rpma_info *info;
	ret = rpma_info_new(addr, NULL /* port */, side, &info);
	if (ret)
		goto err_destroy_id;

	if (side == RPMA_INFO_PASSIVE) {
		ret = rpma_info_bind_addr(info, temp_id);
		if (ret)
			goto err_info_delete;
	} else {
		ret = rpma_info_resolve_addr(info, temp_id,
				RPMA_DEFAULT_TIMEOUT_MS);
		if (ret)
			goto err_info_delete;
	}

	/* obtain the device */
	*dev = temp_id->verbs;

err_info_delete:
	(void) rpma_info_delete(&info);

err_destroy_id:
	(void) rdma_destroy_id(temp_id);

	return ret;
}

/*
 * rpma_utils_ibv_context_is_odp_capable -- query the extended device context's
 * capabilities and check if it supports On-Demand Paging
 */
int
rpma_utils_ibv_context_is_odp_capable(struct ibv_context *dev,
		int *is_odp_capable)
{
	if (dev == NULL || is_odp_capable == NULL)
		return RPMA_E_INVAL;

	*is_odp_capable = 0;

	/* query an RDMA device's attributes */
	struct ibv_device_attr_ex attr = {{{0}}};
	Rpma_provider_error = ibv_query_device_ex(dev, NULL /* input */,
			&attr);
	if (Rpma_provider_error) {
		RPMA_LOG_ERROR_WITH_ERRNO(Rpma_provider_error,
			"ibv_query_device_ex(attr={0})");
		return RPMA_E_PROVIDER;
	}

	/*
	 * Check whether On-Demand Paging is supported for all required types
	 * of operations.
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

	return 0;
}

/*
 * rpma_utils_conn_event_2str -- return const string representation of
 * RPMA_CONN_* enums
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
	default:
		return "Unknown connection event";
	}
}
