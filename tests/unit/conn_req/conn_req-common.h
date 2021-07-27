/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn_req-common.c -- the conn_req unit tests common definitions
 */

#ifndef CONN_REQ_COMMON
#define CONN_REQ_COMMON

#include "cmocka_headers.h"
#include "conn_req.h"
#include "mocks-rpma-conn_cfg.h"
#include "mocks-rpma-cq.h"

#define MOCK_CONN_REQ		(struct rpma_conn_req *)0xC410
#define MOCK_GET_RCQ(cstate) \
	((cstate)->get_cqe.rcq_size ? MOCK_RPMA_RCQ : NULL)
#define MOCK_GET_CONN_CFG(cstate) \
	((cstate)->get_cqe.cfg == MOCK_CONN_CFG_DEFAULT ? \
			NULL : (cstate)->get_cqe.cfg)
#define CONN_REQ_NEW_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_RCQ(test_func, \
		setup_func, teardown_func) \
	{#test_func "__without_rcq", (test_func), (setup_func), \
		(teardown_func), &Conn_req_new_conn_cfg_default}, \
	{#test_func "__with_rcq", (test_func), (setup_func), \
		(teardown_func), &Conn_req_new_conn_cfg_custom}
#define CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_RCQ(test_func) \
	CONN_REQ_NEW_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_RCQ(test_func, \
		NULL, NULL)
#define CONN_REQ_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_RCQ(test_func, \
		setup_func, teardown_func) \
	{#test_func "__without_rcq", (test_func), (setup_func), \
		(teardown_func), &Conn_req_conn_cfg_default}, \
	{#test_func "__with_rcq", (test_func), (setup_func), \
		(teardown_func), &Conn_req_conn_cfg_custom}
#define CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(test_func) \
	CONN_REQ_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_RCQ(test_func, \
		NULL, NULL)

/*
 * All the resources used between setup__conn_req_from_cm_event and
 * teardown__conn_req_from_cm_event
 */
struct conn_req_test_state {
	struct conn_cfg_get_cq_size_mock_args get_cqe;

	struct rdma_cm_event event;
	struct rdma_cm_id id;
	struct rpma_conn_req *req;
};

extern struct conn_req_test_state Conn_req_conn_cfg_default;
extern struct conn_req_test_state Conn_req_conn_cfg_custom;

int setup__conn_req_from_cm_event(void **cstate_ptr);
int teardown__conn_req_from_cm_event(void **cstate_ptr);

/*
 * All the resources used between setup__conn_req_new and teardown__conn_req_new
 */
struct conn_req_new_test_state {
	struct conn_cfg_get_timeout_mock_args get_t;
	struct conn_cfg_get_cq_size_mock_args get_cqe;

	struct rdma_cm_id id;
	struct rpma_conn_req *req;
};

extern struct conn_req_new_test_state Conn_req_new_conn_cfg_default;
extern struct conn_req_new_test_state Conn_req_new_conn_cfg_custom;

int setup__conn_req_new(void **cstate_ptr);
int teardown__conn_req_new(void **cstate_ptr);

void configure_conn_req_new(void **cstate_ptr);
void configure_conn_req(void **cstate_ptr);

#endif /* CONN_REQ_COMMON */
