// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */
/* Copyright 2021-2022, Fujitsu */

/*
 * conn_req-common.c -- the conn_req unit tests common functions
 */

#include "conn_req-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"
#include "mocks-rpma-conn_cfg.h"
#include "mocks-rpma-srq.h"

const char Private_data[] = "Random data";

struct conn_req_new_test_state Conn_req_new_conn_cfg_default = {
	.get_args.cfg = MOCK_CONN_CFG_DEFAULT,
	.get_args.timeout_ms = RPMA_DEFAULT_TIMEOUT_MS,
	.get_args.cq_size = MOCK_CQ_SIZE_DEFAULT,
	.get_args.rcq_size = MOCK_RCQ_SIZE_DEFAULT,
	.get_args.shared = MOCK_SHARED_DEFAULT,
	.get_args.srq = NULL,
	.get_args.srq_rcq = NULL
};

struct conn_req_new_test_state Conn_req_new_conn_cfg_custom = {
	.get_args.cfg = MOCK_CONN_CFG_CUSTOM,
	.get_args.timeout_ms = MOCK_TIMEOUT_MS_CUSTOM,
	.get_args.cq_size = MOCK_CQ_SIZE_CUSTOM,
	.get_args.rcq_size = MOCK_RCQ_SIZE_CUSTOM,
	.get_args.shared = MOCK_SHARED_CUSTOM,
	.get_args.srq = NULL,
	.get_args.srq_rcq = NULL
};

struct conn_req_new_test_state Conn_req_new_conn_cfg_custom_without_srq_rcq = {
	.get_args.cfg = MOCK_CONN_CFG_CUSTOM,
	.get_args.timeout_ms = MOCK_TIMEOUT_MS_CUSTOM,
	.get_args.cq_size = MOCK_CQ_SIZE_CUSTOM,
	.get_args.rcq_size = MOCK_RCQ_SIZE_CUSTOM,
	.get_args.shared = MOCK_SHARED_CUSTOM,
	.get_args.srq = MOCK_RPMA_SRQ,
	.get_args.srq_rcq = NULL
};

struct conn_req_new_test_state Conn_req_new_conn_cfg_default_with_srq_rcq = {
	.get_args.cfg = MOCK_CONN_CFG_DEFAULT,
	.get_args.timeout_ms = RPMA_DEFAULT_TIMEOUT_MS,
	.get_args.cq_size = MOCK_CQ_SIZE_DEFAULT,
	.get_args.rcq_size = MOCK_RCQ_SIZE_DEFAULT,
	.get_args.shared = MOCK_SHARED_DEFAULT,
	.get_args.srq = MOCK_RPMA_SRQ,
	.get_args.srq_rcq = MOCK_RPMA_SRQ_RCQ
};

struct conn_req_test_state Conn_req_conn_cfg_default = {
	.get_args.cfg = MOCK_CONN_CFG_DEFAULT,
	.get_args.cq_size = MOCK_CQ_SIZE_DEFAULT,
	.get_args.rcq_size = MOCK_RCQ_SIZE_DEFAULT,
	.get_args.shared = MOCK_SHARED_DEFAULT,
	.get_args.srq = NULL,
	.get_args.srq_rcq = NULL
};

struct conn_req_test_state Conn_req_conn_cfg_custom = {
	.get_args.cfg = MOCK_CONN_CFG_CUSTOM,
	.get_args.cq_size = MOCK_CQ_SIZE_CUSTOM,
	.get_args.rcq_size = MOCK_RCQ_SIZE_CUSTOM,
	.get_args.shared = MOCK_SHARED_CUSTOM,
	.get_args.srq = NULL,
	.get_args.srq_rcq = NULL
};

struct conn_req_test_state Conn_req_conn_cfg_custom_without_srq_rcq = {
	.get_args.cfg = MOCK_CONN_CFG_CUSTOM,
	.get_args.cq_size = MOCK_CQ_SIZE_CUSTOM,
	.get_args.rcq_size = MOCK_RCQ_SIZE_CUSTOM,
	.get_args.shared = MOCK_SHARED_CUSTOM,
	.get_args.srq = MOCK_RPMA_SRQ,
	.get_args.srq_rcq = NULL
};

struct conn_req_test_state Conn_req_conn_cfg_default_with_srq_rcq = {
	.get_args.cfg = MOCK_CONN_CFG_DEFAULT,
	.get_args.cq_size = MOCK_CQ_SIZE_DEFAULT,
	.get_args.rcq_size = MOCK_RCQ_SIZE_DEFAULT,
	.get_args.shared = MOCK_SHARED_DEFAULT,
	.get_args.srq = MOCK_RPMA_SRQ,
	.get_args.srq_rcq = MOCK_RPMA_SRQ_RCQ
};

/*
 * configure_conn_req_new -- configure prestate for rpma_conn_req_new()
 */
void
configure_conn_req_new(void **cstate_ptr)
{
	/* the default is Conn_req_new_conn_cfg_default */
	struct conn_req_new_test_state *cstate = *cstate_ptr ?
			*cstate_ptr : &Conn_req_new_conn_cfg_default;

	cstate->id.verbs = MOCK_VERBS;
	cstate->id.qp = MOCK_QP;
	cstate->id.route.path_rec = MOCK_PATH_REC;

	*cstate_ptr = cstate;
}

/*
 * configure_conn_req -- configure prestate for rpma_conn_req_connect()
 */
void
configure_conn_req(void **cstate_ptr)
{
	/* the default is Conn_req_conn_cfg_default */
	struct conn_req_test_state *cstate = *cstate_ptr ?
			*cstate_ptr : &Conn_req_conn_cfg_default;

	cstate->event.event = RDMA_CM_EVENT_CONNECT_REQUEST;
	cstate->id.verbs = MOCK_VERBS;
	cstate->id.route.path_rec = MOCK_PATH_REC;
	cstate->event.id = &cstate->id;

	*cstate_ptr = cstate;
}

/*
 * setup__conn_req_new_from_cm_event -- prepare a valid rpma_conn_req object from CM
 * event
 */
int
setup__conn_req_new_from_cm_event(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;
	configure_conn_req((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_cqe, &cstate->get_args);
	will_return(rpma_conn_cfg_get_rcqe, &cstate->get_args);
	will_return(rpma_conn_cfg_get_compl_channel, &cstate->get_args);
	will_return(rpma_conn_cfg_get_srq, &cstate->get_args);
	if (cstate->get_args.srq) {
		expect_value(rpma_srq_get_rcq, srq, MOCK_RPMA_SRQ);
		will_return(rpma_srq_get_rcq, cstate->get_args.srq_rcq);
	}
	if (!cstate->get_args.srq_rcq && cstate->get_args.shared)
		will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	expect_value(rpma_cq_new, cqe, cstate->get_args.cq_size);
	expect_value(rpma_cq_new, shared_channel, MOCK_GET_CHANNEL(cstate));
	will_return(rpma_cq_new, MOCK_RPMA_CQ);
	if (!cstate->get_args.srq_rcq && cstate->get_args.rcq_size) {
		expect_value(rpma_cq_new, cqe, cstate->get_args.rcq_size);
		expect_value(rpma_cq_new, shared_channel,
				MOCK_GET_CHANNEL(cstate));
		will_return(rpma_cq_new, MOCK_RPMA_RCQ);
	}
	expect_value(rpma_peer_setup_qp, id, &cstate->id);
	expect_value(rpma_peer_setup_qp, cfg, cstate->get_args.cfg);
	expect_value(rpma_peer_setup_qp, rcq, MOCK_GET_RCQ(cstate));
	will_return(rpma_peer_setup_qp, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_OK);
	will_return_maybe(__wrap_snprintf, MOCK_OK);
	will_return(rpma_private_data_store, MOCK_PRIVATE_DATA);

	/* run test */
	int ret = rpma_conn_req_new_from_cm_event(MOCK_PEER, &cstate->event,
			cstate->get_args.cfg, &cstate->req);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(cstate->req);

	*cstate_ptr = cstate;

	return 0;
}

/*
 * teardown__conn_req_new_from_cm_event -- delete the rpma_conn_req object created
 * from a CM event
 */
int
teardown__conn_req_new_from_cm_event(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ_DEL(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, MOCK_OK);
	if (!cstate->get_args.srq_rcq && cstate->get_args.shared)
		will_return(ibv_destroy_comp_channel, MOCK_OK);
	expect_function_call(rpma_private_data_delete);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cstate->req);

	*cstate_ptr = NULL;

	return 0;
}

/*
 * setup__conn_req_new -- prepare a new outgoing rpma_conn_req
 */
int
setup__conn_req_new(void **cstate_ptr)
{
	struct conn_req_new_test_state *cstate = *cstate_ptr;
	configure_conn_req_new((void **)&cstate);

	/* configure mocks for rpma_conn_req_new() */
	Mock_ctrl_defer_destruction = MOCK_CTRL_DEFER;
	will_return(rpma_conn_cfg_get_timeout, &cstate->get_args);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &cstate->id);
	expect_value(rpma_info_resolve_addr, id, &cstate->id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
			cstate->get_args.timeout_ms);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	expect_value(rdma_resolve_route, timeout_ms, cstate->get_args.timeout_ms);
	will_return(rdma_resolve_route, MOCK_OK);
	will_return(rpma_conn_cfg_get_cqe, &cstate->get_args);
	will_return(rpma_conn_cfg_get_rcqe, &cstate->get_args);
	will_return(rpma_conn_cfg_get_compl_channel, &cstate->get_args);
	will_return(rpma_conn_cfg_get_srq, &cstate->get_args);
	if (cstate->get_args.srq) {
		expect_value(rpma_srq_get_rcq, srq, MOCK_RPMA_SRQ);
		will_return(rpma_srq_get_rcq, cstate->get_args.srq_rcq);
	}
	if (!cstate->get_args.srq_rcq && cstate->get_args.shared)
		will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	expect_value(rpma_cq_new, cqe, cstate->get_args.cq_size);
	expect_value(rpma_cq_new, shared_channel, MOCK_GET_CHANNEL(cstate));
	will_return(rpma_cq_new, MOCK_RPMA_CQ);
	if (!cstate->get_args.srq_rcq && cstate->get_args.rcq_size) {
		expect_value(rpma_cq_new, cqe, cstate->get_args.rcq_size);
		expect_value(rpma_cq_new, shared_channel,
				MOCK_GET_CHANNEL(cstate));
		will_return(rpma_cq_new, MOCK_RPMA_RCQ);
	}
	expect_value(rpma_peer_setup_qp, id, &cstate->id);
	expect_value(rpma_peer_setup_qp, cfg, cstate->get_args.cfg);
	expect_value(rpma_peer_setup_qp, rcq, MOCK_GET_RCQ(cstate));
	will_return(rpma_peer_setup_qp, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_OK);
	will_return_maybe(__wrap_snprintf, MOCK_STDIO_ERROR);

	/* run test */
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT,
			MOCK_GET_CONN_CFG(cstate), &cstate->req);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(cstate->req);

	*cstate_ptr = cstate;

	/* restore default mock configuration */
	Mock_ctrl_defer_destruction = MOCK_CTRL_NO_DEFER;

	return 0;
}

/*
 * teardown__conn_req_new -- delete the outgoing rpma_conn_req object
 */
int
teardown__conn_req_new(void **cstate_ptr)
{
	struct conn_req_new_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ_DEL(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, 0);
	if (!cstate->get_args.srq_rcq && cstate->get_args.shared)
		will_return(ibv_destroy_comp_channel, MOCK_OK);
	expect_function_call(rpma_private_data_delete);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cstate->req);

	*cstate_ptr = NULL;

	return 0;
}
