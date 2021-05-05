/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2021, Intel Corporation */

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
#define MOCK_ACCESS		(unsigned)7

/*
 * The test usage value is a combination of all possible
 * RPMA_MR_USAGE_* values.
 */
#define MOCK_USAGE		(unsigned)255

extern int OdpCapable;
extern int OdpIncapable;

int setup__peer(void **in_out);
int teardown__peer(void **peer_ptr);

#endif /* PEER_COMMON_H */
