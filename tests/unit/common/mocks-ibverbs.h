/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * mocks-ibverbs.h -- the ibverbs mocks' header
 */

#ifndef MOCKS_IBVERBS_H
#define MOCKS_IBVERBS_H

#include <infiniband/verbs.h>

/* mocked IBV entities */
extern struct verbs_context Verbs_context;
extern struct ibv_comp_channel Ibv_comp_channel;
extern struct ibv_cq Ibv_cq;
extern struct ibv_qp Ibv_qp;
extern struct ibv_mr Ibv_mr;

/* random values or pointers to mocked IBV entities */
#define MOCK_VERBS		(&Verbs_context.context)
#define MOCK_COMP_CHANNEL	(struct ibv_comp_channel *)&Ibv_comp_channel
#define MOCK_IBV_CQ		(struct ibv_cq *)&Ibv_cq
#define MOCK_IBV_PD		(struct ibv_pd *)0x00D0
#define MOCK_QP			(struct ibv_qp *)&Ibv_qp
#define MOCK_MR			(struct ibv_mr *)&Ibv_mr

struct ibv_alloc_pd_mock_args {
	int validate_params;
	struct ibv_pd *pd;
};

struct ibv_dealloc_pd_mock_args {
	int validate_params;
	int ret;
};

struct ibv_post_send_mock_args {
	struct ibv_qp *qp;
	enum ibv_wr_opcode opcode;
	unsigned send_flags;
	uint64_t wr_id;
	uint32_t imm_data;
	int ret;
};

struct ibv_post_recv_mock_args {
	struct ibv_qp *qp;
	uint64_t wr_id;
	int ret;
};

#ifdef ON_DEMAND_PAGING_SUPPORTED
int ibv_query_device_ex_mock(struct ibv_context *context,
		const struct ibv_query_device_ex_input *input,
		struct ibv_device_attr_ex *attr,
		size_t attr_size);
#endif

int ibv_post_send_mock(struct ibv_qp *qp, struct ibv_send_wr *wr,
		struct ibv_send_wr **bad_wr);

int ibv_post_recv_mock(struct ibv_qp *qp, struct ibv_recv_wr *wr,
			struct ibv_recv_wr **bad_wr);

int ibv_req_notify_cq_mock(struct ibv_cq *cq, int solicited_only);

#endif /* MOCKS_IBVERBS_H */
