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
	struct conn_cfg_get_timeout_mock_args get_t;
	struct conn_cfg_get_cq_size_mock_args get_cqe;

	struct rdma_cm_id id;
	struct rpma_conn_req *req;
};

void prestate_init(struct conn_req_new_test_state *prestate,
		struct rpma_conn_cfg *cfg, int timeout_ms,
		uint32_t cq_size, uint32_t rcq_size);

int setup__conn_req_new(void **cstate_ptr);
int teardown__conn_req_new(void **cstate_ptr);

int group_setup_conn_req(void **unused);

#endif /* CONN_REQ_COMMON */
