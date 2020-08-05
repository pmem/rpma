// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn_req-from_cm_event.c -- the rpma_conn_req_from_cm_event() unit tests
 *
 * APIs covered:
 * - rpma_conn_req_from_cm_event()
 */

#include "conn_req-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"

/*
 * from_cm_event__peer_NULL -- NULL peer is invalid
 */
static void
from_cm_event__peer_NULL(void **unused)
{
	/* run test */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(NULL, &event, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * from_cm_event__edata_NULL -- NULL edata is invalid
 */
static void
from_cm_event__edata_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, NULL, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * from_cm_event__req_ptr_NULL -- NULL req_ptr is invalid
 */
static void
from_cm_event__req_ptr_NULL(void **unused)
{
	/* run test */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * from_cm_event__peer_NULL_edata_NULL_req_ptr_NULL -- NULL peer,
 * NULL edata and NULL req_ptr are not valid
 */
static void
from_cm_event__peer_NULL_edata_NULL_req_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_req_from_cm_event(NULL, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * from_cm_event__RDMA_CM_EVENT_CONNECT_ERROR -- edata of type
 * RDMA_CM_EVENT_CONNECT_ERROR
 */
static void
from_cm_event__RDMA_CM_EVENT_CONNECT_ERROR(void **unused)
{
	/* run test */
	struct rdma_cm_event event = CM_EVENT_CONNECT_ERROR_INIT;
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * from_cm_event__create_comp_channel_EAGAIN -- ibv_create_comp_channel()
 * fails with EAGAIN
 */
static void
from_cm_event__create_comp_channel_EAGAIN(void **unused)
{
	/* configure mocks */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;
	event.id = &id;
	will_return(ibv_create_comp_channel, NULL);
	will_return(ibv_create_comp_channel, EAGAIN);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * from_cm_event__create_cq_EAGAIN -- ibv_create_cq() fails with EAGAIN
 */
static void
from_cm_event__create_cq_EAGAIN(void **unused)
{
	/* configure mocks */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;
	event.id = &id;
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	will_return(ibv_create_cq, NULL);
	will_return(ibv_create_cq, EAGAIN);
	will_return(ibv_destroy_comp_channel, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * from_cm_event__req_notify_cq_fail -- ibv_req_notify_cq() fails
 */
static void
from_cm_event__req_notify_cq_fail(void **unused)
{
	/* configure mocks */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;
	event.id = &id;
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	will_return(ibv_create_cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_ERRNO);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), MOCK_ERRNO);
	assert_null(req);
}

/*
 * from_cm_event_test_peer_create_qp_E_PROVIDER_EAGAIN -- rpma_peer_create_qp()
 * fails with RPMA_E_PROVIDER+EAGAIN
 */
static void
from_cm_event__peer_create_qp_E_PROVIDER_EAGAIN(void **unused)
{
	/* configure mocks */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;
	event.id = &id;
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	will_return(ibv_create_cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_OK);
	expect_value(rpma_peer_create_qp, id, &id);
	will_return(rpma_peer_create_qp, RPMA_E_PROVIDER);
	will_return(rpma_peer_create_qp, EAGAIN);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * from_cm_event__create_qp_EAGAIN_subsequent_EIO -- rpma_peer_create_qp()
 * fails with RPMA_E_PROVIDER+EAGAIN followed by ibv_destroy_cq() and
 * ibv_destroy_comp_channel() fail with EIO
 */
static void
from_cm_event__create_qp_EAGAIN_subsequent_EIO(
		void **unused)
{
	/* configure mocks */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;
	event.id = &id;
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	will_return(ibv_create_cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_OK);
	expect_value(rpma_peer_create_qp, id, &id);
	will_return(rpma_peer_create_qp, RPMA_E_PROVIDER); /* first error */
	will_return(rpma_peer_create_qp, EAGAIN);
	will_return(ibv_destroy_cq, EIO); /* second error */
	will_return(ibv_destroy_comp_channel, EIO); /* third error */

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * from_cm_event__malloc_ENOMEM -- malloc() fails with ENOMEM
 */
static void
from_cm_event__malloc_ENOMEM(void **unused)
{
	/* configure mocks */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;
	event.id = &id;
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	will_return(ibv_create_cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_OK);
	expect_value(rpma_peer_create_qp, id, &id);
	will_return(rpma_peer_create_qp, MOCK_OK);
	will_return(__wrap__test_malloc, ENOMEM);
	expect_value(rdma_destroy_qp, id, &id);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * from_cm_event__malloc_ENOMEM_subsequent_EAGAIN -- malloc() fail with
 * ENOMEM followed by ibv_destroy_cq() and ibv_destroy_comp_channel() fail with
 * EAGAIN
 */
static void
from_cm_event__malloc_ENOMEM_subsequent_EAGAIN(void **unused)
{
	/* configure mocks */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;
	event.id = &id;
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	will_return(ibv_create_cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_OK);
	expect_value(rpma_peer_create_qp, id, &id);
	will_return(rpma_peer_create_qp, MOCK_OK);
	will_return(__wrap__test_malloc, ENOMEM); /* first error */
	expect_value(rdma_destroy_qp, id, &id);
	will_return(ibv_destroy_cq, EAGAIN); /* second error */
	will_return(ibv_destroy_comp_channel, EAGAIN); /* third error */

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * from_cm_event__private_data_store_ENOMEM -- rpma_private_data_store()
 * fails with RPMA_E_NOMEM
 */
static void
from_cm_event__private_data_store_ENOMEM(void **unused)
{
	/* configure mocks */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;
	event.id = &id;
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	will_return(ibv_create_cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_OK);
	expect_value(rpma_peer_create_qp, id, &id);
	will_return(rpma_peer_create_qp, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_OK);
	will_return(rpma_private_data_store, NULL);
	expect_value(rdma_destroy_qp, id, &id);
	will_return(ibv_destroy_cq, EAGAIN); /* second error */
	will_return(ibv_destroy_comp_channel, MOCK_OK);
	expect_value(rdma_destroy_id, id, &id);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * conn_req_from_cm__lifecycle - happy day scenario
 */
static void
conn_req_from_cm__lifecycle(void **unused)
{
	/*
	 * The thing is done by setup__conn_req_from_cm_event() and
	 * teardown__conn_req_from_cm_event().
	 */
}

const struct CMUnitTest test_from_cm_event[] = {
	/* rpma_conn_req_from_cm_event() unit tests */
	cmocka_unit_test(from_cm_event__peer_NULL),
	cmocka_unit_test(from_cm_event__edata_NULL),
	cmocka_unit_test(from_cm_event__req_ptr_NULL),
	cmocka_unit_test(
		from_cm_event__peer_NULL_edata_NULL_req_ptr_NULL),
	cmocka_unit_test(
		from_cm_event__RDMA_CM_EVENT_CONNECT_ERROR),
	cmocka_unit_test(from_cm_event__create_comp_channel_EAGAIN),
	cmocka_unit_test(from_cm_event__create_cq_EAGAIN),
	cmocka_unit_test(from_cm_event__req_notify_cq_fail),
	cmocka_unit_test(
		from_cm_event__peer_create_qp_E_PROVIDER_EAGAIN),
	cmocka_unit_test(
		from_cm_event__create_qp_EAGAIN_subsequent_EIO),
	cmocka_unit_test(from_cm_event__malloc_ENOMEM),
	cmocka_unit_test(
		from_cm_event__malloc_ENOMEM_subsequent_EAGAIN),
	cmocka_unit_test(
		from_cm_event__private_data_store_ENOMEM),
	/* rpma_conn_req_from_cm_event()/_delete() lifecycle */
	cmocka_unit_test_setup_teardown(conn_req_from_cm__lifecycle,
		setup__conn_req_from_cm_event,
		teardown__conn_req_from_cm_event),
};
