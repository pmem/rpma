/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * rpma.c -- entry points for librpma
 */

#include "librpma.h"

/*
 * rpma_utils_get_ibv_context -- XXX
 */
int
rpma_utils_get_ibv_context(const char *addr, struct ibv_context **dev)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_peer_new -- XXX
 */
int
rpma_peer_new(struct rpma_peer_cfg *pcfg, struct ibv_context *dev,
		struct rpma_peer **peer)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_peer_delete -- XXX
 */
int
rpma_peer_delete(struct rpma_peer **peer)
{
	return RPMA_E_NOSUPP;
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
