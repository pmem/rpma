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

#define RPMA_FLAG_ON(set, flag) (set) |= (flag)
#define RPMA_FLAG_OFF(set, flag) (set) &= ~(flag)

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
		RPMA_FLAG_ON(access, IBV_ACCESS_REMOTE_READ);
		RPMA_FLAG_OFF(usage, RPMA_MR_USAGE_READ_SRC);
	}

	if (usage & RPMA_MR_USAGE_READ_DST) {
		RPMA_FLAG_ON(access, IBV_ACCESS_LOCAL_WRITE);
		RPMA_FLAG_OFF(usage, RPMA_MR_USAGE_READ_DST);
	}

	/* XXX we will decide later how we should handle this situation */
	/* ASSERTeq(usage, 0); */

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
	return RPMA_E_NOSUPP;
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

	struct rpma_mr_local *mr;
	mr = (struct rpma_mr_local *)Malloc(sizeof(struct rpma_mr_local));
	if (mr == NULL)
		return RPMA_E_NOMEM;

	struct ibv_mr *ibv_mr;
	int ret = rpma_peer_mr_reg(peer, &ibv_mr, ptr, size,
			usage_to_access(usage));
	if (ret)
		goto err_free_rpma_mr_local;

	mr->ibv_mr = ibv_mr;
	mr->plt = plt;
	*mr_ptr = mr;

	return 0;

err_free_rpma_mr_local:
	Free(mr);

	return ret;
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
	if (mr->ibv_mr) {
		errno = ibv_dereg_mr(mr->ibv_mr);
		if (errno) {
			Rpma_provider_error = errno;
			ret = RPMA_E_PROVIDER;
		}
	}

	Free(mr);
	*mr_ptr = NULL;

	return ret;
}

/*
 * rpma_mr_serialize_get_size -- size of the buffer for serialized memory region
 */
size_t
rpma_mr_serialize_get_size()
{
	/*
	 * Calculate a sum of sizes of rpma_mr_remote structure's fields:
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
 * rpma_mr_deserialize -- deserialize a memory region
 */
int
rpma_mr_deserialize(char *buff, size_t buff_size,
		struct rpma_mr_remote **mr_ptr)
{
	if (buff == NULL || mr_ptr == NULL)
		return RPMA_E_INVAL;

	if (buff_size != rpma_mr_serialize_get_size())
		return RPMA_E_NOSUPP;

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
