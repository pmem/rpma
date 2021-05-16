// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn_req-private_data.c -- the rpma_conn_req_get_private_data() unit tests
 *
 * API covered:
 * - rpma_conn_req_get_private_data()
 */

#include "conn_req-common.h"

/*
 * get_private_data__success -- happy day scenario
 */
static void
get_private_data__success(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_OK);
	expect_function_call(rpma_private_data_discard);

	/* run test */
	struct rpma_conn_private_data data;
	int ret = rpma_conn_req_get_private_data(cstate->req, &data);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(data.ptr, MOCK_PRIVATE_DATA);
	assert_int_equal(data.len, MOCK_PDATA_LEN);

	/* clean up test */
	rpma_conn_req_delete(&cstate->req);
}

/*
 * get_private_data__conn_req_NULL -- conn_req NULL is invalid
 */
static void
get_private_data__conn_req_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_private_data data;
	int ret = rpma_conn_req_get_private_data(NULL, &data);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_private_data__pdata_NULL -- pdata NULL is invalid
 */
static void
get_private_data__pdata_NULL(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_OK);
	expect_function_call(rpma_private_data_discard);

	/* run test */
	int ret = rpma_conn_req_get_private_data(cstate->req, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);

	/* clean up test */
	rpma_conn_req_delete(&cstate->req);
}

static const struct CMUnitTest test_private_data[] = {
	cmocka_unit_test(get_private_data__success),
	cmocka_unit_test(get_private_data__conn_req_NULL),
	cmocka_unit_test(get_private_data__pdata_NULL),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_private_data, NULL, NULL);
}
