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
#define MOCK_CQ			(&Ibv_cq)
#define MOCK_VERBS		((struct ibv_context *)0xABC1)
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

extern struct ibv_cq Ibv_cq;	/* mock IBV CQ */
extern struct ibv_mr Ibv_mr;	/* mock IBV MR */

struct posix_memalign_args {
	void *ptr;
};

int ibv_post_send_mock(struct ibv_qp *qp, struct ibv_send_wr *wr,
			struct ibv_send_wr **bad_wr);
int ibv_poll_cq_mock(struct ibv_cq *cq, int num_entries, struct ibv_wc *wc);

void *__real__test_malloc(size_t size);
void *__wrap__test_malloc(size_t size);
int __wrap_fprintf(FILE *__restrict __stream,
		const char *__restrict __format, ...);
