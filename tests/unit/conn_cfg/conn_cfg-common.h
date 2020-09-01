/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * conn_cfg-common.h -- the conn_cfg unit tests common definitions
 */

#ifndef CONN_CFG_COMMON
#define CONN_CFG_COMMON

#include "cmocka_headers.h"
#include "test-common.h"
#include "conn_req.h"

/*
 * All the resources used between setup__conn_cfg_new and teardown__conn_cfg_new
 */
struct conn_cfg_test_state {
	struct rpma_conn_cfg *cfg;
};

int setup__conn_cfg(void **cstate_ptr);
int teardown__conn_cfg(void **cstate_ptr);

#endif /* CONN_CFG_COMMON */
