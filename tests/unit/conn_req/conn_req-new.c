// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn_req-new.c -- the rpma_conn_req_new() unit tests
 *
 * APIs covered:
 * - rpma_conn_req_new()
 */

#include "conn_req-common.h"
#include "mocks-ibverbs.h"

/*
 * new__peer_NULL -- NULL peer is invalid
 */
static void
new__peer_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(NULL, MOCK_IP_ADDRESS, MOCK_SERVICE,
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
	int ret = rpma_conn_req_new(MOCK_PEER, NULL, MOCK_SERVICE,
					&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * new__service_NULL -- NULL service is invalid
 */
static void
new__service_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, NULL,
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
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
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
	int ret = rpma_conn_req_new(NULL, NULL, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new__info_new_E_PROVIDER_EAGAIN -- rpma_info_new() fails with
 * RPMA_E_PROVIDER+EAGAIN
 */
static void
new__info_new_E_PROVIDER_EAGAIN(void **unused)
{
	struct rdma_cm_id id = {0};

	/* configure mocks */
	will_return(rpma_info_new, NULL);
	will_return(rpma_info_new, RPMA_E_PROVIDER);
	will_return(rpma_info_new, EAGAIN);
	will_return_maybe(rdma_create_id, &id);
	will_return_maybe(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * new__create_id_EAGAIN -- rdma_create_id() fails with EAGAIN
 */
static void
new__create_id_EAGAIN(void **unused)
{
	/* configure mocks */
	will_return(rdma_create_id, NULL);
	will_return(rdma_create_id, EAGAIN);
	will_return_maybe(rpma_info_new, MOCK_INFO);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * new__resolve_addr_E_PROVIDER_EAGAIN -- rpma_info_resolve_addr() fails
 * with RPMA_E_PROVIDER+EAGAIN
 */
static void
new__resolve_addr_E_PROVIDER_EAGAIN(void **unused)
{
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &id);
	expect_value(rpma_info_resolve_addr, id, &id);
	will_return(rpma_info_resolve_addr, RPMA_E_PROVIDER);
	will_return(rpma_info_resolve_addr, EAGAIN);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * new__resolve_route_EAGAIN -- rdma_resolve_route() fails with EAGAIN
 */
static void
new__resolve_route_EAGAIN(void **unused)
{
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &id);
	expect_value(rpma_info_resolve_addr, id, &id);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	will_return(rdma_resolve_route, EAGAIN);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * new__create_comp_channel_EAGAIN -- ibv_create_comp_channel() fails with
 * EAGAIN
 */
static void
new__create_comp_channel_EAGAIN(void **unused)
{
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &id);
	expect_value(rpma_info_resolve_addr, id, &id);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	will_return(rdma_resolve_route, MOCK_OK);
	will_return(ibv_create_comp_channel, NULL);
	will_return(ibv_create_comp_channel, EAGAIN);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * new__create_cq_EAGAIN -- ibv_create_cq() fails with EAGAIN
 */
static void
new__create_cq_EAGAIN(void **unused)
{
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &id);
	expect_value(rpma_info_resolve_addr, id, &id);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	will_return(rdma_resolve_route, MOCK_OK);
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	will_return(ibv_create_cq, NULL);
	will_return(ibv_create_cq, EAGAIN);
	will_return(ibv_destroy_comp_channel, MOCK_OK);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * new__peer_create_qp_E_PROVIDER_EAGAIN -- rpma_peer_create_qp() fails
 * with RPMA_E_PROVIDER+EAGAIN
 */
static void
new__peer_create_qp_E_PROVIDER_EAGAIN(void **unused)
{
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &id);
	expect_value(rpma_info_resolve_addr, id, &id);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	will_return(rdma_resolve_route, MOCK_OK);
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	will_return(ibv_create_cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_OK);
	expect_value(rpma_peer_create_qp, id, &id);
	will_return(rpma_peer_create_qp, RPMA_E_PROVIDER);
	will_return(rpma_peer_create_qp, EAGAIN);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * new__malloc_ENOMEM -- malloc() fails with ENOMEM
 */
static void
new__malloc_ENOMEM(void **unused)
{
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &id);
	expect_value(rpma_info_resolve_addr, id, &id);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	will_return(rdma_resolve_route, MOCK_OK);
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	will_return(ibv_create_cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_OK);
	expect_value(rpma_peer_create_qp, id, &id);
	will_return(rpma_peer_create_qp, MOCK_OK);
	will_return(__wrap__test_malloc, ENOMEM);
	expect_value(rdma_destroy_qp, id, &id);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * new__malloc_ENOMEM_subsequent_EAGAIN -- malloc() fails with ENOMEM
 * whereas subsequent calls fail with EAGAIN
 */
static void
new__malloc_ENOMEM_subsequent_EAGAIN(void **unused)
{
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &id);
	expect_value(rpma_info_resolve_addr, id, &id);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	will_return(rdma_resolve_route, MOCK_OK);
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	will_return(ibv_create_cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_OK);
	expect_value(rpma_peer_create_qp, id, &id);
	will_return(rpma_peer_create_qp, MOCK_OK);
	will_return(__wrap__test_malloc, ENOMEM); /* first error */
	expect_value(rdma_destroy_qp, id, &id);
	will_return(ibv_destroy_cq, EAGAIN); /* second error */
	will_return(ibv_destroy_comp_channel, EAGAIN); /* third error */
	will_return(rdma_destroy_id, EAGAIN); /* fourth error */

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
			&req);

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

const struct CMUnitTest test_new[] = {
	/* rpma_conn_req_new() unit tests */
	cmocka_unit_test(new__peer_NULL),
	cmocka_unit_test(new__addr_NULL),
	cmocka_unit_test(new__service_NULL),
	cmocka_unit_test(new__req_ptr_NULL),
	cmocka_unit_test(new__all_NULL),
	cmocka_unit_test(new__info_new_E_PROVIDER_EAGAIN),
	cmocka_unit_test(new__create_id_EAGAIN),
	cmocka_unit_test(new__resolve_addr_E_PROVIDER_EAGAIN),
	cmocka_unit_test(new__resolve_route_EAGAIN),
	cmocka_unit_test(new__create_comp_channel_EAGAIN),
	cmocka_unit_test(new__create_cq_EAGAIN),
	cmocka_unit_test(new__peer_create_qp_E_PROVIDER_EAGAIN),
	cmocka_unit_test(new__malloc_ENOMEM),
	cmocka_unit_test(new__malloc_ENOMEM_subsequent_EAGAIN),
	cmocka_unit_test_setup_teardown(new__success,
		setup__conn_req_new, teardown__conn_req_new),
	cmocka_unit_test(NULL)
};
