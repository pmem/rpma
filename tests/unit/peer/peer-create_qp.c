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

static struct conn_cfg_get_q_size_mock_args Get_sq_size = {
	.cfg = MOCK_CONN_CFG_CUSTOM,
	.q_size = MOCK_SQ_SIZE_CUSTOM
};

static struct conn_cfg_get_q_size_mock_args Get_rq_size = {
	.cfg = MOCK_CONN_CFG_CUSTOM,
	.q_size = MOCK_RQ_SIZE_CUSTOM
};

static struct rpma_cq *rcqs[] = {
	NULL,
	MOCK_RPMA_RCQ
};

static int num_rcqs = sizeof(rcqs) / sizeof(rcqs[0]);

/*
 * configure_create_qp -- configure common mock for rdma_create_qp()
 */
static void
configure_create_qp(struct rpma_cq *rcq)
{
	will_return(rpma_conn_cfg_get_sq_size, &Get_sq_size);
	will_return(rpma_conn_cfg_get_rq_size, &Get_rq_size);
	expect_value(rpma_cq_get_ibv_cq, cq, MOCK_RPMA_CQ);
	will_return(rpma_cq_get_ibv_cq, MOCK_IBV_CQ);
	if (rcq) {
		expect_value(rpma_cq_get_ibv_cq, cq, MOCK_RPMA_RCQ);
		will_return(rpma_cq_get_ibv_cq, MOCK_IBV_RCQ);
	}
	expect_value(rdma_create_qp, id, MOCK_CM_ID);
	expect_value(rdma_create_qp, pd, MOCK_IBV_PD);
	expect_value(rdma_create_qp, qp_init_attr->qp_context, NULL);
	expect_value(rdma_create_qp, qp_init_attr->send_cq,
		MOCK_IBV_CQ);
	expect_value(rdma_create_qp, qp_init_attr->recv_cq,
		rcq ? MOCK_IBV_RCQ : MOCK_IBV_CQ);
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
}

/*
 * create_qp__peer_NULL -- NULL peer is invalid
 */
static void
create_qp__peer_NULL(void **unused)
{
	/* run test */
	int ret = rpma_peer_create_qp(NULL, MOCK_CM_ID, MOCK_RPMA_CQ,
			NULL, MOCK_CONN_CFG_DEFAULT);

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
	int ret = rpma_peer_create_qp(peer, NULL, MOCK_RPMA_CQ, NULL,
			MOCK_CONN_CFG_DEFAULT);

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
	int ret = rpma_peer_create_qp(peer, MOCK_CM_ID, NULL,
			NULL, MOCK_CONN_CFG_DEFAULT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * create_qp__rdma_create_qp_ERRNO -- rdma_create_qp() fails with MOCK_ERRNO
 */
static void
create_qp__rdma_create_qp_ERRNO(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;

	for (int i = 0; i < num_rcqs; i++) {
		/* configure mock */
		configure_create_qp(rcqs[i]);
		will_return(rdma_create_qp, MOCK_ERRNO);

		/* run test */
		int ret = rpma_peer_create_qp(peer, MOCK_CM_ID, MOCK_RPMA_CQ,
				rcqs[i], MOCK_CONN_CFG_CUSTOM);

		/* verify the results */
		assert_int_equal(ret, RPMA_E_PROVIDER);
	}
}

/*
 * create_qp__success -- happy day scenario
 */
static void
create_qp__success(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;

	for (int i = 0; i < num_rcqs; i++) {
		/* configure mock */
		configure_create_qp(rcqs[i]);
		will_return(rdma_create_qp, MOCK_OK);

		/* run test */
		int ret = rpma_peer_create_qp(peer, MOCK_CM_ID, MOCK_RPMA_CQ,
				rcqs[i], MOCK_CONN_CFG_CUSTOM);

		/* verify the results */
		assert_int_equal(ret, MOCK_OK);
	}
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
				create_qp__rdma_create_qp_ERRNO,
				setup__peer, teardown__peer, &OdpCapable),
		cmocka_unit_test_prestate_setup_teardown(create_qp__success,
				setup__peer, teardown__peer, &OdpCapable),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
