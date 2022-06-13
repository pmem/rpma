/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Fujitsu */

/*
 * srq_cfg-common.h -- the srq_cfg unit tests common definitions
 */

#ifndef SRQ_CFG_COMMON
#define SRQ_CFG_COMMON

#include "cmocka_headers.h"
#include "librpma.h"
#include "test-common.h"

/*
 * All the resources used between setup__srq_cfg and teardown__srq_cfg
 */
struct srq_cfg_test_state {
	struct rpma_srq_cfg *cfg;
};

int setup__srq_cfg(void **cstate_ptr);
int teardown__srq_cfg(void **cstate_ptr);

#endif /* SRQ_CFG_COMMON */
