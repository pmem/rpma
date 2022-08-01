// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */
/* Copyright 2021-2022, Fujitsu */

/*
 * conn_req-new.c -- the rpma_conn_req_new() unit tests
 *
 * API covered:
 * - rpma_conn_req_new()
 */

#include "conn_req-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rpma-conn_cfg.h"

/*
 * new__peer_NULL -- NULL peer is invalid
 */
static void
new__peer_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(NULL, MOCK_IP_ADDRESS, MOCK_PORT, NULL,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * new__addr_NULL -- NULL addr is invalid
 */
static void
new__addr_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, NULL, MOCK_PORT, NULL, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * new__port_NULL -- NULL port is invalid
 */
static void
new__port_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, NULL, NULL,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * new__req_ptr_NULL -- NULL req_ptr is invalid
 */
static void
new__req_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, NULL,
					NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new__all_NULL -- all NULL arguments are invalid
 */
static void
new__all_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_req_new(NULL, NULL, NULL, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new__info_new_ERRNO -- rpma_info_new() fails with MOCK_ERRNO
 */
static void
new__info_new_ERRNO(void **unused)
{
	struct conn_req_new_test_state *cstate = NULL;
	configure_conn_req_new((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &cstate->get_args);
	will_return(rpma_info_new, NULL);
	will_return(rpma_info_new, RPMA_E_PROVIDER);
	will_return(rpma_info_new, MOCK_ERRNO);
	will_return_maybe(rdma_create_id, &cstate->id);
	will_return_maybe(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, NULL,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * new__create_id_ERRNO -- rdma_create_id() fails with MOCK_ERRNO
 */
static void
new__create_id_ERRNO(void **unused)
{
	struct conn_req_new_test_state *cstate = NULL;
	configure_conn_req_new((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &cstate->get_args);
	will_return(rdma_create_id, NULL);
	will_return(rdma_create_id, MOCK_ERRNO);
	will_return_maybe(rpma_info_new, MOCK_INFO);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, NULL,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * new__resolve_addr_ERRNO -- rpma_info_resolve_addr() fails
 * with MOCK_ERRNO
 */
static void
new__resolve_addr_ERRNO(void **unused)
{
	struct conn_req_new_test_state *cstate = NULL;
	configure_conn_req_new((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &cstate->get_args);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &cstate->id);
	expect_value(rpma_info_resolve_addr, id, &cstate->id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
			RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rpma_info_resolve_addr, RPMA_E_PROVIDER);
	will_return(rpma_info_resolve_addr, MOCK_ERRNO);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, NULL,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * new__resolve_addr_ERRNO_subsequent_ERRNO2 -- rdma_destroy_id() fails
 * with MOCK_ERRNO2 after rpma_info_resolve_addr() failed with MOCK_ERRNO
 */
static void
new__resolve_addr_ERRNO_subsequent_ERRNO2(void **unused)
{
	struct conn_req_new_test_state *cstate = NULL;
	configure_conn_req_new((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &cstate->get_args);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &cstate->id);
	expect_value(rpma_info_resolve_addr, id, &cstate->id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
			RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rpma_info_resolve_addr, RPMA_E_PROVIDER);
	will_return(rpma_info_resolve_addr, MOCK_ERRNO); /* first error */
	will_return(rdma_destroy_id, MOCK_ERRNO2); /* second error */

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, NULL,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * new__resolve_route_ERRNO -- rdma_resolve_route() fails with MOCK_ERRNO
 */
static void
new__resolve_route_ERRNO(void **unused)
{
	struct conn_req_new_test_state *cstate = NULL;
	configure_conn_req_new((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &cstate->get_args);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &cstate->id);
	expect_value(rpma_info_resolve_addr, id, &cstate->id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
			RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	/*
	 * XXX rdma_resolve_route() mock assumes all its expects comes from
	 * another mock. The following expect breaks this assumption.
	 */
	expect_value(rdma_resolve_route, timeout_ms, RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rdma_resolve_route, MOCK_ERRNO);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, NULL,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * new__resolve_route_ERRNO_subsequent_ERRNO2 -- rdma_destroy_id() fails with
 * MOCK_ERRNO2 after rdma_resolve_route() failed with MOCK_ERRNO
 */
static void
new__resolve_route_ERRNO_subsequent_ERRNO2(void **unused)
{
	struct conn_req_new_test_state *cstate = NULL;
	configure_conn_req_new((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &cstate->get_args);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &cstate->id);
	expect_value(rpma_info_resolve_addr, id, &cstate->id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
		RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	/*
	 * XXX rdma_resolve_route() mock assumes all its expects comes from
	 * another mock. The following expect breaks this assumption.
	 */
	expect_value(rdma_resolve_route, timeout_ms, RPMA_DEFAULT_TIMEOUT_MS);
	will_return(rdma_resolve_route, MOCK_ERRNO); /* first error */
	will_return(rdma_destroy_id, MOCK_ERRNO2); /* second error */

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, NULL,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * new__shared_true_srq_rcq_not_NULL -- true shared and non-NULL srq_rcq are invalid
 */
static void
new__shared_true_srq_rcq_not_NULL(void **unused)
{
	struct conn_req_new_test_state *cstate = &Conn_req_new_conn_cfg_custom_without_srq_rcq;
	configure_conn_req_new((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &cstate->get_args);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &cstate->id);
	expect_value(rpma_info_resolve_addr, id, &cstate->id);
	expect_value(rpma_info_resolve_addr, timeout_ms, cstate->get_args.timeout_ms);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	/*
	 * XXX rdma_resolve_route() mock assumes all its expects comes from
	 * another mock. The following expect breaks this assumption.
	 */
	expect_value(rdma_resolve_route, timeout_ms, cstate->get_args.timeout_ms);
	will_return(rdma_resolve_route, MOCK_OK);
	will_return(rpma_conn_cfg_get_cqe, &cstate->get_args);
	will_return(rpma_conn_cfg_get_rcqe, &cstate->get_args);
	will_return(rpma_conn_cfg_get_compl_channel, &cstate->get_args);
	will_return(rpma_conn_cfg_get_srq, &cstate->get_args);
	if (cstate->get_args.srq) {
		expect_value(rpma_srq_get_rcq, srq, MOCK_RPMA_SRQ);
		will_return(rpma_srq_get_rcq, MOCK_RPMA_SRQ_RCQ);
	}
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT,
			MOCK_GET_CONN_CFG(cstate), &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * new__cq_new_ERRNO -- rpma_cq_new(cqe) fails with MOCK_ERRNO
 */
static void
new__cq_new_ERRNO(void **cstate_ptr)
{
	struct conn_req_new_test_state *cstate = *cstate_ptr;
	configure_conn_req_new((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &cstate->get_args);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &cstate->id);
	expect_value(rpma_info_resolve_addr, id, &cstate->id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
			cstate->get_args.timeout_ms);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	/*
	 * XXX rdma_resolve_route() mock assumes all its expects comes from
	 * another mock. The following expect breaks this assumption.
	 */
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
	will_return(rpma_cq_new, NULL);
	will_return(rpma_cq_new, RPMA_E_PROVIDER);
	will_return(rpma_cq_new, MOCK_ERRNO);
	if (!cstate->get_args.srq_rcq && cstate->get_args.shared)
		will_return(ibv_destroy_comp_channel, MOCK_OK);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT,
			MOCK_GET_CONN_CFG(cstate), &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * new__cq_new_ERRNO_subsequent_ERRNO2 -- rdma_destroy_id() fails with
 * MOCK_ERRNO2 after rpma_cq_new(cqe) failed with MOCK_ERRNO
 */
static void
new__cq_new_ERRNO_subsequent_ERRNO2(void **cstate_ptr)
{
	struct conn_req_new_test_state *cstate = *cstate_ptr;
	configure_conn_req_new((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &cstate->get_args);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &cstate->id);
	expect_value(rpma_info_resolve_addr, id, &cstate->id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
			cstate->get_args.timeout_ms);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	/*
	 * XXX rdma_resolve_route() mock assumes all its expects comes from
	 * another mock. The following expect breaks this assumption.
	 */
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
	will_return(rpma_cq_new, NULL);
	will_return(rpma_cq_new, RPMA_E_PROVIDER);
	will_return(rpma_cq_new, MOCK_ERRNO); /* first error */
	if (!cstate->get_args.srq_rcq && cstate->get_args.shared)
		will_return(ibv_destroy_comp_channel, MOCK_OK);
	will_return(rdma_destroy_id, MOCK_ERRNO2); /* second error */

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT,
			MOCK_GET_CONN_CFG(cstate), &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * new__rcq_new_ERRNO -- rpma_cq_new(rcqe) fails with MOCK_ERRNO
 */
static void
new__rcq_new_ERRNO(void **unused)
{
	struct conn_req_new_test_state *cstate = &Conn_req_new_conn_cfg_custom;
	configure_conn_req_new((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &cstate->get_args);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &cstate->id);
	expect_value(rpma_info_resolve_addr, id, &cstate->id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
			cstate->get_args.timeout_ms);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	/*
	 * XXX rdma_resolve_route() mock assumes all its expects comes from
	 * another mock. The following expect breaks this assumption.
	 */
	expect_value(rdma_resolve_route, timeout_ms, cstate->get_args.timeout_ms);
	will_return(rdma_resolve_route, MOCK_OK);
	will_return(rpma_conn_cfg_get_cqe, &cstate->get_args);
	will_return(rpma_conn_cfg_get_rcqe, &cstate->get_args);
	will_return(rpma_conn_cfg_get_compl_channel, &cstate->get_args);
	will_return(rpma_conn_cfg_get_srq, &cstate->get_args);
	if (cstate->get_args.shared)
		will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	expect_value(rpma_cq_new, cqe, cstate->get_args.cq_size);
	expect_value(rpma_cq_new, shared_channel, MOCK_GET_CHANNEL(cstate));
	will_return(rpma_cq_new, MOCK_RPMA_CQ);
	expect_value(rpma_cq_new, cqe, cstate->get_args.rcq_size);
	expect_value(rpma_cq_new, shared_channel, MOCK_GET_CHANNEL(cstate));
	will_return(rpma_cq_new, NULL);
	will_return(rpma_cq_new, RPMA_E_PROVIDER);
	will_return(rpma_cq_new, MOCK_ERRNO);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);
	if (cstate->get_args.shared)
		will_return(ibv_destroy_comp_channel, MOCK_OK);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT,
			cstate->get_args.cfg, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * new__rcq_new_ERRNO_subsequent_ERRNO2 -- rpma_cq_new(rcqe) fails with
 * MOCK_ERRNO whereas subsequent (rpma_cq_delete(&cq), rdma_destroy_id())
 * fail with MOCK_ERRNO2
 */
static void
new__rcq_new_ERRNO_subsequent_ERRNO2(void **unused)
{
	struct conn_req_new_test_state *cstate = &Conn_req_new_conn_cfg_custom;
	configure_conn_req_new((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &cstate->get_args);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &cstate->id);
	expect_value(rpma_info_resolve_addr, id, &cstate->id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
			cstate->get_args.timeout_ms);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	/*
	 * XXX rdma_resolve_route() mock assumes all its expects comes from
	 * another mock. The following expect breaks this assumption.
	 */
	expect_value(rdma_resolve_route, timeout_ms, cstate->get_args.timeout_ms);
	will_return(rdma_resolve_route, MOCK_OK);
	will_return(rpma_conn_cfg_get_cqe, &cstate->get_args);
	will_return(rpma_conn_cfg_get_rcqe, &cstate->get_args);
	will_return(rpma_conn_cfg_get_compl_channel, &cstate->get_args);
	will_return(rpma_conn_cfg_get_srq, &cstate->get_args);
	if (cstate->get_args.shared)
		will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	expect_value(rpma_cq_new, cqe, cstate->get_args.cq_size);
	expect_value(rpma_cq_new, shared_channel, MOCK_GET_CHANNEL(cstate));
	will_return(rpma_cq_new, MOCK_RPMA_CQ);
	expect_value(rpma_cq_new, cqe, cstate->get_args.rcq_size);
	expect_value(rpma_cq_new, shared_channel, MOCK_GET_CHANNEL(cstate));
	will_return(rpma_cq_new, NULL);
	will_return(rpma_cq_new, RPMA_E_PROVIDER);
	will_return(rpma_cq_new, MOCK_ERRNO); /* first error */
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO2); /* second error */
	if (cstate->get_args.shared)
		will_return(ibv_destroy_comp_channel, MOCK_OK);
	will_return(rdma_destroy_id, MOCK_ERRNO2); /* third error */

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT,
			cstate->get_args.cfg, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * new__peer_create_qp_ERRNO -- rpma_peer_setup_qp() fails
 * with MOCK_ERRNO
 */
static void
new__peer_create_qp_ERRNO(void **cstate_ptr)
{
	struct conn_req_new_test_state *cstate = *cstate_ptr;
	configure_conn_req_new((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &cstate->get_args);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &cstate->id);
	expect_value(rpma_info_resolve_addr, id, &cstate->id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
				cstate->get_args.timeout_ms);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	/*
	 * XXX rdma_resolve_route() mock assumes all its expects comes from
	 * another mock. The following expect breaks this assumption.
	 */
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
	will_return(rpma_peer_setup_qp, RPMA_E_PROVIDER);
	will_return(rpma_peer_setup_qp, MOCK_ERRNO);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ_DEL(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);
	if (!cstate->get_args.srq_rcq && cstate->get_args.shared)
		will_return(ibv_destroy_comp_channel, MOCK_OK);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT,
			MOCK_GET_CONN_CFG(cstate), &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * new__peer_create_qp_ERRNO_subsequent_ERRNO2 -- rpma_peer_setup_qp()
 * fails with MOCK_ERRNO whereas subsequent (rpma_cq_delete(&rcq),
 * rpma_cq_delete(&cq), rdma_destroy_id()) fail with MOCK_ERRNO2
 */
static void
new__peer_create_qp_ERRNO_subsequent_ERRNO2(void **cstate_ptr)
{
	struct conn_req_new_test_state *cstate = *cstate_ptr;
	configure_conn_req_new((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &cstate->get_args);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &cstate->id);
	expect_value(rpma_info_resolve_addr, id, &cstate->id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
			cstate->get_args.timeout_ms);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	/*
	 * XXX rdma_resolve_route() mock assumes all its expects comes from
	 * another mock. The following expect breaks this assumption.
	 */
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
	will_return(rpma_peer_setup_qp, RPMA_E_PROVIDER);
	will_return(rpma_peer_setup_qp, MOCK_ERRNO); /* first error */
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ_DEL(cstate));
	if (!cstate->get_args.srq_rcq && cstate->get_args.rcq_size) {
		will_return(rpma_cq_delete, RPMA_E_PROVIDER);
		will_return(rpma_cq_delete, MOCK_ERRNO2); /* second error */
	} else {
		/* rcq == NULL cannot fail */
		will_return(rpma_cq_delete, MOCK_OK);
	}
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO2); /* second or third error */
	if (!cstate->get_args.srq_rcq && cstate->get_args.shared)
		will_return(ibv_destroy_comp_channel, MOCK_OK);
	will_return(rdma_destroy_id, MOCK_ERRNO2); /* third or fourth error */

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT,
			MOCK_GET_CONN_CFG(cstate), &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(req);
}

/*
 * new__malloc_ERRNO -- malloc() fails with MOCK_ERRNO
 */
static void
new__malloc_ERRNO(void **cstate_ptr)
{
	struct conn_req_new_test_state *cstate = *cstate_ptr;
	configure_conn_req_new((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &cstate->get_args);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &cstate->id);
	expect_value(rpma_info_resolve_addr, id, &cstate->id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
				cstate->get_args.timeout_ms);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	/*
	 * XXX rdma_resolve_route() mock assumes all its expects comes from
	 * another mock. The following expect breaks this assumption.
	 */
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
	expect_value(rpma_peer_setup_qp, rcq, MOCK_GET_RCQ(cstate));
	expect_value(rpma_peer_setup_qp, cfg, cstate->get_args.cfg);
	will_return(rpma_peer_setup_qp, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_ERRNO);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ_DEL(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);
	if (!cstate->get_args.srq_rcq && cstate->get_args.shared)
		will_return(ibv_destroy_comp_channel, MOCK_OK);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT,
			MOCK_GET_CONN_CFG(cstate), &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * new__malloc_ERRNO_subsequent_ERRNO2 -- malloc() fails with MOCK_ERRNO
 * whereas subsequent (rpma_cq_delete(&rcq), rpma_cq_delete(&cq),
 * rdma_destroy_id()) fail with MOCK_ERRNO2
 */
static void
new__malloc_ERRNO_subsequent_ERRNO2(void **cstate_ptr)
{
	struct conn_req_new_test_state *cstate = *cstate_ptr;
	configure_conn_req_new((void **)&cstate);

	/* configure mocks */
	will_return(rpma_conn_cfg_get_timeout, &cstate->get_args);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &cstate->id);
	expect_value(rpma_info_resolve_addr, id, &cstate->id);
	expect_value(rpma_info_resolve_addr, timeout_ms,
				cstate->get_args.timeout_ms);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	/*
	 * XXX rdma_resolve_route() mock assumes all its expects comes from
	 * another mock. The following expect breaks this assumption.
	 */
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
	expect_value(rpma_peer_setup_qp, rcq, MOCK_GET_RCQ(cstate));
	expect_value(rpma_peer_setup_qp, cfg, cstate->get_args.cfg);
	will_return(rpma_peer_setup_qp, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_ERRNO); /* first error */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ_DEL(cstate));
	if (!cstate->get_args.srq_rcq && cstate->get_args.rcq_size) {
		will_return(rpma_cq_delete, RPMA_E_PROVIDER);
		will_return(rpma_cq_delete, MOCK_ERRNO2); /* second error */
	} else {
		/* rcq == NULL cannot fail */
		will_return(rpma_cq_delete, MOCK_OK);
	}
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO2); /* second or third error */
	if (!cstate->get_args.srq_rcq && cstate->get_args.shared)
		will_return(ibv_destroy_comp_channel, MOCK_OK);
	will_return(rdma_destroy_id, MOCK_ERRNO2); /* third or fourth error */

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT,
			MOCK_GET_CONN_CFG(cstate), &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * new__success -- all is OK
 */
static void
new__success(void **unused)
{
	/*
	 * The thing is done by setup__conn_req_new() and
	 * teardown__conn_req_new().
	 */
}

static const struct CMUnitTest test_new[] = {
	/* rpma_conn_req_new() unit tests */
	cmocka_unit_test(new__peer_NULL),
	cmocka_unit_test(new__addr_NULL),
	cmocka_unit_test(new__port_NULL),
	cmocka_unit_test(new__req_ptr_NULL),
	cmocka_unit_test(new__all_NULL),
	cmocka_unit_test(new__info_new_ERRNO),
	cmocka_unit_test(new__create_id_ERRNO),
	cmocka_unit_test(new__resolve_addr_ERRNO),
	cmocka_unit_test(new__resolve_addr_ERRNO_subsequent_ERRNO2),
	cmocka_unit_test(new__resolve_route_ERRNO),
	cmocka_unit_test(new__resolve_route_ERRNO_subsequent_ERRNO2),
	cmocka_unit_test(new__shared_true_srq_rcq_not_NULL),
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_RCQ(new__cq_new_ERRNO),
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_SRQ_RCQ(new__cq_new_ERRNO),
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_RCQ(
		new__cq_new_ERRNO_subsequent_ERRNO2),
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_SRQ_RCQ(
		new__cq_new_ERRNO_subsequent_ERRNO2),
	cmocka_unit_test(new__rcq_new_ERRNO),
	cmocka_unit_test(new__rcq_new_ERRNO_subsequent_ERRNO2),
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_RCQ(new__peer_create_qp_ERRNO),
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_SRQ_RCQ(new__peer_create_qp_ERRNO),
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_RCQ(
		new__peer_create_qp_ERRNO_subsequent_ERRNO2),
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_SRQ_RCQ(
		new__peer_create_qp_ERRNO_subsequent_ERRNO2),
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_RCQ(new__malloc_ERRNO),
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_SRQ_RCQ(new__malloc_ERRNO),
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_RCQ(
		new__malloc_ERRNO_subsequent_ERRNO2),
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_SRQ_RCQ(new__malloc_ERRNO_subsequent_ERRNO2),
	CONN_REQ_NEW_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_RCQ(
		new__success, setup__conn_req_new, teardown__conn_req_new),
	CONN_REQ_NEW_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_SRQ_RCQ(
		new__success, setup__conn_req_new, teardown__conn_req_new),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_new, NULL, NULL);
}
