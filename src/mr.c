// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * mr.c -- librpma memory region-related implementations
 */

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
