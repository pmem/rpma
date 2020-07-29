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
extern struct ibv_qp Ibv_qp;
extern struct ibv_mr Ibv_mr;

/* random values or pointers to mocked IBV entities */
#define MOCK_VERBS		(struct ibv_context *)0x4E4B
#define MOCK_COMP_CHANNEL	(struct ibv_comp_channel *)&Ibv_comp_channel
#define MOCK_IBV_CQ		(struct ibv_cq *)&Ibv_cq
#define MOCK_QP			(struct ibv_qp *)&Ibv_qp
#define MOCK_MR			(struct ibv_mr *)&Ibv_mr

struct ibv_post_send_mock_args {
	struct ibv_qp *qp;
	enum ibv_wr_opcode opcode;
	unsigned send_flags;
	uint64_t wr_id;
	int ret;
};

int ibv_post_send_mock(struct ibv_qp *qp, struct ibv_send_wr *wr,
		struct ibv_send_wr **bad_wr);

int ibv_req_notify_cq_mock(struct ibv_cq *cq, int solicited_only);

#endif /* MOCKS_IBVERBS_H */
