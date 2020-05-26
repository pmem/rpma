/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * peer.c -- entry points for librpma
 */

#include "alloc.h"
#include "error.h"
#include "mr.h"
#include "out.h"
#include "peer.h"
#include "rpma_err.h"
#include "util.h"

#include "librpma.h"

/*
 * mr_usage_to_verbs_access -- (internal) convert RPMA usage to verbs memory
 * region access
 */
static int
mr_usage_to_verbs_access(int usage)
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

	ASSERTeq(usage, 0);

	return access;
}

/*
 * rpma_mr_reg -- create a new local memory object encapsulating a newly
 * registered memory region using provided ptr, size and usage
 */
int
rpma_mr_reg(struct rpma_peer *peer, void *ptr, size_t size,
		int usage, int plt, struct rpma_mr_local **mr_ptr)
{
	if (peer == NULL || plt != 0 || mr_ptr == NULL)
		return RPMA_E_INVAL;

	if (usage & (~RPMA_MR_USAGE_VALID))
		return RPMA_E_INVAL;

	int access = mr_usage_to_verbs_access(usage);
	int ret = 0;

	struct ibv_mr *ibv_mr = ibv_reg_mr(peer->pd, ptr, size, access);
	if (ibv_mr == NULL) {
		Rpma_provider_error = errno;
		return RPMA_E_PROVIDER;
	}

	struct rpma_mr_local *mr = Malloc(sizeof(*mr));
	if (mr == NULL) {
		ASSERTeq(errno, ENOMEM);
		ret = RPMA_E_NOMEM;
		goto err_dereg_mr;
	}

	mr->ibv_mr = ibv_mr;
	*mr_ptr = mr;

	return 0;

err_dereg_mr:
	ibv_dereg_mr(ibv_mr);
	return ret;
}

/*
 * rpma_mr_dereg -- attempt deregistering the mr-encapsulated verbs
 * memory region; if succeeded delete the mr
 */
int
rpma_mr_dereg(struct rpma_mr_local **mr_ptr)
{
	if (mr_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_mr_local *mr = *mr_ptr;

	if (mr == NULL)
		return 0;

	int ret = ibv_dereg_mr(mr->ibv_mr);
	if (ret) {
		Rpma_provider_error = ret;
		return RPMA_E_PROVIDER;
	}

	Free(mr);
	*mr_ptr = NULL;

	return 0;
}
