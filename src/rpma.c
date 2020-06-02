/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * rpma.c -- entry points for librpma
 */

#include "librpma.h"

/* public librpma API */

/*
 * rpma_utils_get_ibv_context -- XXX
 */
int
rpma_utils_get_ibv_context(const char *addr, struct ibv_context **dev)
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
