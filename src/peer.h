/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * peer.h -- internal librpma peer-related definitions
 */

#ifndef LIBRPMA_PEER_H
#define LIBRPMA_PEER_H 1

struct rpma_peer {
	struct ibv_pd *pd;
};

#endif /* LIBRPMA_PEER_H */
