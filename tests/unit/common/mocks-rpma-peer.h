/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2022, Intel Corporation */

/*
 * mocks-rpma-peer.h -- a rpma_peer mocks header
 */

#ifndef MOCKS_RPMA_PEER_H
#define MOCKS_RPMA_PEER_H

#define MOCK_PTR	(void *)0x0001020304050607
#define MOCK_SIZE	(size_t)0x08090a0b0c0d0e0f
#define MOCK_RKEY	(uint32_t)0x10111213

/* structure of arguments used in rpma_peer_setup_mr_reg() */
struct rpma_peer_setup_mr_reg_args {
	int usage;
	int access;
	struct ibv_mr *mr;
	int verrno;
};

#endif /* MOCKS_RPMA_PEER_H */
