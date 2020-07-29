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
#include "test-common.h"
#include "conn.h"

extern struct rdma_cm_id Cm_id;	/* mock CM ID */
extern struct rdma_event_channel Evch; /* mock event channel */

#define MOCK_EVCH		(struct rdma_event_channel *)&Evch
#define MOCK_CM_ID		(struct rdma_cm_id *)&Cm_id
#define MOCK_RPMA_MR_LOCAL	(struct rpma_mr_local *)0xC411
#define MOCK_RPMA_MR_REMOTE	(struct rpma_mr_remote *)0xC412
#define MOCK_LOCAL_OFFSET	(size_t)0xC413
#define MOCK_REMOTE_OFFSET	(size_t)0xC414
#define MOCK_LEN		(size_t)0xC415
#define MOCK_FLAGS		(int)0xC416
#define MOCK_OP_CONTEXT		(void *)0xC417
#define MOCK_WC_STATUS		(int)0x51A5
#define MOCK_FD			0x00FD
#define MOCK_COMPLETION_FD	0x00FE

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

int group_setup_get_completion_fd(void **unused);
int group_setup_get_event_fd(void **unused);
int group_setup_next_completion(void **unused);
int group_setup_read(void **unused);
int group_setup_write(void **unused);

#endif /* CONN_COMMON */
