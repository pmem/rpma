// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */
/* Copyright (c) 2021 Fujitsu */

/*
 * mr.c -- librpma memory region-related implementations
 */

#include <endian.h>
#include <inttypes.h>
#include <stdlib.h>

#include "librpma.h"
#include "log_internal.h"
#include "mr.h"
#include "peer.h"

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

#define STATIC_ASSERT(cond, msg)\
	typedef char static_assertion_##msg[(cond) ? 1 : -1]

#define SIZEOF_IN_BITS(type)	(8 * sizeof(type))
#define MAX_VALUE_OF(type)	((1 << SIZEOF_IN_BITS(type)) - 1)

#define RPMA_MR_DESC_SIZE (2 * sizeof(uint64_t) + sizeof(uint32_t) \
			+ sizeof(uint8_t))

/* a bit-wise OR of all allowed values */
#define USAGE_ALL_ALLOWED (RPMA_MR_USAGE_READ_SRC | RPMA_MR_USAGE_READ_DST |\
		RPMA_MR_USAGE_WRITE_SRC | RPMA_MR_USAGE_WRITE_DST |\
		RPMA_MR_USAGE_SEND | RPMA_MR_USAGE_RECV |\
		RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY |\
		RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT)

/*
 * Make sure the size of the usage field in the rpma_mr_get_descriptor()
 * and rpma_mr_remote_from_descriptor() functions ('uint8_t' as for now)
 * is big enough to store all possible 'RPMA_MR_USAGE_*' values.
 */
STATIC_ASSERT(USAGE_ALL_ALLOWED <= MAX_VALUE_OF(uint8_t), usage_too_small);

/* generate operation completion on success */
#define RPMA_F_COMPLETION_ON_SUCCESS \
	(RPMA_F_COMPLETION_ALWAYS & ~RPMA_F_COMPLETION_ON_ERROR)

struct rpma_mr_local {
	struct ibv_mr *ibv_mr; /* an IBV memory registration object */
	int usage; /* usage of the memory region */
};

struct rpma_mr_remote {
	uint64_t raddr; /* the base virtual address of the memory region */
	uint64_t size; /* the size of the memory being registered */
	uint32_t rkey; /* remote key of the memory region */
	int usage; /* usage of the memory region */
};

/* internal librpma API */

/*
 * rpma_mr_read -- post an RDMA read from src to dst
 */
int
rpma_mr_read(struct ibv_qp *qp,
	struct rpma_mr_local *dst, size_t dst_offset,
	const struct rpma_mr_remote *src,  size_t src_offset,
	size_t len, int flags, const void *op_context)
{
	struct ibv_send_wr wr;
	struct ibv_sge sge;

	if (src == NULL) {
		/* source */
		wr.wr.rdma.remote_addr = 0;
		wr.wr.rdma.rkey = 0;

		/* destination */
		wr.sg_list = NULL;
		wr.num_sge = 0;
	} else {
		/* source */
		wr.wr.rdma.remote_addr = src->raddr + src_offset;
		wr.wr.rdma.rkey = src->rkey;

		/* destination */
		sge.addr = (uint64_t)((uintptr_t)dst->ibv_mr->addr +
				dst_offset);
		sge.length = (uint32_t)len;
		sge.lkey = dst->ibv_mr->lkey;

		wr.sg_list = &sge;
		wr.num_sge = 1;
	}

	wr.wr_id = (uint64_t)op_context;
	wr.next = NULL;
	wr.opcode = IBV_WR_RDMA_READ;
	wr.send_flags = (flags & RPMA_F_COMPLETION_ON_SUCCESS) ?
		IBV_SEND_SIGNALED : 0;

	struct ibv_send_wr *bad_wr;
	int ret = ibv_post_send(qp, &wr, &bad_wr);
	if (ret) {
		RPMA_LOG_ERROR_WITH_ERRNO(ret,
			"ibv_post_send(src_addr=0x%x, rkey=0x%x, dst_addr=0x%x, length=%u, lkey=0x%x, wr_id=0x%x, opcode=IBV_WR_RDMA_READ, send_flags=%s)",
			wr.wr.rdma.remote_addr, wr.wr.rdma.rkey,
			sge.addr, sge.length, sge.lkey, wr.wr_id,
			(flags & RPMA_F_COMPLETION_ON_SUCCESS) ?
				"IBV_SEND_SIGNALED" : "0");
		return RPMA_E_PROVIDER;
	}

	return 0;
}

/*
 * rpma_mr_write -- post an RDMA write from src to dst
 */
int
rpma_mr_write(struct ibv_qp *qp,
	struct rpma_mr_remote *dst, size_t dst_offset,
	const struct rpma_mr_local *src, size_t src_offset,
	size_t len, int flags, enum ibv_wr_opcode operation,
	uint32_t imm, const void *op_context, bool fence)
{
	struct ibv_send_wr wr;
	struct ibv_sge sge;

	if (src == NULL) {
		/* source */
		wr.sg_list = NULL;
		wr.num_sge = 0;

		/* destination */
		wr.wr.rdma.remote_addr = 0;
		wr.wr.rdma.rkey = 0;
	} else {
		/* source */
		sge.addr = (uint64_t)((uintptr_t)src->ibv_mr->addr +
				src_offset);
		sge.length = (uint32_t)len;
		sge.lkey = src->ibv_mr->lkey;

		wr.sg_list = &sge;
		wr.num_sge = 1;

		/* destination */
		wr.wr.rdma.remote_addr = dst->raddr + dst_offset;
		wr.wr.rdma.rkey = dst->rkey;
	}

	wr.wr_id = (uint64_t)op_context;
	wr.next = NULL;

	wr.opcode = operation;
	switch (wr.opcode) {
	case IBV_WR_RDMA_WRITE:
		break;
	case IBV_WR_RDMA_WRITE_WITH_IMM:
		wr.imm_data = htonl(imm);
		break;
	default:
		RPMA_LOG_ERROR("unsupported wr.opcode == %d", wr.opcode);
		return RPMA_E_NOSUPP;
	}

	wr.send_flags = (flags & RPMA_F_COMPLETION_ON_SUCCESS) ?
		IBV_SEND_SIGNALED : 0;
	wr.send_flags |= fence ? IBV_SEND_FENCE : 0;

	struct ibv_send_wr *bad_wr;
	int ret = ibv_post_send(qp, &wr, &bad_wr);
	if (ret) {
		RPMA_LOG_ERROR_WITH_ERRNO(ret,
			"ibv_post_send(dst_addr=0x%x, rkey=0x%x, src_addr=0x%x, length=%u, lkey=0x%x, wr_id=0x%x, opcode=IBV_WR_RDMA_WRITE, send_flags=%s)",
			wr.wr.rdma.remote_addr, wr.wr.rdma.rkey,
			sge.addr, sge.length, sge.lkey, wr.wr_id,
			(flags & RPMA_F_COMPLETION_ON_SUCCESS) ?
				"IBV_SEND_SIGNALED" : "0");
		return RPMA_E_PROVIDER;
	}

	return 0;
}

/*
 * rpma_mr_atomic_write -- post the atomic 8 bytes RDMA write from src to dst
 */
int
rpma_mr_atomic_write(struct ibv_qp *qp,
	struct rpma_mr_remote *dst, size_t dst_offset,
	const char src[8], int flags, const void *op_context)
{
	struct ibv_send_wr wr;
	struct ibv_sge sge;

	/* source */
	sge.addr = (uint64_t)((uintptr_t)src);
	sge.length = 8; /* 8-bytes atomic write with IBV_SEND_INLINE flag */
	wr.sg_list = &sge;
	wr.num_sge = 1;

	/* destination */
	wr.wr.rdma.remote_addr = dst->raddr + dst_offset;
	wr.wr.rdma.rkey = dst->rkey;

	wr.wr_id = (uint64_t)op_context;
	wr.next = NULL;
	wr.opcode = IBV_WR_RDMA_WRITE;

	/*
	 * IBV_SEND_FENCE is used here to force any ongoing read operation
	 * (that may emulate a remote flush) to be finished before
	 * the atomic write is executed.
	 */
	wr.send_flags = IBV_SEND_INLINE | IBV_SEND_FENCE;
	if (flags & RPMA_F_COMPLETION_ON_SUCCESS)
		wr.send_flags |= IBV_SEND_SIGNALED;

	struct ibv_send_wr *bad_wr;
	int ret = ibv_post_send(qp, &wr, &bad_wr);
	if (ret) {
		RPMA_LOG_ERROR_WITH_ERRNO(ret,
			"ibv_post_send(dst_addr=0x%x, rkey=0x%x, src_addr=0x%x, wr_id=0x%x, opcode=IBV_WR_RDMA_WRITE, send_flags=%s)",
			wr.wr.rdma.remote_addr, wr.wr.rdma.rkey,
			sge.addr, wr.wr_id,
			(flags & RPMA_F_COMPLETION_ON_SUCCESS) ?
				"IBV_SEND_SIGNALED" : "0");
		return RPMA_E_PROVIDER;
	}

	return 0;
}

/*
 * rpma_mr_send -- post an RDMA send from src
 */
int
rpma_mr_send(struct ibv_qp *qp,
	const struct rpma_mr_local *src,  size_t offset,
	size_t len, int flags, enum ibv_wr_opcode operation,
	uint32_t imm, const void *op_context)
{
	struct ibv_send_wr wr;
	struct ibv_sge sge;

	/* source */
	if (src == NULL) {
		wr.sg_list = NULL;
		wr.num_sge = 0;
	} else {
		sge.addr = (uint64_t)((uintptr_t)src->ibv_mr->addr + offset);
		sge.length = (uint32_t)len;
		sge.lkey = src->ibv_mr->lkey;

		wr.sg_list = &sge;
		wr.num_sge = 1;
	}

	wr.next = NULL;
	wr.opcode = operation;
	switch (wr.opcode) {
	case IBV_WR_SEND:
		break;
	case IBV_WR_SEND_WITH_IMM:
		wr.imm_data = htonl(imm);
		break;
	default:
		RPMA_LOG_ERROR("unsupported wr.opcode == %d", wr.opcode);
		return RPMA_E_NOSUPP;
	}

	wr.wr_id = (uint64_t)op_context;
	wr.send_flags = (flags & RPMA_F_COMPLETION_ON_SUCCESS) ?
		IBV_SEND_SIGNALED : 0;

	struct ibv_send_wr *bad_wr;
	int ret = ibv_post_send(qp, &wr, &bad_wr);
	if (ret) {
		RPMA_LOG_ERROR_WITH_ERRNO(ret, "ibv_post_send");
		return RPMA_E_PROVIDER;
	}

	return 0;
}

/*
 * rpma_mr_recv -- post an RDMA recv from dst
 */
int
rpma_mr_recv(struct ibv_qp *qp,
	struct rpma_mr_local *dst,  size_t offset,
	size_t len, const void *op_context)
{
	struct ibv_recv_wr wr;
	struct ibv_sge sge;

	/* source */
	if (dst == NULL) {
		wr.sg_list = NULL;
		wr.num_sge = 0;
	} else {
		sge.addr = (uint64_t)((uintptr_t)dst->ibv_mr->addr + offset);
		sge.length = (uint32_t)len;
		sge.lkey = dst->ibv_mr->lkey;

		wr.sg_list = &sge;
		wr.num_sge = 1;
	}

	wr.next = NULL;
	wr.wr_id = (uint64_t)op_context;

	struct ibv_recv_wr *bad_wr;
	int ret = ibv_post_recv(qp, &wr, &bad_wr);
	if (ret) {
		RPMA_LOG_ERROR_WITH_ERRNO(ret, "ibv_post_recv");
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
		struct rpma_mr_local **mr_ptr)
{
	int ret;

	if (peer == NULL || ptr == NULL || size == 0 || mr_ptr == NULL)
		return RPMA_E_INVAL;

	if (usage == 0 || (usage & ~USAGE_ALL_ALLOWED))
		return RPMA_E_INVAL;

	struct rpma_mr_local *mr;
	mr = malloc(sizeof(struct rpma_mr_local));
	if (mr == NULL)
		return RPMA_E_NOMEM;

	struct ibv_mr *ibv_mr;
	if ((ret = rpma_peer_mr_reg(peer, &ibv_mr, ptr, size, usage))) {
		free(mr);
		return ret;
	}

	mr->ibv_mr = ibv_mr;
	mr->usage = usage;
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
		RPMA_LOG_ERROR_WITH_ERRNO(errno, "ibv_dereg_mr()");
		ret = RPMA_E_PROVIDER;
	}

	free(mr);
	*mr_ptr = NULL;

	return ret;
}

/*
 * rpma_mr_get_descriptor -- get a descriptor of memory region
 */
int
rpma_mr_get_descriptor(const struct rpma_mr_local *mr, void *desc)
{
	if (mr == NULL || desc == NULL)
		return RPMA_E_INVAL;

	char *buff = (char *)desc;

	uint64_t addr = htole64((uint64_t)mr->ibv_mr->addr);
	memcpy(buff, &addr, sizeof(uint64_t));
	buff += sizeof(uint64_t);

	uint64_t length = htole64((uint64_t)mr->ibv_mr->length);
	memcpy(buff, &length, sizeof(uint64_t));
	buff += sizeof(uint64_t);

	uint32_t rkey = htole32(mr->ibv_mr->rkey);
	memcpy(buff, &rkey, sizeof(uint32_t));
	buff += sizeof(uint32_t);

	*((uint8_t *)buff) = (uint8_t)mr->usage;

	return 0;
}

/*
 * rpma_mr_remote_from_descriptor -- create a remote memory region from
 * a descriptor
 */
int
rpma_mr_remote_from_descriptor(const void *desc,
		size_t desc_size, struct rpma_mr_remote **mr_ptr)
{
	if (desc == NULL || mr_ptr == NULL)
		return RPMA_E_INVAL;

	char *buff = (char *)desc;

	uint64_t raddr;
	uint64_t size;
	uint32_t rkey;

	if (desc_size < RPMA_MR_DESC_SIZE) {
		RPMA_LOG_ERROR(
			"incorrect size of the descriptor: %i bytes (should be at least: %i bytes)",
			desc_size, RPMA_MR_DESC_SIZE);
		return RPMA_E_INVAL;
	}

	memcpy(&raddr, buff, sizeof(uint64_t));
	buff += sizeof(uint64_t);

	memcpy(&size, buff, sizeof(uint64_t));
	buff += sizeof(uint64_t);

	memcpy(&rkey, buff, sizeof(uint32_t));
	buff += sizeof(uint32_t);

	uint8_t usage = *(uint8_t *)buff;

	if (usage == 0) {
		RPMA_LOG_ERROR("usage type of memory is not set");
		return RPMA_E_INVAL;
	}

	struct rpma_mr_remote *mr = malloc(sizeof(struct rpma_mr_remote));
	if (mr == NULL)
		return RPMA_E_NOMEM;

	mr->raddr = le64toh(raddr);
	mr->size = le64toh(size);
	mr->rkey = le32toh(rkey);
	mr->usage = usage;
	*mr_ptr = mr;

	RPMA_LOG_INFO("new rpma_mr_remote(raddr=0x%" PRIx64 ", size=%" PRIu64
			", rkey=0x%" PRIx32 ", usage=0x%" PRIx8 ")",
			raddr, size, rkey, usage);

	return 0;
}

/*
 * rpma_mr_get_descriptor_size -- get size of a memory region descriptor
 */
int
rpma_mr_get_descriptor_size(const struct rpma_mr_local *mr, size_t *desc_size)
{
	if (mr == NULL || desc_size == NULL)
		return RPMA_E_INVAL;

	*desc_size = RPMA_MR_DESC_SIZE;

	return 0;
}

/*
 * rpma_mr_get_ptr -- get a local registered memory pointer
 */
int
rpma_mr_get_ptr(const struct rpma_mr_local *mr, void **ptr)
{
	if (mr == NULL || ptr == NULL)
		return RPMA_E_INVAL;

	*ptr = mr->ibv_mr->addr;

	return 0;
}

/*
 * rpma_mr_get_size -- get a local registered memory size
 */
int
rpma_mr_get_size(const struct rpma_mr_local *mr, size_t *size)
{
	if (mr == NULL || size == NULL)
		return RPMA_E_INVAL;

	*size = mr->ibv_mr->length;

	return 0;
}

/*
 * rpma_mr_remote_get_size -- get a remote memory region size
 */
int
rpma_mr_remote_get_size(const struct rpma_mr_remote *mr, size_t *size)
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

	free(*mr_ptr);
	*mr_ptr = NULL;

	return 0;
}

/*
 * rpma_mr_remote_get_flush_type -- get a flush type supported
 * by the remote memory region
 */
int
rpma_mr_remote_get_flush_type(const struct rpma_mr_remote *mr, int *flush_type)
{
	if (mr == NULL || flush_type == NULL)
		return RPMA_E_INVAL;

	*flush_type = mr->usage & (RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT |
					RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY);

	return 0;
}

/*
 * rpma_mr_advise -- give advice about an address range in a memory registration
 */
int
rpma_mr_advise(struct rpma_mr_local *mr, size_t offset, size_t len,
		int advice, uint32_t flags)
{
#ifdef IBV_ADVISE_MR_SUPPORTED
	struct ibv_sge sg_list;
	sg_list.lkey = mr->ibv_mr->lkey;
	sg_list.addr = (uint64_t)((uintptr_t)mr->ibv_mr->addr + offset);
	sg_list.length = (uint32_t)len;

	int ret = ibv_advise_mr(mr->ibv_mr->pd,
			(enum ibv_advise_mr_advice)advice, flags, &sg_list, 1);
	if (ret) {
		RPMA_LOG_ERROR_WITH_ERRNO(ret, "ibv_advise_mr()");
		if (ret == EOPNOTSUPP || ret == ENOTSUP)
			return RPMA_E_NOSUPP;
		else if (ret == EFAULT || ret == EINVAL)
			return RPMA_E_INVAL;
		else
			return RPMA_E_PROVIDER;
	}

	return 0;
#else
	RPMA_LOG_ERROR("ibv_advise_mr() is not supported by the system");

	return RPMA_E_NOSUPP;
#endif
}
