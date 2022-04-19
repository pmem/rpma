/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * mocks-rpma-cq.h -- librpma cq.c module mocks
 */

#ifndef MOCKS_RPMA_CQ_H
#define MOCKS_RPMA_CQ_H

#include "test-common.h"
#include "cq.h"

#define MOCK_RPMA_CQ		(struct rpma_cq *)0xD418
#define MOCK_RPMA_RCQ		(struct rpma_cq *)0xD419

struct state_rpma_ibv_create_comp_channel {
	struct ibv_context *ibv_ctx;
	struct ibv_comp_channel *channel;
};

struct state_rpma_ibv_destroy_comp_channel {
	struct ibv_comp_channel *channel;
};

#endif /* MOCKS_RPMA_CQ_H */
