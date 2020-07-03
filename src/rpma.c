/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * rpma.c -- entry points for librpma
 */

#include <rdma/rdma_cma.h>

#include "librpma.h"
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

	struct rpma_info *info;
	int ret = rpma_info_new(addr, NULL /* service */, side, &info);
	if (ret)
		return ret;

	struct rdma_cm_id *temp_id;
	ret = rdma_create_id(NULL, &temp_id, NULL, RDMA_PS_TCP);
	if (ret) {
		Rpma_provider_error = errno;
		ret = RPMA_E_PROVIDER;
		goto err_info_delete;
	}

	ret = rpma_info_assign_addr(info, temp_id);
	if (ret)
		goto err_destroy_id;

	/* obtain the device */
	*dev = temp_id->verbs;

err_destroy_id:
	(void) rdma_destroy_id(temp_id);

err_info_delete:
	(void) rpma_info_delete(&info);
	return ret;
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
