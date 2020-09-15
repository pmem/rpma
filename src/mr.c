// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

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
#include "rpma_err.h"

#ifdef TEST_MOCK_ALLOC
#include "cmocka_alloc.h"
#endif

/* a bit-wise OR of all allowed values */
#define USAGE_ALL_ALLOWED (RPMA_MR_USAGE_READ_SRC | RPMA_MR_USAGE_READ_DST |\
		RPMA_MR_USAGE_WRITE_SRC | RPMA_MR_USAGE_WRITE_DST |\
		RPMA_MR_USAGE_SEND | RPMA_MR_USAGE_RECV |\
		RPMA_MR_USAGE_FLUSHABLE)

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
 *
 * Note: APM type of flush requires the same access as RPMA_MR_USAGE_READ_SRC
 */
static int
usage_to_access(int usage)
{
	int access = 0;

	if (usage & (RPMA_MR_USAGE_READ_SRC | RPMA_MR_USAGE_FLUSHABLE)) {
		access |= IBV_ACCESS_REMOTE_READ;
		usage &= ~(RPMA_MR_USAGE_READ_SRC | RPMA_MR_USAGE_FLUSHABLE);
	}

	if (usage & RPMA_MR_USAGE_READ_DST) {
		access |= IBV_ACCESS_LOCAL_WRITE;
		usage &= ~RPMA_MR_USAGE_READ_DST;
	}

	if (usage & RPMA_MR_USAGE_WRITE_SRC) {
		access |= IBV_ACCESS_LOCAL_WRITE;
		usage &= ~RPMA_MR_USAGE_WRITE_SRC;
	}

	if (usage & RPMA_MR_USAGE_WRITE_DST) {
		/*
		 * If IBV_ACCESS_REMOTE_WRITE is set, then
		 * IBV_ACCESS_LOCAL_WRITE must be set too.
		 */
		access |= IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_LOCAL_WRITE;
		usage &= ~RPMA_MR_USAGE_WRITE_DST;
	}

	if (usage & RPMA_MR_USAGE_RECV) {
		access |= IBV_ACCESS_LOCAL_WRITE;
		usage &= ~RPMA_MR_USAGE_RECV;
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
	wr.next = NULL;
	wr.opcode = IBV_WR_RDMA_READ;
	wr.send_flags = (flags & RPMA_F_COMPLETION_ON_SUCCESS) ?
		IBV_SEND_SIGNALED : 0;

	struct ibv_send_wr *bad_wr;
	int ret = ibv_post_send(qp, &wr, &bad_wr);
	if (ret) {
		Rpma_provider_error = ret;
		RPMA_LOG_ERROR_WITH_ERRNO(Rpma_provider_error,
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
	struct rpma_mr_local *src,  size_t src_offset,
	size_t len, int flags, void *op_context)
{
	struct ibv_send_wr wr;
	struct ibv_sge sge;

	/* source */
	sge.addr = (uint64_t)((uintptr_t)src->ibv_mr->addr + src_offset);
	sge.length = (uint32_t)len;
	sge.lkey = src->ibv_mr->lkey;
	wr.sg_list = &sge;
	wr.num_sge = 1;

	/* destination */
	wr.wr.rdma.remote_addr = dst->raddr + dst_offset;
	wr.wr.rdma.rkey = dst->rkey;

	wr.wr_id = (uint64_t)op_context;
	wr.next = NULL;
	wr.opcode = IBV_WR_RDMA_WRITE;
	wr.send_flags = (flags & RPMA_F_COMPLETION_ON_SUCCESS) ?
		IBV_SEND_SIGNALED : 0;

	struct ibv_send_wr *bad_wr;
	int ret = ibv_post_send(qp, &wr, &bad_wr);
	if (ret) {
		Rpma_provider_error = ret;
		RPMA_LOG_ERROR_WITH_ERRNO(Rpma_provider_error,
			"ibv_post_send(src_addr=0x%x, rkey=0x%x, dst_addr=0x%x, length=%u, lkey=0x%x, wr_id=0x%x, opcode=IBV_WR_RDMA_WRITE, send_flags=%s)",
			wr.wr.rdma.remote_addr, wr.wr.rdma.rkey,
			sge.addr, sge.length, sge.lkey, wr.wr_id,
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
	struct rpma_mr_local *src,  size_t offset,
	size_t len, int flags, void *op_context)
{
	struct ibv_send_wr wr;
	struct ibv_sge sge;

	/* source */
	sge.addr = (uint64_t)((uintptr_t)src->ibv_mr->addr + offset);
	sge.length = (uint32_t)len;
	sge.lkey = src->ibv_mr->lkey;

	wr.sg_list = &sge;
	wr.num_sge = 1;
	wr.next = NULL;

	wr.opcode = IBV_WR_SEND;
	wr.wr_id = (uint64_t)op_context;
	wr.send_flags = (flags & RPMA_F_COMPLETION_ON_SUCCESS) ?
		IBV_SEND_SIGNALED : 0;

	struct ibv_send_wr *bad_wr;
	int ret = ibv_post_send(qp, &wr, &bad_wr);
	if (ret) {
		Rpma_provider_error = ret;
		RPMA_LOG_ERROR_WITH_ERRNO(Rpma_provider_error, "ibv_post_send");
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
	size_t len, void *op_context)
{
	struct ibv_recv_wr wr;
	struct ibv_sge sge;

	/* source */
	sge.addr = (uint64_t)((uintptr_t)dst->ibv_mr->addr + offset);
	sge.length = (uint32_t)len;
	sge.lkey = dst->ibv_mr->lkey;

	wr.sg_list = &sge;
	wr.num_sge = 1;
	wr.next = NULL;
	wr.wr_id = (uint64_t)op_context;

	struct ibv_recv_wr *bad_wr;
	int ret = ibv_post_recv(qp, &wr, &bad_wr);
	if (ret) {
		Rpma_provider_error = ret;
		RPMA_LOG_ERROR_WITH_ERRNO(Rpma_provider_error, "ibv_post_recv");
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
	mr = malloc(sizeof(struct rpma_mr_local));
	if (mr == NULL)
		return RPMA_E_NOMEM;

	struct ibv_mr *ibv_mr;
	int ret = rpma_peer_mr_reg(peer, &ibv_mr, ptr, size,
			usage_to_access(usage));
	if (ret) {
		free(mr);
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
		RPMA_LOG_ERROR_WITH_ERRNO(Rpma_provider_error,
				"ibv_dereg_mr()");
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
rpma_mr_get_descriptor(struct rpma_mr_local *mr, void *desc)
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

	*((uint8_t *)buff) = (uint8_t)mr->plt;

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

	size_t full_size = 2 * sizeof(uint64_t) + sizeof(uint32_t)
			+ sizeof(uint8_t);

	if (desc_size < full_size) {
		RPMA_LOG_ERROR(
			"incorrect size of the descriptor: %i bytes (should be at least: %i bytes)",
			desc_size, full_size);
		return RPMA_E_INVAL;
	}

	memcpy(&raddr, buff, sizeof(uint64_t));
	buff += sizeof(uint64_t);

	memcpy(&size, buff, sizeof(uint64_t));
	buff += sizeof(uint64_t);

	memcpy(&rkey, buff, sizeof(uint32_t));
	buff += sizeof(uint32_t);

	uint8_t plt = *(uint8_t *)buff;

	if (plt != RPMA_MR_PLT_VOLATILE && plt != RPMA_MR_PLT_PERSISTENT) {
		RPMA_LOG_ERROR(
			"incorrect value of the memory placement read from the descriptor: %i",
			plt);
		return RPMA_E_INVAL;
	}

	struct rpma_mr_remote *mr = malloc(sizeof(struct rpma_mr_remote));
	if (mr == NULL)
		return RPMA_E_NOMEM;

	mr->raddr = le64toh(raddr);
	mr->size = le64toh(size);
	mr->rkey = le32toh(rkey);
	mr->plt = plt;
	*mr_ptr = mr;

	RPMA_LOG_INFO("new rpma_mr_remote(raddr=0x%" PRIx64 ", size=%" PRIu64
			", rkey=0x%" PRIx32 ", plt=%s)",
			raddr, size, rkey,
			((plt == RPMA_MR_PLT_VOLATILE) ?
					"volatile" : "persistent"));

	return 0;
}

/*
 * rpma_mr_get_descriptor_size -- get size of a memory region descriptor
 */
int
rpma_mr_get_descriptor_size(struct rpma_mr_local *mr, size_t *desc_size)
{
	if (mr == NULL || desc_size == NULL)
		return RPMA_E_INVAL;

	*desc_size = 2 * sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint8_t);

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

	free(*mr_ptr);
	*mr_ptr = NULL;

	return 0;
}
