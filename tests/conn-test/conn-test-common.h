/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn-test-common.c -- the connection unit tests common definitions
 */

#ifndef CONN_COMMON
#define CONN_COMMON

#include "cmocka_headers.h"
#include "conn.h"

extern struct rdma_cm_id Cm_id;	/* mock CM ID */

#define MOCK_EVCH		(struct rdma_event_channel *)0xE4C4
#define MOCK_CQ			(struct ibv_cq *)0x00C0
#define MOCK_CM_ID		(struct rdma_cm_id *)&Cm_id
#define MOCK_QP			(struct ibv_qp *)0xC41D
#define MOCK_CONN		(struct rpma_conn *)0xA41F
#define MOCK_PRIVATE_DATA	((void *)"Random data")
#define MOCK_PDATA_LEN		(strlen(MOCK_PRIVATE_DATA) + 1)
#define MOCK_PRIVATE_DATA_2	((void *)"Another random data")
#define MOCK_PDATA_LEN_2	(strlen(MOCK_PRIVATE_DATA_2) + 1)

#define MOCK_DST		(struct rpma_mr_local *)0xC411
#define MOCK_SRC		(struct rpma_mr_remote *)0xC412
#define MOCK_DST_OFFSET		(size_t)0xC413
#define MOCK_SRC_OFFSET		(size_t)0xC414
#define MOCK_LEN		(size_t)0xC415
#define MOCK_FLAGS		(int)0xC416
#define MOCK_OP_CONTEXT		(void *)0xC417

#define MOCK_OK			0

extern int Rdma_migrate_id_counter;

#define RDMA_MIGRATE_TO_EVCH 0
#define RDMA_MIGRATE_FROM_EVCH 1
#define RDMA_MIGRATE_COUNTER_INIT (RDMA_MIGRATE_TO_EVCH)

/* all the resources used between conn_setup and conn_teardown */
struct conn_test_state {
	struct rpma_conn *conn;
	struct rpma_conn_private_data data;
};

int conn_setup(void **cstate_ptr);

int conn_teardown(void **cstate_ptr);

#endif /* CONN_COMMON */
