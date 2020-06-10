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
#include "out.h"

/* public librpma API */

/*
 * rpma_utils_get_ibv_context -- obtain an RDMA device context by IP address
 */
int
rpma_utils_get_ibv_context(const char *addr, struct ibv_context **dev)
{
	struct rpma_info *info;

	if (addr == NULL || dev == NULL)
		return RPMA_E_INVAL;

	/* XXX only passive for now */
	int ret = rpma_info_new(addr, NULL /* service */,
			RPMA_INFO_PASSIVE, &info);
	if (ret)
		return ret;

	struct rdma_cm_id *temp_id;
	ret = rdma_create_id(NULL, &temp_id, NULL, RDMA_PS_TCP);
	if (ret) {
		Rpma_provider_error = errno;
		ret = RPMA_E_PROVIDER;
		goto err_info_delete;
	}

	/* bind the address */
	ret = rpma_info_bind_addr(info, temp_id);
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
 * rpma_mr_reg -- XXX
 */
int
rpma_mr_reg(struct rpma_peer *peer, void *ptr, size_t size, int usage, int plt,
		struct rpma_mr_local **mr)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_mr_dereg -- XXX
 */
int
rpma_mr_dereg(struct rpma_mr_local **mr)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_read -- XXX
 */
int
rpma_read(struct rpma_conn *conn, void *op_context,
	struct rpma_mr_local *dst, size_t dst_offset,
	struct rpma_mr_remote *src,  size_t src_offset,
	size_t len, int flags)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_next_completion -- XXX
 */
int
rpma_conn_next_completion(struct rpma_conn *conn, struct rpma_completion *cmpl)
{
	return RPMA_E_NOSUPP;
}
