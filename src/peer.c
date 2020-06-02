/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * peer.c -- librpma peer-related implementations
 */

#include <errno.h>

#include "cmocka_alloc.h"
#include "rpma_err.h"
#include "out.h"

#include "librpma.h"

struct rpma_peer {
	struct ibv_pd *pd;
};

/*
 * rpma_peer_new -- create a new peer object encapsulating a newly allocated
 * verbs protection domain for provided ibv_context
 */
int
rpma_peer_new(struct ibv_context *ibv_ctx, struct rpma_peer **peer_ptr)
{
	int ret;

	if (ibv_ctx == NULL || peer_ptr == NULL)
		return RPMA_E_INVAL;

	/*
	 * The ibv_alloc_pd(3) manual page does not document that this function
	 * returns any error via errno but seemingly it is. For the usability
	 * sake, we try to deduce what really happened using the errno value.
	 * To make sure the errno value was set by the ibv_alloc_pd(3) function
	 * it is zeroed out before the function call.
	 */
	errno = 0;
	struct ibv_pd *pd = ibv_alloc_pd(ibv_ctx);
	if (pd == NULL) {
		if (errno == ENOMEM) {
			return RPMA_E_NOMEM;
		} else if (errno != 0) {
			Rpma_provider_error = errno;
			return RPMA_E_PROVIDER;
		} else {
			return RPMA_E_UNKNOWN;
		}
	}

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
