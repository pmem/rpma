/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2021, Fujitsu */

/*
 * cq-common.h -- the completion queue unit tests common definitions
 */

#ifndef CQ_COMMON
#define CQ_COMMON

#include "cq.h"
#include "test-common.h"

#define MOCK_WC_STATUS		(int)0x51A5

int setup__cq_create(void **cq_ptr);
int teardown__cq_destroy(void **cq_ptr);

#endif /* CQ_COMMON */
