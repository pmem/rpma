/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2021, Fujitsu */

/*
 * cq-common.h -- the rpma_cq unit tests common definitions
 */

#ifndef CQ_COMMON
#define CQ_COMMON

#include "test-common.h"
#include "cq.h"

#define MOCK_WC_STATUS_ERROR		(int)0x51A5

int setup__cq_new(void **cq_ptr);
int teardown__cq_delete(void **cq_ptr);
int group_setup_common_cq(void **unused);

#endif /* CQ_COMMON */
