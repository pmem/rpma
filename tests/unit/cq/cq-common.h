/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * cq-common.h -- the rpma_cq unit tests common definitions
 */

#ifndef CQ_COMMON
#define CQ_COMMON

#include "test-common.h"
#include "cq.h"

#define MOCK_WC_STATUS_ERROR		(int)0x51A5

/* all the resources used between setup__cq_new and teardown__cq_delete */
struct cq_test_state {
	struct ibv_comp_channel *shared_channel;
	struct rpma_cq *cq;
};

extern struct cq_test_state CQ_without_channel;
extern struct cq_test_state CQ_with_channel;

int setup__cq_new(void **cq_ptr);
int teardown__cq_delete(void **cq_ptr);
int group_setup_common_cq(void **unused);

#endif /* CQ_COMMON */
