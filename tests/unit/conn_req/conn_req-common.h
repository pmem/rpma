/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * conn_req-common.c -- the conn_req unit tests common definitions
 */

#ifndef CONN_REQ_COMMON
#define CONN_REQ_COMMON

#include "cmocka_headers.h"
#include "test-common.h"
#include "conn_req.h"

#define MOCK_CONN_REQ		(struct rpma_conn_req *)0xC410
#define MOCK_RPMA_MR_LOCAL	(struct rpma_mr_local *)0xC411
#define MOCK_LOCAL_OFFSET	(size_t)0xC413
#define MOCK_LEN		(size_t)0xC415
#define MOCK_FLAGS		(int)0xC416
#define MOCK_OP_CONTEXT		(void *)0xC417

/*
 * All the resources used between setup__conn_req_from_cm_event and
 * teardown__conn_req_from_cm_event
 */
struct conn_req_test_state {
	struct rdma_cm_event event;
	struct rdma_cm_id id;
	struct rpma_conn_req *req;
};

int setup__conn_req_from_cm_event(void **cstate_ptr);
int teardown__conn_req_from_cm_event(void **cstate_ptr);

/*
 * All the resources used between setup__conn_req_new and teardown__conn_req_new
 */
struct conn_req_new_test_state {
	struct rdma_cm_id id;
	struct rpma_conn_req *req;
};

int setup__conn_req_new(void **cstate_ptr);
int teardown__conn_req_new(void **cstate_ptr);

int group_setup_conn_req(void **unused);

#endif /* CONN_REQ_COMMON */
