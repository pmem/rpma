/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2022, Intel Corporation */
/* Copyright (c) 2022-2023, Fujitsu Limited */

/*
 * peer-common.h -- the header of the common part of the peer unit test
 */

#ifndef PEER_COMMON_H
#define PEER_COMMON_H 1

#include <infiniband/verbs.h>

#define MOCK_CM_ID		(struct rdma_cm_id *)0xC41D
#define MOCK_ADDR		(void *)0x2B6A

/*
 * The basic access value should be a combination of
 * IBV_ACCESS_(LOCAL|REMOTE)_(READ|WRITE) because IBV_ACCESS_ON_DEMAND
 * is added dynamically during the fall-back to using On-Demand Paging
 * registration type.
 */
#define MOCK_ACCESS (unsigned)(\
	IBV_ACCESS_LOCAL_WRITE |\
	IBV_ACCESS_REMOTE_WRITE |\
	IBV_ACCESS_REMOTE_READ)

/*
 * The test usage value is a combination of all possible
 * RPMA_MR_USAGE_* values.
 */
#define MOCK_USAGE (unsigned)(\
	RPMA_MR_USAGE_READ_SRC |\
	RPMA_MR_USAGE_READ_DST |\
	RPMA_MR_USAGE_WRITE_SRC |\
	RPMA_MR_USAGE_WRITE_DST |\
	RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY |\
	RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT |\
	RPMA_MR_USAGE_SEND |\
	RPMA_MR_USAGE_RECV)

struct prestate {
	enum ibv_transport_type transport_type;
	int usage;
	unsigned access;
	int is_odp_capable;
	int is_atomic_write_capable;
	int is_flush_capable;
	struct rpma_peer *peer;
};

extern struct prestate prestate_Capable;
extern struct prestate prestate_Incapable;

int setup__peer(void **pprestate);
int teardown__peer(void **pprestate);

#endif /* PEER_COMMON_H */
