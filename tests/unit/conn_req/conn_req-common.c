// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn_req-common.c -- the conn_req unit tests common functions
 */

#include "conn_req-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"
#include "mocks-rpma-conn_cfg.h"

const char Private_data[] = "Random data";

/*
 * prestate_init -- initialize conn_req_new prestate
 */
void
prestate_init(struct conn_req_new_test_state *prestate,
		struct rpma_conn_cfg *cfg, int timeout_ms,
		uint32_t cq_size, uint32_t rcq_size)
{
	memset(prestate, 0, sizeof(struct conn_req_new_test_state));
	prestate->id.verbs = MOCK_VERBS;
	prestate->id.qp = MOCK_QP;
	prestate->get_t.cfg = cfg;
	prestate->get_t.timeout_ms = timeout_ms;
	prestate->get_cqe.cfg = cfg;
	prestate->get_cqe.cq_size = cq_size;
	prestate->get_cqe.rcq_size = rcq_size;
}

/*
 * setup__conn_req_from_cm_event -- prepare a valid rpma_conn_req object from CM
 * event
 */
int
setup__conn_req_from_cm_event(void **cstate_ptr)
{
	static struct conn_req_test_state cstate = {{0}};
	memset(&cstate, 0, sizeof(cstate));
	cstate.event.event = RDMA_CM_EVENT_CONNECT_REQUEST;
	cstate.event.id = &cstate.id;
	cstate.id.verbs = MOCK_VERBS;
	struct conn_cfg_get_cq_size_mock_args get_cqe = {
			.cfg = MOCK_CONN_CFG_DEFAULT,
			.cq_size = MOCK_CQ_SIZE_DEFAULT,
			.rcq_size = MOCK_RCQ_SIZE_DEFAULT
	};

	/* configure mocks */
	will_return(rpma_conn_cfg_get_cqe, &get_cqe);
	expect_value(rpma_cq_new, cqe, MOCK_CQ_SIZE_DEFAULT);
	will_return(rpma_cq_new, MOCK_RPMA_CQ);
	expect_value(rpma_peer_create_qp, id, &cstate.id);
	expect_value(rpma_peer_create_qp, cfg, MOCK_CONN_CFG_DEFAULT);
	will_return(rpma_peer_create_qp, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_OK);
	will_return(rpma_private_data_store, MOCK_PRIVATE_DATA);

	/* run test */
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &cstate.event,
			MOCK_CONN_CFG_DEFAULT, &cstate.req);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(cstate.req);

	*cstate_ptr = &cstate;

	return 0;
}

/*
 * teardown__conn_req_from_cm_event -- delete the rpma_conn_req object created
 * from a CM event
 */
int
teardown__conn_req_from_cm_event(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(rpma_cq_delete, MOCK_OK);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_OK);
	expect_function_call(rpma_private_data_discard);

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

	/* configure mocks for rpma_conn_req_new() */
	Mock_ctrl_defer_destruction = MOCK_CTRL_DEFER;
	will_return(rpma_conn_cfg_get_timeout, &cstate->get_t);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &cstate->id);
	expect_value(rpma_info_resolve_addr, id, &cstate->id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
			cstate->get_t.timeout_ms);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	expect_value(rdma_resolve_route, timeout_ms, cstate->get_t.timeout_ms);
	will_return(rdma_resolve_route, MOCK_OK);
	will_return(rpma_conn_cfg_get_cqe, &cstate->get_cqe);
	expect_value(rpma_cq_new, cqe, cstate->get_cqe.cq_size);
	will_return(rpma_cq_new, MOCK_RPMA_CQ);
	if (cstate->get_cqe.rcq_size) {
		expect_value(rpma_cq_new, cqe, cstate->get_cqe.rcq_size);
		will_return(rpma_cq_new, MOCK_RPMA_RCQ);
	}
	expect_value(rpma_peer_create_qp, id, &cstate->id);
	expect_value(rpma_peer_create_qp, cfg, cstate->get_cqe.cfg);
	will_return(rpma_peer_create_qp, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_conn_cfg *cfg =
			(cstate->get_cqe.cfg == MOCK_CONN_CFG_DEFAULT ?
					NULL : cstate->get_cqe.cfg);
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, cfg,
			&cstate->req);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(cstate->req);

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
	will_return(rpma_cq_delete, MOCK_OK);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, 0);
	expect_function_call(rpma_private_data_discard);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cstate->req);

	*cstate_ptr = NULL;

	return 0;
}
