/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * peer.c -- entry points for librpma
 */

#include "alloc.h"
#include "error.h"
#include "out.h"
#include "peer.h"
#include "rpma_err.h"

#include "librpma.h"

/*
 * rpma_peer_new -- create a new peer object encapsulating a newly allocated
 * verbs protection domain for provided ibv_context
 */
int
rpma_peer_new(struct rpma_peer_cfg *pcfg, struct ibv_context *dev,
		struct rpma_peer **peer_ptr)
{
	int ret;

	if (pcfg != NULL || dev == NULL || peer_ptr == NULL)
		return RPMA_E_INVAL;

	struct ibv_pd *pd = ibv_alloc_pd(dev);
	if (pd == NULL)
		return RPMA_E_UNKNOWN;

	struct rpma_peer *peer = Malloc(sizeof(*peer));
	if (peer == NULL) {
		ASSERTeq(errno, ENOMEM);
		ret = RPMA_E_NOMEM;
		goto err_dealloc_pd;
	}

	peer->pd = pd;
	*peer_ptr = peer;

	return 0;

err_dealloc_pd:
	ibv_dealloc_pd(pd);
	return ret;
}

/*
 * rpma_peer_delete -- attempt deallocating the peer-encapsulated verbs
 * protection domain; if succeeded delete the peer
 */
int
rpma_peer_delete(struct rpma_peer **peer_ptr)
{
	if (peer_ptr == NULL)
		return RPMA_E_INVAL;

	struct rpma_peer *peer = *peer_ptr;
	if (peer == NULL)
		return 0;

	int ret = ibv_dealloc_pd(peer->pd);
	if (ret) {
		Rpma_provider_error = ret;
		return RPMA_E_PROVIDER;
	}

	Free(peer);
	*peer_ptr = NULL;

	return 0;
}
