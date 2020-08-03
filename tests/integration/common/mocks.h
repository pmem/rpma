/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mocks.h -- common mocks for integration tests
 */

#include <rdma/rdma_cma.h>
#include <stdio.h>

#define MOCK_ADDR		"127.0.0.1"
#define MOCK_SERVICE		"1234"
#define MOCK_MR			(&Ibv_mr)
#define MOCK_MR_RAW		(&Ibv_mr_raw)
#define MOCK_RAW_SIZE		8 /* memory region size */
#define MOCK_COMP_CHANNEL	(&Ibv_comp_channel)
#define MOCK_CQ			(&Ibv_cq)
#define MOCK_VERBS		(&Ibv_context)
#define MOCK_EVCH		((struct rdma_event_channel *)0xE4C4)
#define MOCK_SRC_ADDR		((struct sockaddr *)0x0ADD)
#define MOCK_DST_ADDR		((struct sockaddr *)0x0ADE)
#define MOCK_IBV_PD		((struct ibv_pd *)0x00D0)
#define MOCK_QP			((struct ibv_qp *)0xD56A)
#define MOCK_OP_CONTEXT		((void *)0xC417)
#define MOCK_RKEY		((uint32_t)0x10111213)
#define MOCK_TIMEOUT		1000 /* RPMA_DEFAULT_TIMEOUT */
#define MOCK_DEFAULT_Q_SIZE	10 /* RPMA_DEFAULT_Q_SIZE */
#define MOCK_MAX_SGE		1 /* RPMA_MAX_SGE */
#define MOCK_MAX_INLINE_DATA	0 /* RPMA_MAX_INLINE_DATA */
#define MOCK_SIZE		1024
#define MOCK_OK			0

/* mock IBV completion channel */
extern struct ibv_comp_channel Ibv_comp_channel;
extern struct ibv_context Ibv_context;	/* mock IBV context */
extern struct ibv_cq Ibv_cq;		/* mock IBV CQ */
extern struct ibv_mr Ibv_mr;		/* mock IBV MR */
extern struct ibv_mr Ibv_mr_raw;	/* mock IBV MR RAW */

struct posix_memalign_args {
	void *ptr;
};

int ibv_post_send_mock(struct ibv_qp *qp, struct ibv_send_wr *wr,
			struct ibv_send_wr **bad_wr);
int ibv_poll_cq_mock(struct ibv_cq *cq, int num_entries, struct ibv_wc *wc);

int ibv_req_notify_cq_mock(struct ibv_cq *cq, int solicited_only);

void *__real__test_malloc(size_t size);
void *__wrap__test_malloc(size_t size);
int __wrap_fprintf(FILE *__restrict __stream,
		const char *__restrict __format, ...);
