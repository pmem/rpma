/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn_req-test.c -- a connection request unit test
 */

#include <infiniband/verbs.h>

#include "cmocka_headers.h"
#include "conn_req.h"
#include "info.h"
#include "rpma_err.h"
#include "test-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"

/* tests */

/*
 * from_cm_event_test_peer_NULL -- NULL peer is invalid
 */
static void
from_cm_event_test_peer_NULL(void **unused)
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
 * from_cm_event_test_edata_NULL -- NULL edata is invalid
 */
static void
from_cm_event_test_edata_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, NULL, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * from_cm_event_test_req_ptr_NULL -- NULL req_ptr is invalid
 */
static void
from_cm_event_test_req_ptr_NULL(void **unused)
{
	/* run test */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * from_cm_event_test_peer_NULL_edata_NULL_req_ptr_NULL -- NULL peer,
 * NULL edata and NULL req_ptr are not valid
 */
static void
from_cm_event_test_peer_NULL_edata_NULL_req_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_req_from_cm_event(NULL, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * from_cm_event_test_RDMA_CM_EVENT_CONNECT_ERROR -- edata of type
 * RDMA_CM_EVENT_CONNECT_ERROR
 */
static void
from_cm_event_test_RDMA_CM_EVENT_CONNECT_ERROR(void **unused)
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
 * from_cm_event_test_create_comp_channel_EAGAIN -- ibv_create_comp_channel()
 * fails with EAGAIN
 */
static void
from_cm_event_test_create_comp_channel_EAGAIN(void **unused)
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
 * from_cm_event_test_create_cq_EAGAIN -- ibv_create_cq() fails with EAGAIN
 */
static void
from_cm_event_test_create_cq_EAGAIN(void **unused)
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
 * from_cm_event_test_peer_create_qp_E_PROVIDER_EAGAIN -- rpma_peer_create_qp()
 * fails with RPMA_E_PROVIDER+EAGAIN
 */
static void
from_cm_event_test_peer_create_qp_E_PROVIDER_EAGAIN(void **unused)
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
 * from_cm_event_test_create_qp_EAGAIN_subsequent_EIO -- rpma_peer_create_qp()
 * fails with RPMA_E_PROVIDER+EAGAIN followed by ibv_destroy_cq() and
 * ibv_destroy_comp_channel() fail with EIO
 */
static void
from_cm_event_test_create_qp_EAGAIN_subsequent_EIO(
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
 * from_cm_event_test_malloc_ENOMEM -- malloc() fails with ENOMEM
 */
static void
from_cm_event_test_malloc_ENOMEM(void **unused)
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
 * from_cm_event_test_malloc_ENOMEM_subsequent_EAGAIN -- malloc() fail with
 * ENOMEM followed by ibv_destroy_cq() and ibv_destroy_comp_channel() fail with
 * EAGAIN
 */
static void
from_cm_event_test_malloc_ENOMEM_subsequent_EAGAIN(void **unused)
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
 * from_cm_event_test_private_data_store_ENOMEM -- rpma_private_data_store()
 * fails with RPMA_E_NOMEM
 */
static void
from_cm_event_test_private_data_store_ENOMEM(void **unused)
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
 * all the resources used between conn_req_from_cm_event_setup and
 * conn_req_from_cm_event_teardown
 */
struct conn_req_test_state {
	struct rdma_cm_event event;
	struct rdma_cm_id id;
	struct rpma_conn_req *req;
};

/*
 * conn_req_from_cm_event_setup -- prepare a valid rpma_conn_req object from CM
 * event
 */
static int
conn_req_from_cm_event_setup(void **cstate_ptr)
{
	static struct conn_req_test_state cstate = {{0}};
	memset(&cstate, 0, sizeof(cstate));
	cstate.event.event = RDMA_CM_EVENT_CONNECT_REQUEST;
	cstate.event.id = &cstate.id;
	cstate.id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	will_return(ibv_create_cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_OK);
	expect_value(rpma_peer_create_qp, id, &cstate.id);
	will_return(rpma_peer_create_qp, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_OK);
	will_return(rpma_private_data_store, MOCK_PRIVATE_DATA);

	/* run test */
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &cstate.event,
		&cstate.req);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(cstate.req);

	*cstate_ptr = &cstate;

	return 0;
}

/*
 * conn_req_from_cm_event_teardown -- delete the rpma_conn_req object created
 * from a CM event
 */
static int
conn_req_from_cm_event_teardown(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cstate->req);

	*cstate_ptr = NULL;

	return 0;
}

/*
 * conn_req_from_cm_test_lifecycle - happy day scenario
 */
static void
conn_req_from_cm_test_lifecycle(void **unused)
{
	/*
	 * The thing is done by conn_req_from_cm_event_setup() and
	 * conn_req_from_cm_event_teardown().
	 */
}

/*
 * new_test_peer_NULL -- NULL peer is invalid
 */
static void
new_test_peer_NULL(void **unused)
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
 * new_test_addr_NULL -- NULL addr is invalid
 */
static void
new_test_addr_NULL(void **unused)
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
 * new_test_service_NULL -- NULL service is invalid
 */
static void
new_test_service_NULL(void **unused)
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
 * new_test_req_ptr_NULL -- NULL req_ptr is invalid
 */
static void
new_test_req_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
					NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new_test_all_NULL -- all NULL arguments are invalid
 */
static void
new_test_all_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_req_new(NULL, NULL, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new_test_info_new_E_PROVIDER_EAGAIN -- rpma_info_new() fails with
 * RPMA_E_PROVIDER+EAGAIN
 */
static void
new_test_info_new_E_PROVIDER_EAGAIN(void **unused)
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
 * new_test_create_id_EAGAIN -- rdma_create_id() fails with EAGAIN
 */
static void
new_test_create_id_EAGAIN(void **unused)
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
 * new_test_resolve_addr_E_PROVIDER_EAGAIN -- rpma_info_resolve_addr() fails
 * with RPMA_E_PROVIDER+EAGAIN
 */
static void
new_test_resolve_addr_E_PROVIDER_EAGAIN(void **unused)
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
 * new_test_resolve_route_EAGAIN -- rdma_resolve_route() fails with EAGAIN
 */
static void
new_test_resolve_route_EAGAIN(void **unused)
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
 * new_test_create_comp_channel_EAGAIN -- ibv_create_comp_channel() fails with
 * EAGAIN
 */
static void
new_test_create_comp_channel_EAGAIN(void **unused)
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
 * new_test_create_cq_EAGAIN -- ibv_create_cq() fails with EAGAIN
 */
static void
new_test_create_cq_EAGAIN(void **unused)
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
 * new_test_peer_create_qp_E_PROVIDER_EAGAIN -- rpma_peer_create_qp() fails
 * with RPMA_E_PROVIDER+EAGAIN
 */
static void
new_test_peer_create_qp_E_PROVIDER_EAGAIN(void **unused)
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
 * new_test_malloc_ENOMEM -- malloc() fails with ENOMEM
 */
static void
new_test_malloc_ENOMEM(void **unused)
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
 * new_test_malloc_ENOMEM_subsequent_EAGAIN -- malloc() fails with ENOMEM
 * whereas subsequent calls fail with EAGAIN
 */
static void
new_test_malloc_ENOMEM_subsequent_EAGAIN(void **unused)
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
 * all the resources used between conn_req_new_setup and conn_req_new_teardown
 */
struct conn_req_new_test_state {
	struct rdma_cm_id id;
	struct rpma_conn_req *req;
};

/*
 * conn_req_new_setup -- prepare a new outgoing rpma_conn_req
 */
static int
conn_req_new_setup(void **cstate_ptr)
{
	static struct conn_req_new_test_state cstate = {{0}};
	memset(&cstate, 0, sizeof(cstate));
	cstate.id.verbs = MOCK_VERBS;

	/* configure mocks for rpma_conn_req_new() */
	Mock_ctrl_defer_destruction = MOCK_CTRL_DEFER;
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &cstate.id);
	expect_value(rpma_info_resolve_addr, id, &cstate.id);
	will_return(rpma_info_resolve_addr, MOCK_OK);
	will_return(rdma_resolve_route, MOCK_OK);
	will_return(ibv_create_comp_channel, MOCK_COMP_CHANNEL);
	will_return(ibv_create_cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_OK);
	expect_value(rpma_peer_create_qp, id, &cstate.id);
	will_return(rpma_peer_create_qp, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_OK);

	/* run test */
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
			&cstate.req);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(cstate.req);

	*cstate_ptr = &cstate;

	/* restore default mock configuration */
	Mock_ctrl_defer_destruction = MOCK_CTRL_NO_DEFER;

	return 0;
}

/*
 * conn_req_new_teardown -- delete the outgoing rpma_conn_req object
 */
static int
conn_req_new_teardown(void **cstate_ptr)
{
	struct conn_req_new_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, 0);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cstate->req);

	*cstate_ptr = NULL;

	return 0;
}

/*
 * new_test_success -- all is OK
 */
static void
new_test_success(void **unused)
{
	/*
	 * The thing is done by conn_req_new_setup() and
	 * conn_req_new_teardown().
	 */
}

/*
 * del_test_req_ptr_NULL -- NULL req_ptr is invalid
 */
static void
del_test_req_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_req_delete(NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * del_test_req_NULL -- NULL req is valid - quick exit
 */
static void
del_test_req_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_delete(&req);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * del_via_reject_test_destroy_cq_EAGAIN - ibv_destroy_cq() fails with EAGAIN
 */
static void
del_via_reject_test_destroy_cq_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, EAGAIN);
	will_return(ibv_destroy_comp_channel, MOCK_OK);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
}

/*
 * del_via_reject_test_destroy_cq_EAGAIN_subsequent_EIO -
 * rdma_ack_cm_event() fails with EIO after rdma_reject() failed
 * with EIO after ibv_destroy_cq() failed with EAGAIN
 */
static void
del_via_reject_test_destroy_cq_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, EAGAIN);
	will_return(ibv_destroy_comp_channel, EIO); /* first error */
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, EIO); /* second error */
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, EIO); /* third error */

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
}

/*
 * del_via_reject_test_destroy_comp_channel_EAGAIN -- ibv_destroy_comp_channel()
 * fails with EAGAIN
 */
static void
del_via_reject_test_destroy_comp_channel_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, EAGAIN);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
}

/*
 * del_via_reject_test_destroy_comp_channel_EAGAIN_subsequent_EIO --
 * ibv_destroy_comp_channel() fails with EAGAIN whereas subsequent fail with EIO
 */
static void
del_via_reject_test_destroy_comp_channel_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, EAGAIN); /* first error */
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, EIO); /* second error */
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, EIO); /* third error */

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
}

/*
 * del_via_reject_test_reject_EAGAIN -- rdma_reject() fails with EAGAIN
 */
static void
del_via_reject_test_reject_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, EAGAIN);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
}

/*
 * del_via_reject_test_reject_EAGAIN_ack_EIO - rdma_ack_cm_event()
 * fails with EIO after rdma_reject() failed with EAGAIN
 */
static void
del_via_reject_test_reject_EAGAIN_ack_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, EAGAIN);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, EIO);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
}

/*
 * del_via_reject_test_ack_EAGAIN - rdma_ack_cm_event() fails with EAGAIN
 */
static void
del_via_reject_test_ack_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, EAGAIN);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
}

/*
 * del_via_destroy_test_destroy_cq_EAGAIN - ibv_destroy_cq()
 * fails with EAGAIN
 */
static void
del_via_destroy_test_destroy_cq_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(conn_req_new_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, EAGAIN);
	will_return(ibv_destroy_comp_channel, MOCK_OK);
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
}

/*
 * del_via_destroy_test_destroy_cq_EAGAIN_subsequent_EIO --
 * ibv_destroy_comp_channel() and rdma_destroy_id() fail with EIO after
 * ibv_destroy_cq() failed with EAGAIN
 */
static void
del_via_destroy_test_destroy_cq_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(conn_req_new_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, EAGAIN); /* first error */
	will_return(ibv_destroy_comp_channel, EIO); /* second error */
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, EIO); /* third error */

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
}

/*
 * del_via_destroy_test_destroy_comp_channel_EAGAIN --
 * ibv_destroy_comp_channel() fails with EAGAIN
 */
static void
del_via_destroy_test_destroy_comp_channel_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(conn_req_new_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, EAGAIN);
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
}

/*
 * del_via_destroy_test_destroy_comp_channel_EAGAIN_subsequent_EIO --
 * ibv_destroy_comp_channel() fails with EAGAIN whereas subsequent fail with EIO
 */
static void
del_via_destroy_test_destroy_comp_channel_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(conn_req_new_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, EAGAIN);
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, EIO);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
}

/*
 * del_via_destroy_test_destroy_id_EAGAIN - rdma_destroy_id()
 * fails with EAGAIN
 */
static void
del_via_destroy_test_destroy_id_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(conn_req_new_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, EAGAIN);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
}

/*
 * conn_test_req_ptr_NULL -- NULL req_ptr is invalid
 */
static void
conn_test_req_ptr_NULL(void **unused)
{
	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(NULL, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(conn);
}

/*
 * conn_test_conn_ptr_NULL -- NULL conn_ptr is invalid
 */
static void
conn_test_conn_ptr_NULL(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_req_connect(&cstate->req, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_non_null(cstate->req);
}

/*
 * conn_test_req_NULL -- NULL *req_ptr is invalid
 */
static void
conn_test_req_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(conn);
	assert_null(req);
}

/*
 * conn_test_pdata_NULL_pdata_ptr_NULL -- pdata->ptr == NULL is invalid
 */
static void
conn_test_pdata_NULL_pdata_ptr_NULL(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;

	/* run test */
	struct rpma_conn *conn = NULL;
	struct rpma_conn_private_data pdata = {NULL, 1};
	int ret = rpma_conn_req_connect(&cstate->req, &pdata, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_non_null(cstate->req);
	assert_null(conn);
}

/*
 * conn_test_pdata_NULL_pdata_len_0 -- pdata->len == 0 is invalid
 */
static void
conn_test_pdata_NULL_pdata_len_0(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;
	char buff = 0;

	/* run test */
	struct rpma_conn *conn = NULL;
	struct rpma_conn_private_data pdata = {&buff, 0};
	int ret = rpma_conn_req_connect(&cstate->req, &pdata, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_non_null(cstate->req);
	assert_null(conn);
}

/*
 * conn_test_pdata_NULL_pdata_ptr_NULL_len_0 -- pdata->ptr == NULL and
 * pdata->len == 0 are invalid
 */
static void
conn_test_pdata_NULL_pdata_ptr_NULL_len_0(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;

	/* run test */
	struct rpma_conn *conn = NULL;
	struct rpma_conn_private_data pdata = {NULL, 0};
	int ret = rpma_conn_req_connect(&cstate->req, &pdata, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_non_null(cstate->req);
	assert_null(conn);
}

/*
 * conn_via_accept_test_accept_EAGAIN -- rdma_accept() fails with EAGAIN
 */
static void
conn_via_accept_test_accept_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, EAGAIN);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_OK);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * conn_via_accept_test_accept_EAGAIN_subsequent_EIO -- rdma_accept()
 * fails with EAGAIN whereas subsequent (rdma_ack_cm_event(),
 * ibv_destroy_cq()) fail with EIO
 */
static void
conn_via_accept_test_accept_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, EAGAIN); /* first error */
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, EIO); /* second error */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, EIO); /* third error */
	will_return(ibv_destroy_comp_channel, EIO); /* fourth error */

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * conn_via_accept_test_ack_EAGAIN -- rdma_ack_cm_event() fails with EAGAIN
 */
static void
conn_via_accept_test_ack_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, EAGAIN);
	expect_value(rdma_disconnect, id, &cstate->id);
	will_return(rdma_disconnect, MOCK_OK);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * conn_via_accept_test_ack_EAGAIN_subsequent_EIO -- rdma_ack_cm_event()
 * fails with EAGAIN whereas subsequent (rdma_disconnect(), ibv_destroy_cq())
 * fail with EIO
 */
static void
conn_via_accept_test_ack_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, EAGAIN); /* first error */
	expect_value(rdma_disconnect, id, &cstate->id);
	will_return(rdma_disconnect, EIO); /* second error */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, EIO); /* third error */
	will_return(ibv_destroy_comp_channel, EIO); /* fourth error */

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * conn_via_accept_test_conn_new_EAGAIN -- rpma_conn_new() fails with
 * RPMA_E_PROVIDER + EAGAIN
 */
static void
conn_via_accept_test_conn_new_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_OK);
	expect_value(rpma_conn_new, id, &cstate->id);
	will_return(rpma_conn_new, NULL);
	will_return(rpma_conn_new, RPMA_E_PROVIDER);
	will_return(rpma_conn_new, EAGAIN);
	expect_value(rdma_disconnect, id, &cstate->id);
	will_return(rdma_disconnect, MOCK_OK);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * conn_via_accept_test_conn_new_EAGAIN_subsequent_EIO --
 * rpma_conn_new() fails with RPMA_E_PROVIDER + EAGAIN
 * whereas subsequent (rdma_disconnect(), ibv_destroy_cq()) fail with EIO
 */
static void
conn_via_accept_test_conn_new_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_OK);
	expect_value(rpma_conn_new, id, &cstate->id);
	will_return(rpma_conn_new, NULL);
	will_return(rpma_conn_new, RPMA_E_PROVIDER);
	will_return(rpma_conn_new, EAGAIN); /* first error */
	expect_value(rdma_disconnect, id, &cstate->id);
	will_return(rdma_disconnect, EIO); /* second error */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, EIO); /* third error */
	will_return(ibv_destroy_comp_channel, EIO); /* fourth error */

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * conn_via_accept_test_set_private_data_ENOMEM -- rpma_conn_set_private_data()
 * fails with ENOMEM
 */
static void
conn_via_accept_test_set_private_data_ENOMEM(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_OK);
	expect_value(rpma_conn_new, id, &cstate->id);
	will_return(rpma_conn_new, MOCK_CONN);
	expect_value(rpma_conn_set_private_data, conn, MOCK_CONN);
	expect_value(rpma_conn_set_private_data, pdata->ptr, MOCK_PRIVATE_DATA);
	expect_value(rpma_conn_set_private_data, pdata->len, MOCK_PDATA_LEN);
	will_return(rpma_conn_set_private_data, RPMA_E_NOMEM);
	expect_value(rpma_conn_delete, conn, MOCK_CONN);
	will_return(rpma_conn_delete, MOCK_OK);
	expect_value(rdma_disconnect, id, &cstate->id);
	will_return(rdma_disconnect, MOCK_OK);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * conn_via_accept_test_success_incoming -- rpma_conn_req_connect()
 * success (using an incoming connection request)
 */
static void
conn_via_accept_test_success_incoming(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_OK);
	expect_value(rpma_conn_new, id, &cstate->id);
	will_return(rpma_conn_new, MOCK_CONN);
	expect_value(rpma_conn_set_private_data, conn, MOCK_CONN);
	expect_value(rpma_conn_set_private_data, pdata->ptr, MOCK_PRIVATE_DATA);
	expect_value(rpma_conn_set_private_data, pdata->len, MOCK_PDATA_LEN);
	will_return(rpma_conn_set_private_data, 0);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cstate->req);
	assert_int_equal(conn, MOCK_CONN);
}

/*
 * conn_via_connect_test_connect_EAGAIN -- rdma_connect() fails with EAGAIN
 */
static void
conn_via_connect_test_connect_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(conn_req_new_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rpma_conn_new, id, &cstate->id);
	will_return(rpma_conn_new, MOCK_CONN);
	expect_value(rdma_connect, id, &cstate->id);
	will_return(rdma_connect, EAGAIN);
	expect_value(rpma_conn_delete, conn, MOCK_CONN);
	will_return(rpma_conn_delete, MOCK_OK);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * conn_via_connect_test_connect_EAGAIN_subsequent_EIO -- rdma_connect()
 * fails with EAGAIN whereas subsequent (ibv_destroy_cq(), rdma_destroy_id())
 * fail with EIO
 */
static void
conn_via_connect_test_connect_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(conn_req_new_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rpma_conn_new, id, &cstate->id);
	will_return(rpma_conn_new, MOCK_CONN);
	expect_value(rdma_connect, id, &cstate->id);
	will_return(rdma_connect, EAGAIN); /* first error */
	expect_value(rpma_conn_delete, conn, MOCK_CONN);
	will_return(rpma_conn_delete, RPMA_E_PROVIDER);
	will_return(rpma_conn_delete, EIO); /* second error */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, EIO); /* third error */
	will_return(ibv_destroy_comp_channel, EIO); /* fourth error */
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, EIO); /* fifth error */

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * conn_via_connect_test_conn_new_EAGAIN -- rpma_conn_new() fails with
 * RPMA_E_PROVIDER + EAGAIN
 */
static void
conn_via_connect_test_conn_new_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(conn_req_new_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rpma_conn_new, id, &cstate->id);
	will_return(rpma_conn_new, NULL);
	will_return(rpma_conn_new, RPMA_E_PROVIDER);
	will_return(rpma_conn_new, EAGAIN);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * conn_via_connect_test_conn_new_EAGAIN_subsequent_EIO --
 * rpma_conn_new() fails with RPMA_E_PROVIDER + EAGAIN whereas subsequent
 * (rdma_disconnect(), ibv_destroy_cq(), rdma_destroy_id()) fail with EIO
 */
static void
conn_via_connect_test_conn_new_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(conn_req_new_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rpma_conn_new, id, &cstate->id);
	will_return(rpma_conn_new, NULL);
	will_return(rpma_conn_new, RPMA_E_PROVIDER);
	will_return(rpma_conn_new, EAGAIN); /* first error */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, EIO); /* second error */
	will_return(ibv_destroy_comp_channel, EIO); /* third error */
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, EIO); /* fourth error */

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * conn_via_connect_test_success_outgoing -- rpma_conn_req_connect()
 * success (using an outgoing connection request)
 */
static void
conn_via_connect_test_success_outgoing(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(conn_req_new_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_connect, id, &cstate->id);
	will_return(rdma_connect, MOCK_OK);
	expect_value(rpma_conn_new, id, &cstate->id);
	will_return(rpma_conn_new, MOCK_CONN);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cstate->req);
	assert_int_equal(conn, MOCK_CONN);
}

/*
 * group_setup_conn_req -- prepare resources for all tests in the group
 */
int
group_setup_conn_req(void **unused)
{
	/* set the req_notify_cq callback in mock of IBV CQ */
	Ibv_context.ops.req_notify_cq = ibv_req_notify_cq_mock;
	Ibv_cq.context = &Ibv_context;

	return 0;
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_conn_req_from_cm_event() unit tests */
		cmocka_unit_test(from_cm_event_test_peer_NULL),
		cmocka_unit_test(from_cm_event_test_edata_NULL),
		cmocka_unit_test(from_cm_event_test_req_ptr_NULL),
		cmocka_unit_test(
			from_cm_event_test_peer_NULL_edata_NULL_req_ptr_NULL),
		cmocka_unit_test(
			from_cm_event_test_RDMA_CM_EVENT_CONNECT_ERROR),
		cmocka_unit_test(from_cm_event_test_create_comp_channel_EAGAIN),
		cmocka_unit_test(from_cm_event_test_create_cq_EAGAIN),
		cmocka_unit_test(
			from_cm_event_test_peer_create_qp_E_PROVIDER_EAGAIN),
		cmocka_unit_test(
			from_cm_event_test_create_qp_EAGAIN_subsequent_EIO),
		cmocka_unit_test(from_cm_event_test_malloc_ENOMEM),
		cmocka_unit_test(
			from_cm_event_test_malloc_ENOMEM_subsequent_EAGAIN),
		cmocka_unit_test(
			from_cm_event_test_private_data_store_ENOMEM),

		/* rpma_conn_req_from_cm_event()/_delete() lifecycle */
		cmocka_unit_test_setup_teardown(conn_req_from_cm_test_lifecycle,
			conn_req_from_cm_event_setup,
			conn_req_from_cm_event_teardown),

		/* rpma_conn_req_new() unit tests */
		cmocka_unit_test(new_test_peer_NULL),
		cmocka_unit_test(new_test_addr_NULL),
		cmocka_unit_test(new_test_service_NULL),
		cmocka_unit_test(new_test_req_ptr_NULL),
		cmocka_unit_test(new_test_all_NULL),
		cmocka_unit_test(new_test_info_new_E_PROVIDER_EAGAIN),
		cmocka_unit_test(new_test_create_id_EAGAIN),
		cmocka_unit_test(new_test_resolve_addr_E_PROVIDER_EAGAIN),
		cmocka_unit_test(new_test_resolve_route_EAGAIN),
		cmocka_unit_test(new_test_create_comp_channel_EAGAIN),
		cmocka_unit_test(new_test_create_cq_EAGAIN),
		cmocka_unit_test(new_test_peer_create_qp_E_PROVIDER_EAGAIN),
		cmocka_unit_test(new_test_malloc_ENOMEM),
		cmocka_unit_test(new_test_malloc_ENOMEM_subsequent_EAGAIN),
		cmocka_unit_test_setup_teardown(new_test_success,
				conn_req_new_setup, conn_req_new_teardown),

		/* rpma_conn_req_delete() unit tests */
		cmocka_unit_test(del_test_req_ptr_NULL),
		cmocka_unit_test(del_test_req_NULL),
		/* delete via rdma_reject() */
		cmocka_unit_test(del_via_reject_test_destroy_cq_EAGAIN),
		cmocka_unit_test(
			del_via_reject_test_destroy_cq_EAGAIN_subsequent_EIO),
		cmocka_unit_test(
			del_via_reject_test_destroy_comp_channel_EAGAIN),
		cmocka_unit_test(
		del_via_reject_test_destroy_comp_channel_EAGAIN_subsequent_EIO),
		cmocka_unit_test(del_via_reject_test_reject_EAGAIN),
		cmocka_unit_test(del_via_reject_test_reject_EAGAIN_ack_EIO),
		cmocka_unit_test(del_via_reject_test_ack_EAGAIN),
		/* delete via rdma_destroy_id() */
		cmocka_unit_test(del_via_destroy_test_destroy_cq_EAGAIN),
		cmocka_unit_test(
			del_via_destroy_test_destroy_cq_EAGAIN_subsequent_EIO),
		cmocka_unit_test(
			del_via_destroy_test_destroy_comp_channel_EAGAIN),
		cmocka_unit_test(
	del_via_destroy_test_destroy_comp_channel_EAGAIN_subsequent_EIO),
		cmocka_unit_test(del_via_destroy_test_destroy_id_EAGAIN),

		/* rpma_conn_req_connect() unit tests */
		cmocka_unit_test(conn_test_req_ptr_NULL),
		cmocka_unit_test_setup_teardown(conn_test_conn_ptr_NULL,
			conn_req_from_cm_event_setup,
			conn_req_from_cm_event_teardown),
		cmocka_unit_test(conn_test_req_NULL),
		cmocka_unit_test_setup_teardown(
			conn_test_pdata_NULL_pdata_ptr_NULL,
			conn_req_from_cm_event_setup,
			conn_req_from_cm_event_teardown),
		cmocka_unit_test_setup_teardown(
			conn_test_pdata_NULL_pdata_len_0,
			conn_req_from_cm_event_setup,
			conn_req_from_cm_event_teardown),
		cmocka_unit_test_setup_teardown(
			conn_test_pdata_NULL_pdata_ptr_NULL_len_0,
			conn_req_from_cm_event_setup,
			conn_req_from_cm_event_teardown),
		/* connect via rdma_accept() */
		cmocka_unit_test(conn_via_accept_test_accept_EAGAIN),
		cmocka_unit_test(
			conn_via_accept_test_accept_EAGAIN_subsequent_EIO),
		cmocka_unit_test(conn_via_accept_test_ack_EAGAIN),
		cmocka_unit_test(
			conn_via_accept_test_ack_EAGAIN_subsequent_EIO),
		cmocka_unit_test(
			conn_via_accept_test_conn_new_EAGAIN),
		cmocka_unit_test(
			conn_via_accept_test_conn_new_EAGAIN_subsequent_EIO),
		cmocka_unit_test(conn_via_accept_test_set_private_data_ENOMEM),
		cmocka_unit_test(conn_via_accept_test_success_incoming),
		/* connect via rdma_connect() */
		cmocka_unit_test(conn_via_connect_test_connect_EAGAIN),
		cmocka_unit_test(
			conn_via_connect_test_connect_EAGAIN_subsequent_EIO),
		cmocka_unit_test(conn_via_connect_test_conn_new_EAGAIN),
		cmocka_unit_test(
			conn_via_connect_test_conn_new_EAGAIN_subsequent_EIO),
		cmocka_unit_test(conn_via_connect_test_success_outgoing),
	};

	return cmocka_run_group_tests(tests, group_setup_conn_req, NULL);
}
