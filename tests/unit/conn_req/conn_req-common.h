/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn_req-common.c -- the conn_req unit tests common definitions
 */

#ifndef CONN_REQ_COMMON
#define CONN_REQ_COMMON

#include "cmocka_headers.h"
#include "test-common.h"
#include "conn_req.h"

/*
 * all the resources used between setup__conn_req_from_cm_event and
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
 * all the resources used between setup__conn_req_new and teardown__conn_req_new
 */
struct conn_req_new_test_state {
	struct rdma_cm_id id;
	struct rpma_conn_req *req;
};

int setup__conn_req_new(void **cstate_ptr);
int teardown__conn_req_new(void **cstate_ptr);

int group_setup_connect(void **unused);
int group_setup_delete(void **unused);
int group_setup_from_cm_event(void **unused);
int group_setup_new(void **unused);
int group_setup_conn_req(void **unused);

#endif /* CONN_REQ_COMMON */
