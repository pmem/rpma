/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * peer.c -- entry points for librpma peer
 */

#include <librpma.h>

#include "device.h"
#include "peer.h"
#include "rpma_utils.h"
#include "alloc.h"

int
rpma_peer_new(struct rpma_peer_cfg *zcfg, struct rpma_device *device,
		struct rpma_peer **peer)
{
	int ret = 0;

	struct rpma_peer *ptr = Malloc(sizeof(struct rpma_peer));
	if (!ptr)
		return RPMA_E_ERRNO;

	ptr->pd = NULL;

	/* protection domain */
	ptr->pd = ibv_alloc_pd(device->device);
	if (!ptr->pd) {
		ret = RPMA_E_UNKNOWN; /* XXX */
		goto err_alloc_pd;
	}

	*peer = ptr;

	return ret;

err_alloc_pd:
	Free(ptr);
	return ret;
}

int
rpma_peer_delete(struct rpma_peer **peer)
{
	struct rpma_peer *ptr = *peer;
	if (!ptr)
		return 0;

	if (ptr->pd)
		ibv_dealloc_pd(ptr->pd);

	Free(ptr);
	*peer = NULL;

	return 0;
}
