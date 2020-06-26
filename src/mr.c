/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mr.c -- librpma memory region-related implementations
 */

#include "librpma.h"
#include "mr.h"

struct rpma_mr_local {
	struct ibv_mr *ibv_mr;
	enum rpma_mr_plt plt;
};

struct rpma_mr_remote {
	uint32_t rkey;
	enum rpma_mr_plt plt;
};

/* internal librpma API */
int
rpma_mr_read(struct ibv_qp *qp, void *op_context,
	struct rpma_mr_local *dst, size_t dst_offset,
	struct rpma_mr_remote *src,  size_t src_offset,
	size_t len, int flags)
{
	return RPMA_E_NOSUPP;
}

/* public librpma API */

/*
 * rpma_mr_reg -- use rpma_peer_mr_reg()
 */
int
rpma_mr_reg(struct rpma_peer *peer, void *ptr, size_t size, int usage,
		enum rpma_mr_plt plt, struct rpma_mr_local **mr)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_mr_dereg -- use ibv_dereg_mr()
 */
int
rpma_mr_dereg(struct rpma_mr_local **mr)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_mr_serialize_get_size -- XXX
 */
size_t
rpma_mr_serialize_get_size()
{
	return (size_t)-1;
}

/*
 * rpma_mr_serialiaze -- XXX
 */
int
rpma_mr_serialiaze(struct rpma_mr_local *mr, void *buff)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_mr_deserialize -- XXX
 */
int
rpma_mr_deserialize(void *buff, size_t buff_size, struct rpma_mr_remote **mr)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_mr_remote_delete -- XXX
 */
int
rpma_mr_remote_delete(struct rpma_mr_remote **mr)
{
	return RPMA_E_NOSUPP;
}
