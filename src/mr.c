// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mr.c -- librpma memory region-related implementations
 */

#include <endian.h>

#include "cmocka_alloc.h"
#include "librpma.h"
#include "mr.h"
#include "peer.h"
#include "out.h"
#include "rpma_err.h"

/* a bit-wise OR of all allowed values */
#define USAGE_ALL_ALLOWED (RPMA_MR_USAGE_READ_SRC | RPMA_MR_USAGE_READ_DST)

/* generate operation completion on success */
#define RPMA_F_COMPLETION_ON_SUCCESS \
	(RPMA_F_COMPLETION_ALWAYS & ~RPMA_F_COMPLETION_ON_ERROR)

struct rpma_mr_local {
	struct ibv_mr *ibv_mr; /* an IBV memory registration object */
	enum rpma_mr_plt plt; /* placement of the memory region */
};

struct rpma_mr_remote {
	uint64_t raddr; /* the base virtual address of the memory region */
	uint64_t size; /* the size of the memory being registered */
	uint32_t rkey; /* remote key of the memory region */
	enum rpma_mr_plt plt; /* placement of the memory region */
};

/* helper functions */

/*
 * usage_to_access -- convert usage to access
 */
static int
usage_to_access(int usage)
{
	int access = 0;

	if (usage & RPMA_MR_USAGE_READ_SRC) {
		access |= IBV_ACCESS_REMOTE_READ;
		usage &= ~RPMA_MR_USAGE_READ_SRC;
	}

	if (usage & RPMA_MR_USAGE_READ_DST) {
		access |= IBV_ACCESS_LOCAL_WRITE;
		usage &= ~RPMA_MR_USAGE_READ_DST;
	}

	return access;
}

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
	int ret = ibv_post_send(qp, &wr, &bad_wr);
	if (ret) {
		Rpma_provider_error = ret;
		return RPMA_E_PROVIDER;
	}

	return 0;
}

/* public librpma API */

/*
 * rpma_mr_reg -- create a local memory registration object
 */
int
rpma_mr_reg(struct rpma_peer *peer, void *ptr, size_t size, int usage,
		enum rpma_mr_plt plt, struct rpma_mr_local **mr_ptr)
{
	if (peer == NULL || ptr == NULL || size == 0 || mr_ptr == NULL)
		return RPMA_E_INVAL;

	if (usage == 0 || (usage & ~USAGE_ALL_ALLOWED))
		return RPMA_E_INVAL;

	struct rpma_mr_local *mr;
	mr = Malloc(sizeof(struct rpma_mr_local));
	if (mr == NULL)
		return RPMA_E_NOMEM;

	struct ibv_mr *ibv_mr;
	int ret = rpma_peer_mr_reg(peer, &ibv_mr, ptr, size,
			usage_to_access(usage));
	if (ret) {
		Free(mr);
		return ret;
	}

	mr->ibv_mr = ibv_mr;
	mr->plt = plt;
	*mr_ptr = mr;

	return 0;
}

/*
 * rpma_mr_dereg -- delete a local memory registration object
 */
int
rpma_mr_dereg(struct rpma_mr_local **mr_ptr)
{
	if (mr_ptr == NULL)
		return RPMA_E_INVAL;

	if (*mr_ptr == NULL)
		return 0;

	int ret = 0;
	struct rpma_mr_local *mr = *mr_ptr;
	errno = ibv_dereg_mr(mr->ibv_mr);
	if (errno) {
		Rpma_provider_error = errno;
		ret = RPMA_E_PROVIDER;
	}

	Free(mr);
	*mr_ptr = NULL;

	return ret;
}

/*
 * rpma_mr_get_descriptor -- get a descriptor of memory region
 */
int
rpma_mr_get_descriptor(struct rpma_mr_local *mr, rpma_mr_descriptor *desc)
{
	if (mr == NULL || desc == NULL)
		return RPMA_E_INVAL;

	char *buff = (char *)desc;

	*((uint64_t *)buff) = htole64((uint64_t)mr->ibv_mr->addr);
	buff += sizeof(uint64_t);
	*((uint64_t *)buff) = htole64((uint64_t)mr->ibv_mr->length);
	buff += sizeof(uint64_t);
	*((uint32_t *)buff) = htole32(mr->ibv_mr->rkey);
	buff += sizeof(uint32_t);
	*((uint8_t *)buff) = (uint8_t)mr->plt;

	return 0;
}

/*
 * rpma_mr_remote_from_descriptor -- create a remote memory region from
 * a descriptor
 */
int
rpma_mr_remote_from_descriptor(const rpma_mr_descriptor *desc,
		struct rpma_mr_remote **mr_ptr)
{
	if (desc == NULL || mr_ptr == NULL)
		return RPMA_E_INVAL;

	char *buff = (char *)desc;

	uint64_t raddr =  le64toh(*(uint64_t *)buff);
	buff += sizeof(uint64_t);
	uint64_t size = le64toh(*(uint64_t *)buff);
	buff += sizeof(uint64_t);
	uint32_t rkey = le32toh(*(uint32_t *)buff);
	buff += sizeof(uint32_t);
	uint8_t plt = *(uint8_t *)buff;

	if (plt != RPMA_MR_PLT_VOLATILE && plt != RPMA_MR_PLT_PERSISTENT)
		return RPMA_E_NOSUPP;

	struct rpma_mr_remote *mr = Malloc(sizeof(struct rpma_mr_remote));
	if (mr == NULL)
		return RPMA_E_NOMEM;

	mr->raddr = raddr;
	mr->size = size;
	mr->rkey = rkey;
	mr->plt = plt;
	*mr_ptr = mr;

	return 0;
}

/*
 * rpma_mr_remote_get_size -- get a remote memory region size
 */
int
rpma_mr_remote_get_size(struct rpma_mr_remote *mr, size_t *size)
{
	if (mr == NULL || size == NULL)
		return RPMA_E_INVAL;

	*size = mr->size;

	return 0;
}

/*
 * rpma_mr_remote_delete -- delete a remote memory region's structure
 */
int
rpma_mr_remote_delete(struct rpma_mr_remote **mr_ptr)
{
	if (mr_ptr == NULL)
		return RPMA_E_INVAL;

	if (*mr_ptr == NULL)
		return 0;

	Free(*mr_ptr);
	*mr_ptr = NULL;

	return 0;
}
