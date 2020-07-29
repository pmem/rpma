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

/* mocked IBV entities */
extern struct ibv_comp_channel Ibv_comp_channel;
extern struct ibv_context Ibv_context;
extern struct ibv_cq Ibv_cq;

/* random values or pointers to mocked IBV entities */
#define MOCK_VERBS		(struct ibv_context *)0x4E4B
#define MOCK_COMP_CHANNEL	(struct ibv_comp_channel *)&Ibv_comp_channel
#define MOCK_IBV_CQ		(struct ibv_cq *)&Ibv_cq
#define MOCK_QP			(struct ibv_qp *)0xC41D

int ibv_req_notify_cq_mock(struct ibv_cq *cq, int solicited_only);

#endif /* MOCKS_IBVERBS_H */
