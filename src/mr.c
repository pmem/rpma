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
	struct ibv_mr *ibv_mr; /* an IBV memory registration object */
	enum rpma_mr_plt plt; /* placement of the memory region */
};

struct rpma_mr_remote {
	uint64_t raddr; /* the base virtual address of the memory region */
	uint32_t rkey; /* remote key of the memory region */
	enum rpma_mr_plt plt; /* placement of the memory region */
};

/* internal librpma API */

/*
 * rpma_mr_read -- post an RDMA read from src to dst
 */
int
rpma_mr_read(struct ibv_qp *qp,
	struct rpma_mr_local *dst, size_t dst_offset,
	struct rpma_mr_remote *src,  size_t src_offset,
	size_t len, int flags, void *op_context)
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
	return sizeof(struct rpma_mr_remote); /* XXX alignment? */
}

/*
 * rpma_mr_serialize -- XXX
 */
int
rpma_mr_serialize(struct rpma_mr_local *mr, void *buff)
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
