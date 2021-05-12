// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * peer-create_qp.c -- a peer unit test
 *
 * API covered:
 * - rpma_peer_create_qp()
 */

#include <infiniband/verbs.h>

#include "cmocka_headers.h"
#include "conn_req.h"
#include "mocks-ibverbs.h"
#include "mocks-rpma-conn_cfg.h"
#include "mocks-rpma-cq.h"
#include "peer.h"
#include "peer-common.h"
#include "test-common.h"

static struct conn_cfg_get_q_size_mock_args Get_sq_size = {
	.cfg = MOCK_CONN_CFG_CUSTOM,
	.q_size = MOCK_SQ_SIZE_CUSTOM
};

static struct conn_cfg_get_q_size_mock_args Get_rq_size = {
	.cfg = MOCK_CONN_CFG_CUSTOM,
	.q_size = MOCK_RQ_SIZE_CUSTOM
};

/*
 * create_qp__peer_NULL -- NULL peer is invalid
 */
static void
create_qp__peer_NULL(void **unused)
{
	/* run test */
	struct rdma_cm_id *id = MOCK_CM_ID;
	struct rpma_cq *cq = MOCK_RPMA_CQ;
	int ret = rpma_peer_create_qp(NULL, id, cq, MOCK_CONN_CFG_DEFAULT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * create_qp__id_NULL -- NULL id is invalid
 */
static void
create_qp__id_NULL(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;

	/* run test */
	struct rpma_cq *cq = MOCK_RPMA_CQ;
	int ret = rpma_peer_create_qp(peer, NULL, cq, MOCK_CONN_CFG_DEFAULT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * create_qp__cq_NULL -- NULL cq is invalid
 */
static void
create_qp__cq_NULL(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;

	/* run test */
	struct rdma_cm_id *id = MOCK_CM_ID;
	int ret = rpma_peer_create_qp(peer, id, NULL, MOCK_CONN_CFG_DEFAULT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * create_qp__rdma_create_qp_EAGAIN -- rdma_create_qp() fails with EAGAIN
 */
static void
create_qp__rdma_create_qp_EAGAIN(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;

	/* configure mock: */
	will_return(rpma_conn_cfg_get_sq_size, &Get_sq_size);
	will_return(rpma_conn_cfg_get_rq_size, &Get_rq_size);
	will_return(rpma_cq_get_ibv_cq, MOCK_IBV_CQ);
	expect_value(rdma_create_qp, id, MOCK_CM_ID);
	expect_value(rdma_create_qp, pd, MOCK_IBV_PD);
	expect_value(rdma_create_qp, qp_init_attr->qp_context, NULL);
	expect_value(rdma_create_qp, qp_init_attr->send_cq, MOCK_IBV_CQ);
	expect_value(rdma_create_qp, qp_init_attr->recv_cq, MOCK_IBV_CQ);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_send_wr,
		MOCK_SQ_SIZE_CUSTOM);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_recv_wr,
		MOCK_RQ_SIZE_CUSTOM);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_send_sge,
		RPMA_MAX_SGE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_recv_sge,
		RPMA_MAX_SGE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_inline_data,
		RPMA_MAX_INLINE_DATA);
	will_return(rdma_create_qp, EAGAIN);

	/* run test */
	struct rdma_cm_id *id = MOCK_CM_ID;
	struct rpma_cq *cq = MOCK_RPMA_CQ;
	int ret = rpma_peer_create_qp(peer, id, cq, MOCK_CONN_CFG_CUSTOM);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * create_qp__success -- happy day scenario
 */
static void
create_qp__success(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;

	/* configure mock: */
	will_return(rpma_conn_cfg_get_sq_size, &Get_sq_size);
	will_return(rpma_conn_cfg_get_rq_size, &Get_rq_size);
	will_return(rpma_cq_get_ibv_cq, MOCK_IBV_CQ);
	expect_value(rdma_create_qp, id, MOCK_CM_ID);
	expect_value(rdma_create_qp, pd, MOCK_IBV_PD);
	expect_value(rdma_create_qp, qp_init_attr->qp_context, NULL);
	expect_value(rdma_create_qp, qp_init_attr->send_cq, MOCK_IBV_CQ);
	expect_value(rdma_create_qp, qp_init_attr->recv_cq, MOCK_IBV_CQ);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_send_wr,
		MOCK_SQ_SIZE_CUSTOM);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_recv_wr,
		MOCK_RQ_SIZE_CUSTOM);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_send_sge,
		RPMA_MAX_SGE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_recv_sge,
		RPMA_MAX_SGE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_inline_data,
		RPMA_MAX_INLINE_DATA);
	will_return(rdma_create_qp, 0);

	/* run test */
	struct rdma_cm_id *id = MOCK_CM_ID;
	struct rpma_cq *cq = MOCK_RPMA_CQ;
	int ret = rpma_peer_create_qp(peer, id, cq, MOCK_CONN_CFG_CUSTOM);

	/* verify the results */
	assert_int_equal(ret, 0);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_peer_create_qp() unit tests */
		cmocka_unit_test(create_qp__peer_NULL),
		cmocka_unit_test_prestate_setup_teardown(create_qp__id_NULL,
				setup__peer, teardown__peer, &OdpCapable),
		cmocka_unit_test_prestate_setup_teardown(create_qp__cq_NULL,
				setup__peer, teardown__peer, &OdpCapable),
		cmocka_unit_test_prestate_setup_teardown(
				create_qp__rdma_create_qp_EAGAIN,
				setup__peer, teardown__peer, &OdpCapable),
		cmocka_unit_test_prestate_setup_teardown(create_qp__success,
				setup__peer, teardown__peer, &OdpCapable),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
