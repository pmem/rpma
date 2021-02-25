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

static struct conn_req_new_test_state prestate_conn_cfg_default;

/*
 * connect_via_accept_wi_pdata__success --
 * rpma_conn_req_get_private_data() success
 */
static void
connect_via_accept_wi_pdata__success_incoming(void **unused)
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
	expect_function_call(rpma_private_data_discard);

	/* get private data */
	struct rpma_conn_private_data data;
	int ret = rpma_conn_req_get_private_data(cstate->req, &data);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(data.ptr, MOCK_PRIVATE_DATA);
	assert_int_equal(data.len, MOCK_PDATA_LEN);

	/* run test */
	struct rpma_conn *conn = NULL;
	ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cstate->req);
	assert_int_equal(conn, MOCK_CONN);
}

/*
 * connect_via_accept_wi_pdata__null_req -- rpma_conn_req_get_private_data()
 * failed (using a null connection request)
 */
static void
connect_via_accept_wi_pdata__null_req(void **unused)
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
	expect_function_call(rpma_private_data_discard);

	/* get private data */
	struct rpma_conn_private_data data;
	int ret = rpma_conn_req_get_private_data(NULL, &data);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);

	/* run test */
	struct rpma_conn *conn = NULL;
	ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cstate->req);
	assert_int_equal(conn, MOCK_CONN);
}

/*
 * connect_via_accept_wi_pdata__null_req -- rpma_conn_req_get_private_data()
 * failed (using a null pdata)
 */
static void
connect_via_accept_wi_pdata__null_pdata(void **unused)
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
	expect_function_call(rpma_private_data_discard);

	/* get private data */
	struct rpma_conn_private_data data;
	int ret = rpma_conn_req_get_private_data(cstate->req, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);

	/* run test */
	struct rpma_conn *conn = NULL;
	ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cstate->req);
	assert_int_equal(conn, MOCK_CONN);
}

static const struct CMUnitTest test_private_data[] = {
	cmocka_unit_test(connect_via_accept_wi_pdata__success_incoming),
	cmocka_unit_test(connect_via_accept_wi_pdata__null_req),
	cmocka_unit_test(connect_via_accept_wi_pdata__null_pdata),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	/* prepare prestate - default conn_cfg */
	prestate_init(&prestate_conn_cfg_default, MOCK_CONN_CFG_DEFAULT,
			RPMA_DEFAULT_TIMEOUT_MS, MOCK_CQ_SIZE_DEFAULT);

	return cmocka_run_group_tests(test_private_data, group_setup_conn_req,
			NULL);
}
