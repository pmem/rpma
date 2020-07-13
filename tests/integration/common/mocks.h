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
#define MOCK_VERBS		((struct ibv_context *)0xABC1)
#define MOCK_CQ			(struct ibv_cq *)0x00C0
#define MOCK_EVCH		(struct rdma_event_channel *)0xE4C4
#define MOCK_SRC_ADDR		(struct sockaddr *)0x0ADD
#define MOCK_DST_ADDR		(struct sockaddr *)0x0ADE
#define MOCK_IBV_PD		(struct ibv_pd *)0x00D0
#define MOCK_MR			(struct ibv_mr *)0x0AD5
#define MOCK_TIMEOUT		1000 /* RPMA_DEFAULT_TIMEOUT */
#define MOCK_DEFAULT_Q_SIZE	10 /* RPMA_DEFAULT_Q_SIZE */
#define MOCK_MAX_SGE		1 /* RPMA_MAX_SGE */
#define MOCK_MAX_INLINE_DATA	0 /* RPMA_MAX_INLINE_DATA */
#define MOCK_OK			0

void *__real__test_malloc(size_t size);
void *__wrap__test_malloc(size_t size);
int __wrap_fprintf(FILE *__restrict __stream,
		const char *__restrict __format, ...);
