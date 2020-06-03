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
#include "out.h"

/*
 * rpma_utils_get_ibv_context -- obtain an RDMA device context by IP address
 */
int
rpma_utils_get_ibv_context(const char *addr, struct ibv_context **dev)
{
	struct rdma_addrinfo *rai;
	struct rdma_addrinfo hints;

	if (addr == NULL || dev == NULL)
		return RPMA_E_INVAL;

	/* prepare hints for rdma_getaddrinfo() */
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags |= RAI_PASSIVE; /* XXX passive for now */
	hints.ai_qp_type = IBV_QPT_RC;
	hints.ai_port_space = RDMA_PS_TCP;

	int ret = rdma_getaddrinfo(addr, NULL, &hints, &rai);
	if (ret) {
		Rpma_provider_error = errno;
		return RPMA_E_PROVIDER;
	}

	struct rdma_cm_id *temp_id;
	ret = rdma_create_id(NULL, &temp_id, NULL, RDMA_PS_TCP);
	if (ret) {
		Rpma_provider_error = errno;
		ret = RPMA_E_PROVIDER;
		goto err_create_id;
	}

	ASSERTeq(rai->ai_flags & RAI_PASSIVE, RAI_PASSIVE);

	/* bind the address */
	ret = rdma_bind_addr(temp_id, rai->ai_src_addr);
	if (ret) {
		Rpma_provider_error = errno;
		ret = RPMA_E_PROVIDER;
		goto err_bind_addr;
	}

	/* obtain the device */
	*dev = temp_id->verbs;

err_bind_addr:
	(void) rdma_destroy_id(temp_id);

err_create_id:
	rdma_freeaddrinfo(rai);
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
 * rpma_conn_next_event -- XXX
 */
int
rpma_conn_next_event(struct rpma_conn *conn, enum rpma_conn_event *event)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_get_mr -- XXX
 */
int
rpma_conn_get_mr(struct rpma_conn *conn, struct rpma_mr_remote **mr)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_disconnect -- XXX
 */
int
rpma_conn_disconnect(struct rpma_conn *conn)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_delete -- XXX
 */
int
rpma_conn_delete(struct rpma_conn **conn)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_req_new -- XXX
 */
int
rpma_conn_req_new(struct rpma_peer *peer, const char *addr, const char *service,
		struct rpma_conn_req **req)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_req_delete -- XXX
 */
int
rpma_conn_req_delete(struct rpma_conn_req **req)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_conn_req_connect -- XXX
 */
int
rpma_conn_req_connect(struct rpma_conn_req *req, struct rpma_conn_cfg *ccfg,
	struct rpma_mr_local *mr, struct rpma_conn **conn)
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
