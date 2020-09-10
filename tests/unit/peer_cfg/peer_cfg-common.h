/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * peer_cfg-common.h -- the peer_cfg unit tests common definitions
 */

#ifndef PEER_CFG_COMMON
#define PEER_CFG_COMMON

#include "cmocka_headers.h"
#include "test-common.h"
#include "conn_req.h"

#define MOCK_PEER_PCFG_PTR	((struct rpma_peer_cfg **)0xA1D1)
#define MOCK_PEER_PCFG		((struct rpma_peer_cfg *)0xA1D2)
#define MOCK_SUPPORTED		false

/*
 * All the resources used between setup__peer_cfg_new and teardown__peer_cfg_new
 */
struct peer_cfg_test_state {
	struct rpma_peer_cfg *cfg;
};

int setup__peer_cfg(void **cstate_ptr);
int teardown__peer_cfg(void **cstate_ptr);

#endif /* PEER_CFG_COMMON */
