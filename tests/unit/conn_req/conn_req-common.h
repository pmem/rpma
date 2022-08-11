/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2022, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn_req-common.h -- the conn_req unit tests common definitions
 */

#ifndef CONN_REQ_COMMON
#define CONN_REQ_COMMON

#include "cmocka_headers.h"
#include "conn_req.h"
#include "mocks-ibverbs.h"
#include "mocks-rpma-conn_cfg.h"
#include "mocks-rpma-cq.h"
#include "mocks-rpma-srq.h"
#include "mocks-stdio.h"

#define DEFAULT_VALUE	"The default one"
#define DEFAULT_LEN	(strlen(DEFAULT_VALUE) + 1)

#define MOCK_CONN_REQ		(struct rpma_conn_req *)0xC410
#define MOCK_GET_RCQ(cstate) \
	((cstate)->get_args.srq_rcq ? MOCK_RPMA_SRQ_RCQ : \
			((cstate)->get_args.rcq_size ? MOCK_RPMA_RCQ : NULL))
#define MOCK_GET_RCQ_DEL(cstate) \
	((!(cstate)->get_args.srq_rcq && (cstate)->get_args.rcq_size) ? MOCK_RPMA_RCQ : NULL)
#define MOCK_GET_CHANNEL(cstate) \
	((!(cstate)->get_args.srq_rcq && (cstate)->get_args.shared) ? \
			MOCK_COMP_CHANNEL : NULL)
#define MOCK_GET_CONN_CFG(cstate) \
	((cstate)->get_args.cfg == MOCK_CONN_CFG_DEFAULT ? \
			NULL : (cstate)->get_args.cfg)

#define CONN_REQ_NEW_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_RCQ(test_func, \
		setup_func, teardown_func) \
	{#test_func "__without_rcq", (test_func), (setup_func), \
		(teardown_func), &Conn_req_new_conn_cfg_default}, \
	{#test_func "__with_rcq", (test_func), (setup_func), \
		(teardown_func), &Conn_req_new_conn_cfg_custom}
#define CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_RCQ(test_func) \
	CONN_REQ_NEW_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_RCQ(test_func, \
		NULL, NULL)

#define CONN_REQ_NEW_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_SRQ_RCQ(test_func, \
		setup_func, teardown_func) \
	{#test_func "__without_srq_rcq", (test_func), (setup_func), (teardown_func), \
		&Conn_req_new_conn_cfg_custom_without_srq_rcq}, \
	{#test_func "__with_srq_rcq", (test_func), (setup_func), (teardown_func), \
		&Conn_req_new_conn_cfg_default_with_srq_rcq}
#define CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_SRQ_RCQ(test_func) \
	CONN_REQ_NEW_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_SRQ_RCQ(test_func, NULL, NULL)

#define CONN_REQ_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_RCQ(test_func, \
		setup_func, teardown_func) \
	{#test_func "__without_rcq", (test_func), (setup_func), \
		(teardown_func), &Conn_req_conn_cfg_default}, \
	{#test_func "__with_rcq", (test_func), (setup_func), \
		(teardown_func), &Conn_req_conn_cfg_custom}
#define CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(test_func) \
	CONN_REQ_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_RCQ(test_func, \
		NULL, NULL)

#define CONN_REQ_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_SRQ_RCQ(test_func, \
		setup_func, teardown_func) \
	{#test_func "__without_srq_rcq", (test_func), (setup_func), (teardown_func), \
		&Conn_req_conn_cfg_custom_without_srq_rcq}, \
	{#test_func "__with_rcq", (test_func), (setup_func), (teardown_func), \
		&Conn_req_conn_cfg_default_with_srq_rcq}
#define CONN_REQ_TEST_WITH_AND_WITHOUT_SRQ_RCQ(test_func) \
	CONN_REQ_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_SRQ_RCQ(test_func, NULL, NULL)

/*
 * All the resources used between setup__conn_req_new_from_cm_event and
 * teardown__conn_req_new_from_cm_event
 */
struct conn_req_test_state {
	struct conn_cfg_get_mock_args get_args;

	struct rdma_cm_event event;
	struct rdma_cm_id id;
	struct rpma_conn_req *req;
};

extern struct conn_req_test_state Conn_req_conn_cfg_default;
extern struct conn_req_test_state Conn_req_conn_cfg_custom;
extern struct conn_req_test_state Conn_req_conn_cfg_custom_without_srq_rcq;
extern struct conn_req_test_state Conn_req_conn_cfg_default_with_srq_rcq;

int setup__conn_req_new_from_cm_event(void **cstate_ptr);
int teardown__conn_req_new_from_cm_event(void **cstate_ptr);

/*
 * All the resources used between setup__conn_req_new and teardown__conn_req_new
 */
struct conn_req_new_test_state {
	struct conn_cfg_get_mock_args get_args;

	struct rdma_cm_id id;
	struct rpma_conn_req *req;
};

extern struct conn_req_new_test_state Conn_req_new_conn_cfg_default;
extern struct conn_req_new_test_state Conn_req_new_conn_cfg_custom;
extern struct conn_req_new_test_state Conn_req_new_conn_cfg_custom_without_srq_rcq;
extern struct conn_req_new_test_state Conn_req_new_conn_cfg_default_with_srq_rcq;

int setup__conn_req_new(void **cstate_ptr);
int teardown__conn_req_new(void **cstate_ptr);

void configure_conn_req_new(void **cstate_ptr);
void configure_conn_req(void **cstate_ptr);

#endif /* CONN_REQ_COMMON */
