// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn_req-connect.c -- the rpma_conn_req_connect() unit tests
 *
 * APIs covered:
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
 * connect_via_accept__accept_EAGAIN -- rdma_accept() fails with EAGAIN
 */
static void
connect_via_accept__accept_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
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
 * connect_via_accept__accept_EAGAIN_subsequent_EIO -- rdma_accept()
 * fails with EAGAIN whereas subsequent (rdma_ack_cm_event(),
 * ibv_destroy_cq()) fail with EIO
 */
static void
connect_via_accept__accept_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
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
 * connect_via_accept__ack_EAGAIN -- rdma_ack_cm_event() fails with EAGAIN
 */
static void
connect_via_accept__ack_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
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
 * connect_via_accept__ack_EAGAIN_subsequent_EIO -- rdma_ack_cm_event()
 * fails with EAGAIN whereas subsequent (rdma_disconnect(), ibv_destroy_cq())
 * fail with EIO
 */
static void
connect_via_accept__ack_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
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
 * connect_via_accept__conn_new_EAGAIN -- rpma_conn_new() fails with
 * RPMA_E_PROVIDER + EAGAIN
 */
static void
connect_via_accept__conn_new_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
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
 * connect_via_accept__conn_new_EAGAIN_subsequent_EIO --
 * rpma_conn_new() fails with RPMA_E_PROVIDER + EAGAIN
 * whereas subsequent (rdma_disconnect(), ibv_destroy_cq()) fail with EIO
 */
static void
connect_via_accept__conn_new_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
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
 * connect_via_accept__set_private_data_ENOMEM -- rpma_conn_set_private_data()
 * fails with ENOMEM
 */
static void
connect_via_accept__set_private_data_ENOMEM(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
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
 * connect_via_accept__success_incoming -- rpma_conn_req_connect()
 * success (using an incoming connection request)
 */
static void
connect_via_accept__success_incoming(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
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
 * connect_via_connect__connect_EAGAIN -- rdma_connect() fails with EAGAIN
 */
static void
connect_via_connect__connect_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
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
 * connect_via_connect__connect_EAGAIN_subsequent_EIO -- rdma_connect()
 * fails with EAGAIN whereas subsequent (ibv_destroy_cq(), rdma_destroy_id())
 * fail with EIO
 */
static void
connect_via_connect__connect_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
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
 * connect_via_connect__conn_new_EAGAIN -- rpma_conn_new() fails with
 * RPMA_E_PROVIDER + EAGAIN
 */
static void
connect_via_connect__conn_new_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
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
 * connect_via_connect__conn_new_EAGAIN_subsequent_EIO --
 * rpma_conn_new() fails with RPMA_E_PROVIDER + EAGAIN whereas subsequent
 * (rdma_disconnect(), ibv_destroy_cq(), rdma_destroy_id()) fail with EIO
 */
static void
connect_via_connect__conn_new_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
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
 * connect_via_connect__success_outgoing -- rpma_conn_req_connect()
 * success (using an outgoing connection request)
 */
static void
connect_via_connect__success_outgoing(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
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

const struct CMUnitTest test_connect[] = {
	/* rpma_conn_req_connect() unit tests */
	cmocka_unit_test(connect__req_ptr_NULL),
	cmocka_unit_test_setup_teardown(connect__conn_ptr_NULL,
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
	cmocka_unit_test(connect_via_accept__accept_EAGAIN),
	cmocka_unit_test(
		connect_via_accept__accept_EAGAIN_subsequent_EIO),
	cmocka_unit_test(connect_via_accept__ack_EAGAIN),
	cmocka_unit_test(
		connect_via_accept__ack_EAGAIN_subsequent_EIO),
	cmocka_unit_test(
		connect_via_accept__conn_new_EAGAIN),
	cmocka_unit_test(
		connect_via_accept__conn_new_EAGAIN_subsequent_EIO),
	cmocka_unit_test(connect_via_accept__set_private_data_ENOMEM),
	cmocka_unit_test(connect_via_accept__success_incoming),
	/* connect via rdma_connect() */
	cmocka_unit_test(connect_via_connect__connect_EAGAIN),
	cmocka_unit_test(
		connect_via_connect__connect_EAGAIN_subsequent_EIO),
	cmocka_unit_test(connect_via_connect__conn_new_EAGAIN),
	cmocka_unit_test(
		connect_via_connect__conn_new_EAGAIN_subsequent_EIO),
	cmocka_unit_test(connect_via_connect__success_outgoing),
	cmocka_unit_test(NULL)
};
