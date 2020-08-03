/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mocks-ibverbs.h -- the ibverbs mocks' header
 */

#ifndef MOCKS_IBVERBS_H
#define MOCKS_IBVERBS_H

#include <infiniband/verbs.h>

int ibv_req_notify_cq_mock(struct ibv_cq *cq, int solicited_only);

#endif /* MOCKS_IBVERBS_H */
