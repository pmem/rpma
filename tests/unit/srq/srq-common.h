/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Fujitsu */

/*
 * srq-common.h -- the rpma_srq unit tests common definitions
 */

#ifndef SRQ_COMMON_H
#define SRQ_COMMON_H

#include <rdma/rdma_cma.h>
#include <stdlib.h>

#include "cmocka_headers.h"
#include "test-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rpma-srq_cfg.h"
#include "mocks-rpma-srq.h"
#include "mocks-rpma-cq.h"
#include "srq.h"

#define MOCK_GET_SRQ_CFG(cstate) \
	((cstate)->get_args.cfg == MOCK_SRQ_CFG_DEFAULT ? NULL : (cstate)->get_args.cfg)
#define MOCK_GET_SRQ_RCQ(cstate) ((cstate)->get_args.rcq_size ? MOCK_RPMA_SRQ_RCQ : NULL)

#define SRQ_NEW_TEST_SETUP_TEARDOWN_WITHOUT_CFG(test_func, setup_func, teardown_func) \
	{#test_func "__without_cfg", (test_func), (setup_func), (teardown_func), \
		&Srq_new_srq_cfg_default}
#define SRQ_NEW_TEST_WITHOUT_CFG(test_func) \
	SRQ_NEW_TEST_SETUP_TEARDOWN_WITHOUT_CFG(test_func, NULL, NULL)

#define SRQ_NEW_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_CFG(test_func, setup_func, teardown_func) \
	{#test_func "__without_cfg", (test_func), (setup_func), (teardown_func), \
		&Srq_new_srq_cfg_default}, \
	{#test_func "__with_cfg", (test_func), (setup_func), (teardown_func), \
		&Srq_new_srq_cfg_custom}
#define SRQ_NEW_TEST_WITH_AND_WITHOUT_CFG(test_func) \
	SRQ_NEW_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_CFG(test_func, NULL, NULL)

struct srq_test_state {
	struct srq_cfg_get_mock_args get_args;
	struct rpma_srq *srq;
};

extern struct srq_test_state Srq_new_srq_cfg_default;
extern struct srq_test_state Srq_new_srq_cfg_custom;

int setup__srq_new(void **srq_ptr);
int teardown__srq_delete(void **srq_ptr);

#endif /* SRQ_COMMON_H */
