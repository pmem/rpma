/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mocks.c -- common mocks for integration tests
 */

#include <rdma/rdma_cma.h>
#include <stdlib.h>
#include <stdio.h>

#include "librpma.h"
#include "cmocka_headers.h"

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

int ibv_dereg_mr(struct ibv_mr *mr);
int rdma_create_id(struct rdma_event_channel *channel,
		struct rdma_cm_id **id, void *context,
		enum rdma_port_space ps);
int rdma_destroy_id(struct rdma_cm_id *id);
struct ibv_pd *ibv_alloc_pd(struct ibv_context *ibv_ctx);
int ibv_dealloc_pd(struct ibv_pd *pd);
struct ibv_cq *ibv_create_cq(struct ibv_context *context, int cqe,
		void *cq_context, struct ibv_comp_channel *channel,
		int comp_vector);
int ibv_destroy_cq(struct ibv_cq *cq);
int rdma_accept(struct rdma_cm_id *id, struct rdma_conn_param *conn_param);
int rdma_connect(struct rdma_cm_id *id, struct rdma_conn_param *conn_param);
int rdma_disconnect(struct rdma_cm_id *id);
int rdma_resolve_route(struct rdma_cm_id *id, int timeout_ms);
int rdma_get_cm_event(struct rdma_event_channel *channel,
		struct rdma_cm_event **event_ptr);
int rdma_ack_cm_event(struct rdma_cm_event *event);
struct rdma_event_channel *rdma_create_event_channel(void);
void rdma_destroy_event_channel(struct rdma_event_channel *channel);
int rdma_create_qp(struct rdma_cm_id *id, struct ibv_pd *pd,
		struct ibv_qp_init_attr *qp_init_attr);
void rdma_destroy_qp(struct rdma_cm_id *id);

#if defined(ibv_reg_mr)
/*
 * Since rdma-core v27.0-105-g5a750676
 * ibv_reg_mr() has been defined as a macro
 * in <infiniband/verbs.h>:
 *
 * https://github.com/linux-rdma/rdma-core/commit/5a750676e8312715100900c6336bbc98577e082b
 *
 * In order to mock the ibv_reg_mr() function
 * the `ibv_reg_mr` symbol has to be undefined first
 * and the additional ibv_reg_mr_iova2() function
 * has to be mocked, because it is called
 * by the 'ibv_reg_mr' macro.
 */
#undef ibv_reg_mr
#define IS_DEFINED_ibv_reg_mr_iova2 1
struct ibv_mr *ibv_reg_mr_iova2(struct ibv_pd *pd, void *addr, size_t length,
			uint64_t iova, unsigned access);
#endif

struct ibv_mr *ibv_reg_mr(struct ibv_pd *pd, void *addr, size_t length,
		int access);
int rdma_migrate_id(struct rdma_cm_id *id, struct rdma_event_channel *channel);
int rdma_reject(struct rdma_cm_id *id, const void *private_data,
		uint8_t private_data_len);
int rdma_listen(struct rdma_cm_id *id, int backlog);
int rdma_getaddrinfo(const char *node, const char *service,
		const struct rdma_addrinfo *hints, struct rdma_addrinfo **res);
void rdma_freeaddrinfo(struct rdma_addrinfo *res);
int rdma_resolve_addr(struct rdma_cm_id *id, struct sockaddr *src_addr,
		struct sockaddr *dst_addr, int timeout_ms);
int rdma_bind_addr(struct rdma_cm_id *id, struct sockaddr *addr);
void *__real__test_malloc(size_t size);
void *__wrap__test_malloc(size_t size);
int __wrap_fprintf(FILE *__restrict __stream,
		const char *__restrict __format, ...);
