// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn_req-connect.c -- the rpma_conn_req_connect() unit tests
 *
 * API covered:
 * - rpma_conn_req_connect()
 */

#include "conn_req-common.h"
#include "test-common.h"

/*
 * connect__req_ptr_NULL -- NULL req_ptr is invalid
 */
static void
connect__req_ptr_NULL(void **unused)
{
	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(NULL, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(conn);
}

/*
 * connect__conn_ptr_NULL -- NULL conn_ptr is invalid
 */
static void
connect__conn_ptr_NULL(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_req_connect(&cstate->req, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_non_null(cstate->req);
}

/*
 * connect__req_NULL -- NULL *req_ptr is invalid
 */
static void
connect__req_NULL(void **unused)
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
 * connect__pdata_NULL_pdata_ptr_NULL -- pdata->ptr == NULL is invalid
 */
static void
connect__pdata_NULL_pdata_ptr_NULL(void **cstate_ptr)
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
 * connect__pdata_NULL_pdata_len_0 -- pdata->len == 0 is invalid
 */
static void
connect__pdata_NULL_pdata_len_0(void **cstate_ptr)
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
 * connect__pdata_NULL_pdata_ptr_NULL_len_0 -- pdata->ptr == NULL and
 * pdata->len == 0 are invalid
 */
static void
connect__pdata_NULL_pdata_ptr_NULL_len_0(void **cstate_ptr)
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
 * connect_via_accept__accept_ERRNO -- rdma_accept() fails with MOCK_ERRNO
 */
static void
connect_via_accept__accept_ERRNO(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, MOCK_ERRNO);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_OK);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_via_accept__accept_ERRNO_subsequent_ERRNO2 -- rdma_accept()
 * fails with MOCK_ERRNO whereas subsequent (rdma_ack_cm_event(),
 * rpma_cq_delete(&req->rcq), rpma_cq_delete(&req->cq)) fail with MOCK_ERRNO2
 */
static void
connect_via_accept__accept_ERRNO_subsequent_ERRNO2(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, MOCK_ERRNO); /* first error */
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_ERRNO2); /* second error */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	if (cstate->get_cqe.rcq_size) {
		will_return(rpma_cq_delete, RPMA_E_PROVIDER);
		will_return(rpma_cq_delete, MOCK_ERRNO2); /* third error */
	} else {
		/* rcq == NULL cannot fail */
		will_return(rpma_cq_delete, MOCK_OK);
	}
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO2); /* third or fourth error */

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_via_accept__ack_ERRNO -- rdma_ack_cm_event() fails with MOCK_ERRNO
 */
static void
connect_via_accept__ack_ERRNO(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_ERRNO);
	expect_value(rdma_disconnect, id, &cstate->id);
	will_return(rdma_disconnect, MOCK_OK);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_via_accept__ack_ERRNO_subsequent_ERRNO2 -- rdma_ack_cm_event()
 * fails with MOCK_ERRNO whereas subsequent (rdma_disconnect(),
 * rpma_cq_delete(&req->rcq), rpma_cq_delete(&req->cq)) fail with MOCK_ERRNO2
 */
static void
connect_via_accept__ack_ERRNO_subsequent_ERRNO2(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_ERRNO); /* first error */
	expect_value(rdma_disconnect, id, &cstate->id);
	will_return(rdma_disconnect, MOCK_ERRNO2); /* second error */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	if (cstate->get_cqe.rcq_size) {
		will_return(rpma_cq_delete, RPMA_E_PROVIDER);
		will_return(rpma_cq_delete, MOCK_ERRNO2); /* third error */
	} else {
		/* rcq == NULL cannot fail */
		will_return(rpma_cq_delete, MOCK_OK);
	}
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO2); /* third or fourth error */

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_via_accept__conn_new_ERRNO -- rpma_conn_new() fails with
 * MOCK_ERRNO
 */
static void
connect_via_accept__conn_new_ERRNO(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_OK);
	expect_value(rpma_conn_new, id, &cstate->id);
	expect_value(rpma_conn_new, rcq, MOCK_GET_RCQ(cstate));
	will_return(rpma_conn_new, NULL);
	will_return(rpma_conn_new, RPMA_E_PROVIDER);
	will_return(rpma_conn_new, MOCK_ERRNO);
	expect_value(rdma_disconnect, id, &cstate->id);
	will_return(rdma_disconnect, MOCK_OK);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_via_accept__conn_new_ERRNO_subsequent_ERRNO2 --
 * rpma_conn_new() fails with MOCK_ERRNO whereas subsequent (rdma_disconnect(),
 * rpma_cq_delete(&req->rcq), rpma_cq_delete(&req->cq)) fail with MOCK_ERRNO2
 */
static void
connect_via_accept__conn_new_ERRNO_subsequent_ERRNO2(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_OK);
	expect_value(rpma_conn_new, id, &cstate->id);
	expect_value(rpma_conn_new, rcq, MOCK_GET_RCQ(cstate));
	will_return(rpma_conn_new, NULL);
	will_return(rpma_conn_new, RPMA_E_PROVIDER);
	will_return(rpma_conn_new, MOCK_ERRNO); /* first error */
	expect_value(rdma_disconnect, id, &cstate->id);
	will_return(rdma_disconnect, MOCK_ERRNO2); /* second error */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	if (cstate->get_cqe.rcq_size) {
		will_return(rpma_cq_delete, RPMA_E_PROVIDER);
		will_return(rpma_cq_delete, MOCK_ERRNO2); /* third error */
	} else {
		/* rcq == NULL cannot fail */
		will_return(rpma_cq_delete, MOCK_OK);
	}
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO2); /* third or fourth error */

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_via_accept__success_incoming -- rpma_conn_req_connect()
 * success (using an incoming connection request)
 */
static void
connect_via_accept__success_incoming(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_OK);
	expect_value(rpma_conn_new, id, &cstate->id);
	expect_value(rpma_conn_new, rcq, MOCK_GET_RCQ(cstate));
	will_return(rpma_conn_new, MOCK_CONN);
	expect_value(rpma_conn_transfer_private_data, conn, MOCK_CONN);
	expect_value(rpma_conn_transfer_private_data, pdata->ptr,
				MOCK_PRIVATE_DATA);
	expect_value(rpma_conn_transfer_private_data, pdata->len,
				MOCK_PDATA_LEN);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cstate->req);
	assert_int_equal(conn, MOCK_CONN);
}

/*
 * connect_via_connect__connect_ERRNO -- rdma_connect() fails with MOCK_ERRNO
 */
static void
connect_via_connect__connect_ERRNO(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rpma_conn_new, id, &cstate->id);
	expect_value(rpma_conn_new, rcq, MOCK_GET_RCQ(cstate));
	will_return(rpma_conn_new, MOCK_CONN);
	expect_value(rdma_connect, id, &cstate->id);
	will_return(rdma_connect, MOCK_ERRNO);
	expect_value(rpma_conn_delete, conn, MOCK_CONN);
	will_return(rpma_conn_delete, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_via_connect__connect_ERRNO_subsequent_ERRNO2 -- rpma_conn_delete()
 * fails with MOCK_ERRNO2 after rdma_connect() failed with MOCK_ERRNO
 */
static void
connect_via_connect__connect_ERRNO_subsequent_ERRNO2(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rpma_conn_new, id, &cstate->id);
	expect_value(rpma_conn_new, rcq, MOCK_GET_RCQ(cstate));
	will_return(rpma_conn_new, MOCK_CONN);
	expect_value(rdma_connect, id, &cstate->id);
	will_return(rdma_connect, MOCK_ERRNO); /* first error */
	expect_value(rpma_conn_delete, conn, MOCK_CONN);
	will_return(rpma_conn_delete, RPMA_E_PROVIDER);
	will_return(rpma_conn_delete, MOCK_ERRNO2); /* second error */

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_via_connect__conn_new_ERRNO -- rpma_conn_new() fails with
 * MOCK_ERRNO
 */
static void
connect_via_connect__conn_new_ERRNO(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rpma_conn_new, id, &cstate->id);
	expect_value(rpma_conn_new, rcq, MOCK_GET_RCQ(cstate));
	will_return(rpma_conn_new, NULL);
	will_return(rpma_conn_new, RPMA_E_PROVIDER);
	will_return(rpma_conn_new, MOCK_ERRNO);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_via_connect__conn_new_ERRNO_subsequent_ERRNO2 --
 * rpma_conn_new() fails with MOCK_ERRNO whereas subsequent
 * (rdma_disconnect(), rpma_cq_delete(&req->rcq), rpma_cq_delete(&req->cq),
 * rdma_destroy_id()) fail with MOCK_ERRNO2
 */
static void
connect_via_connect__conn_new_ERRNO_subsequent_ERRNO2(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rpma_conn_new, id, &cstate->id);
	expect_value(rpma_conn_new, rcq, MOCK_GET_RCQ(cstate));
	will_return(rpma_conn_new, NULL);
	will_return(rpma_conn_new, RPMA_E_PROVIDER);
	will_return(rpma_conn_new, MOCK_ERRNO); /* first error */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	if (cstate->get_cqe.rcq_size) {
		will_return(rpma_cq_delete, RPMA_E_PROVIDER);
		will_return(rpma_cq_delete, MOCK_ERRNO2); /* second error */
	} else {
		/* rcq == NULL cannot fail */
		will_return(rpma_cq_delete, MOCK_OK);
	}
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO2); /* second or third error */
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, MOCK_ERRNO2); /* third or fourth error */

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_via_connect__success_outgoing -- rpma_conn_req_connect()
 * success (using an outgoing connection request)
 */
static void
connect_via_connect__success_outgoing(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_connect, id, &cstate->id);
	will_return(rdma_connect, MOCK_OK);
	expect_value(rpma_conn_new, id, &cstate->id);
	expect_value(rpma_conn_new, rcq, MOCK_GET_RCQ(cstate));
	will_return(rpma_conn_new, MOCK_CONN);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cstate->req);
	assert_int_equal(conn, MOCK_CONN);
}

static const struct CMUnitTest test_connect[] = {
	/* rpma_conn_req_connect() unit tests */
	cmocka_unit_test(connect__req_ptr_NULL),
	cmocka_unit_test_setup_teardown(
		connect__conn_ptr_NULL,
		setup__conn_req_from_cm_event,
		teardown__conn_req_from_cm_event),
	cmocka_unit_test(connect__req_NULL),
	cmocka_unit_test_setup_teardown(
		connect__pdata_NULL_pdata_ptr_NULL,
		setup__conn_req_from_cm_event,
		teardown__conn_req_from_cm_event),
	cmocka_unit_test_setup_teardown(
		connect__pdata_NULL_pdata_len_0,
		setup__conn_req_from_cm_event,
		teardown__conn_req_from_cm_event),
	cmocka_unit_test_setup_teardown(
		connect__pdata_NULL_pdata_ptr_NULL_len_0,
		setup__conn_req_from_cm_event,
		teardown__conn_req_from_cm_event),
	/* connect via rdma_accept() */
	CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(connect_via_accept__accept_ERRNO),
	CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(
		connect_via_accept__accept_ERRNO_subsequent_ERRNO2),
	CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(connect_via_accept__ack_ERRNO),
	CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(
		connect_via_accept__ack_ERRNO_subsequent_ERRNO2),
	CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(connect_via_accept__conn_new_ERRNO),
	CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(
		connect_via_accept__conn_new_ERRNO_subsequent_ERRNO2),
	CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(
		connect_via_accept__success_incoming),
	/* connect via rdma_connect() */
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_RCQ(
		connect_via_connect__connect_ERRNO),
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_RCQ(
		connect_via_connect__connect_ERRNO_subsequent_ERRNO2),
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_RCQ(
		connect_via_connect__conn_new_ERRNO),
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_RCQ(
		connect_via_connect__conn_new_ERRNO_subsequent_ERRNO2),
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_RCQ(
		connect_via_connect__success_outgoing),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_connect, NULL, NULL);
}
