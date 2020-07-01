// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mr.c -- librpma memory region-related implementations
 */

#include <endian.h>

#include "cmocka_alloc.h"
#include "librpma.h"
#include "mr.h"

struct rpma_mr_local {
	struct ibv_mr *ibv_mr; /* an IBV memory registration object */
	enum rpma_mr_plt plt; /* placement of the memory region */
};

struct rpma_mr_remote {
	uint64_t raddr; /* the base virtual address of the memory region */
	uint64_t size; /* the size of the memory registration */
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
 * rpma_mr_serialize_get_size -- size of the buffer for serialized memory region
 */
size_t
rpma_mr_serialize_get_size()
{
	/*
	 * Calculate a sum of rpma_mr_remote structure's fields size:
	 * sum = raddr + size + rkey + plt
	 *
	 * Note: plt has two possible values so it fits perfectly into a single
	 * uint8_t value.
	 */
	return sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint32_t) +
			sizeof(uint8_t);
}

/*
 * rpma_mr_serialize -- serialize a memory region
 */
int
rpma_mr_serialize(struct rpma_mr_local *mr, char *buff)
{
	if (mr == NULL || buff == NULL)
		return RPMA_E_INVAL;

	uint64_t *ptr64 = (uint64_t *)buff;
	uint32_t *ptr32 = NULL;
	uint8_t *ptr8 = NULL;

	*ptr64 = htobe64((uint64_t)mr->ibv_mr->addr);
	++ptr64;
	*ptr64 = htobe64((uint64_t)mr->ibv_mr->length);
	ptr32 = (uint32_t *)++ptr64;
	*ptr32 = htobe32(mr->ibv_mr->rkey);
	ptr8 = (uint8_t *)++ptr32;
	*ptr8 = mr->plt;

	return 0;
}

/*
 * rpma_mr_deserialize -- deserialize a memory region
 */
int
rpma_mr_deserialize(char *buff, size_t buff_size,
		struct rpma_mr_remote **mr_ptr)
{
	if (buff == NULL || mr_ptr == NULL)
		return RPMA_E_INVAL;

	size_t exp_buff_size = rpma_mr_serialize_get_size();
	if (buff_size != exp_buff_size)
		return RPMA_E_NOSUPP;

	struct rpma_mr_remote *mr = Malloc(sizeof(struct rpma_mr_remote));
	if (mr == NULL)
		return RPMA_E_NOMEM;

	uint64_t *ptr64 = (uint64_t *)buff;
	uint32_t *ptr32 = NULL;
	uint8_t *ptr8 = NULL;

	mr->raddr = be64toh(*ptr64);
	++ptr64;
	mr->size = be64toh(*ptr64);
	ptr32 = (uint32_t *)++ptr64;
	mr->rkey = be32toh(*ptr32);
	ptr8 = (uint8_t *)++ptr32;
	mr->plt = *ptr8;

	if (mr->plt != RPMA_MR_PLT_VOLATILE &&
			mr->plt != RPMA_MR_PLT_PERSISTENT) {
		Free(mr);
		return RPMA_E_UNKNOWN;
	}

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
