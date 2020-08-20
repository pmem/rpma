// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

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
#include "peer.h"
#include "peer-common.h"
#include "test-common.h"

/*
 * create_qp__peer_NULL -- NULL peer is invalid
 */
static void
create_qp__peer_NULL(void **unused)
{
	/* run test */
	struct rdma_cm_id *id = MOCK_CM_ID;
	struct ibv_cq *cq = MOCK_IBV_CQ;
	int ret = rpma_peer_create_qp(NULL, id, cq);

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
	struct ibv_cq *cq = MOCK_IBV_CQ;
	int ret = rpma_peer_create_qp(peer, NULL, cq);

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
	int ret = rpma_peer_create_qp(peer, id, NULL);

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
	expect_value(rdma_create_qp, id, MOCK_CM_ID);
	expect_value(rdma_create_qp, pd, MOCK_IBV_PD);
	expect_value(rdma_create_qp, qp_init_attr->qp_context, NULL);
	expect_value(rdma_create_qp, qp_init_attr->send_cq, MOCK_IBV_CQ);
	expect_value(rdma_create_qp, qp_init_attr->recv_cq, MOCK_IBV_CQ);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_send_wr,
		RPMA_DEFAULT_Q_SIZE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_recv_wr,
		RPMA_DEFAULT_Q_SIZE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_send_sge,
		RPMA_MAX_SGE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_recv_sge,
		RPMA_MAX_SGE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_inline_data,
		RPMA_MAX_INLINE_DATA);
	will_return(rdma_create_qp, EAGAIN);

	/* run test */
	struct rdma_cm_id *id = MOCK_CM_ID;
	struct ibv_cq *cq = MOCK_IBV_CQ;
	int ret = rpma_peer_create_qp(peer, id, cq);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
}

/*
 * create_qp__success -- happy day scenario
 */
static void
create_qp__success(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;

	/* configure mock: */
	expect_value(rdma_create_qp, id, MOCK_CM_ID);
	expect_value(rdma_create_qp, pd, MOCK_IBV_PD);
	expect_value(rdma_create_qp, qp_init_attr->qp_context, NULL);
	expect_value(rdma_create_qp, qp_init_attr->send_cq, MOCK_IBV_CQ);
	expect_value(rdma_create_qp, qp_init_attr->recv_cq, MOCK_IBV_CQ);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_send_wr,
		RPMA_DEFAULT_Q_SIZE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_recv_wr,
		RPMA_DEFAULT_Q_SIZE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_send_sge,
		RPMA_MAX_SGE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_recv_sge,
		RPMA_MAX_SGE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_inline_data,
		RPMA_MAX_INLINE_DATA);
	will_return(rdma_create_qp, 0);

	/* run test */
	struct rdma_cm_id *id = MOCK_CM_ID;
	struct ibv_cq *cq = MOCK_IBV_CQ;
	int ret = rpma_peer_create_qp(peer, id, cq);

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
