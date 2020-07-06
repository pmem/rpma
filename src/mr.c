// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mr.c -- librpma memory region-related implementations
 */

#include "librpma.h"
#include "mr.h"
#include "rpma_err.h"

/* generate operation completion on success */
#define RPMA_F_COMPLETION_ON_SUCCESS	(1 << 1)

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
	struct ibv_send_wr wr;
	struct ibv_sge sge;

	/* source */
	wr.wr.rdma.remote_addr = src->raddr + src_offset;
	wr.wr.rdma.rkey = src->rkey;

	/* destination */
	sge.addr = (uint64_t)((uintptr_t)dst->ibv_mr->addr + dst_offset);
	sge.length = (uint32_t)len;
	sge.lkey = dst->ibv_mr->lkey;
	wr.sg_list = &sge;
	wr.num_sge = 1;

	wr.wr_id = (uint64_t)op_context;
	wr.opcode = IBV_WR_RDMA_READ;
	wr.send_flags = (flags & RPMA_F_COMPLETION_ON_SUCCESS) ?
		IBV_SEND_SIGNALED : 0;

	struct ibv_send_wr *bad_wr;
	errno = ibv_post_send(qp, &wr, &bad_wr);
	if (errno) {
		Rpma_provider_error = errno;
		return RPMA_E_PROVIDER;
	}

	return 0;
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
rpma_mr_serialize(struct rpma_mr_local *mr, char *buff)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_mr_deserialize -- XXX
 */
int
rpma_mr_deserialize(char *buff, size_t buff_size, struct rpma_mr_remote **mr)
{
	return RPMA_E_NOSUPP;
}

/*
 * rpma_mr_remote_get_size -- XXX
 */
int
rpma_mr_remote_get_size(struct rpma_mr_remote *mr, size_t *size)
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
