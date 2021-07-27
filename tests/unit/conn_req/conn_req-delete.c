// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn_req-delete.c -- the rpma_conn_req_delete() unit tests
 *
 * API covered:
 * - rpma_conn_req_delete()
 */

#include "conn_req-common.h"

/*
 * delete__req_ptr_NULL -- NULL req_ptr is invalid
 */
static void
delete__req_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_req_delete(NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * delete__req_NULL -- NULL req is valid - quick exit
 */
static void
delete__req_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_delete(&req);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * delete_via_reject__rcq_delete_ERRNO - rpma_cq_delete(&req->rcq)
 * fails with MOCK_ERRNO
 */
static void
delete_via_reject__rcq_delete_ERRNO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = &Conn_req_conn_cfg_custom;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_RCQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_OK);
	expect_function_call(rpma_private_data_discard);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
}

/*
 * delete_via_reject__rcq_delete_ERRNO_subsequent_ERRNO2 -
 * rpma_cq_delete(&req->rcq) fails with MOCK_ERRNO whereas subsequent
 * (rpma_cq_delete(&req->cq), rdma_reject(), rdma_ack_cm_event())
 * fail with MOCK_ERRNO2
 */
static void
delete_via_reject__rcq_delete_ERRNO_subsequent_ERRNO2(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = &Conn_req_conn_cfg_custom;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_RCQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO); /* first error */
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO2); /* second error */
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, MOCK_ERRNO2); /* third error */
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_ERRNO2); /* fourth error */
	expect_function_call(rpma_private_data_discard);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
}

/*
 * delete_via_reject__cq_delete_ERRNO - rpma_cq_delete(&req->cq)
 * fails with MOCK_ERRNO
 */
static void
delete_via_reject__cq_delete_ERRNO(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_OK);
	expect_function_call(rpma_private_data_discard);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
}

/*
 * delete_via_reject__cq_delete_ERRNO_subsequent_ERRNO2 -
 * rpma_cq_delete() fails with MOCK_ERRNO whereas subsequent
 * (rdma_reject(), rdma_ack_cm_event()) fail with MOCK_ERRNO2
 */
static void
delete_via_reject__cq_delete_ERRNO_subsequent_ERRNO2(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO); /* first error */
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, MOCK_ERRNO2); /* second error */
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_ERRNO2); /* third error */
	expect_function_call(rpma_private_data_discard);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
}

/*
 * delete_via_reject__reject_ERRNO -- rdma_reject() fails with MOCK_ERRNO
 */
static void
delete_via_reject__reject_ERRNO(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, MOCK_ERRNO);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_OK);
	expect_function_call(rpma_private_data_discard);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
}

/*
 * delete_via_reject__reject_ERRNO_ack_ERRNO2 - rdma_ack_cm_event()
 * fails with MOCK_ERRNO2 after rdma_reject() failed with MOCK_ERRNO
 */
static void
delete_via_reject__reject_ERRNO_ack_ERRNO2(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, MOCK_ERRNO); /* first error */
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_ERRNO2); /* second error */
	expect_function_call(rpma_private_data_discard);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
}

/*
 * delete_via_reject__ack_ERRNO - rdma_ack_cm_event() fails with MOCK_ERRNO
 */
static void
delete_via_reject__ack_ERRNO(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_ERRNO);
	expect_function_call(rpma_private_data_discard);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
}

/*
 * delete_via_destroy__rcq_delete_ERRNO - rpma_cq_delete(&req->rcq)
 * fails with MOCK_ERRNO
 */
static void
delete_via_destroy__rcq_delete_ERRNO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = &Conn_req_new_conn_cfg_custom;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_RCQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, MOCK_OK);
	expect_function_call(rpma_private_data_discard);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
}

/*
 * delete_via_destroy__rcq_delete_ERRNO_subsequent_ERRNO2 --
 * rpma_cq_delete(&req->rcq) fails with MOCK_ERRNO whereas subsequent
 * (rpma_cq_delete(&req->cq), rdma_destroy_id()) fail with MOCK_ERRNO2
 */
static void
delete_via_destroy__rcq_delete_ERRNO_subsequent_ERRNO2(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = &Conn_req_new_conn_cfg_custom;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_RCQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO); /* first error */
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO2); /* second error */
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, MOCK_ERRNO2); /* third error */
	expect_function_call(rpma_private_data_discard);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
}

/*
 * delete_via_destroy__cq_delete_ERRNO - rpma_cq_delete(&req->cq)
 * fails with MOCK_ERRNO
 */
static void
delete_via_destroy__cq_delete_ERRNO(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO);
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, MOCK_OK);
	expect_function_call(rpma_private_data_discard);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
}

/*
 * delete_via_destroy__cq_delete_ERRNO_subsequent_ERRNO2 --
 * rdma_destroy_id() fails with MOCK_ERRNO2 after rpma_cq_delete(&req->cq)
 * failed with MOCK_ERRNO
 */
static void
delete_via_destroy__cq_delete_ERRNO_subsequent_ERRNO2(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO); /* first error */
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, MOCK_ERRNO2); /* second error */
	expect_function_call(rpma_private_data_discard);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
}

/*
 * delete_via_destroy__destroy_id_ERRNO - rdma_destroy_id()
 * fails with MOCK_ERRNO
 */
static void
delete_via_destroy__destroy_id_ERRNO(void **cstate_ptr)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_RCQ(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, MOCK_ERRNO);
	expect_function_call(rpma_private_data_discard);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
}

static const struct CMUnitTest test_delete[] = {
	/* rpma_conn_req_delete() unit tests */
	cmocka_unit_test(delete__req_ptr_NULL),
	cmocka_unit_test(delete__req_NULL),
	/* delete via rdma_reject() */
	cmocka_unit_test(delete_via_reject__rcq_delete_ERRNO),
	cmocka_unit_test(
		delete_via_reject__rcq_delete_ERRNO_subsequent_ERRNO2),
	CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(
		delete_via_reject__cq_delete_ERRNO),
	CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(
		delete_via_reject__cq_delete_ERRNO_subsequent_ERRNO2),
	CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(delete_via_reject__reject_ERRNO),
	CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(
		delete_via_reject__reject_ERRNO_ack_ERRNO2),
	CONN_REQ_TEST_WITH_AND_WITHOUT_RCQ(delete_via_reject__ack_ERRNO),
	/* delete via rdma_destroy_id() */
	cmocka_unit_test(delete_via_destroy__rcq_delete_ERRNO),
	cmocka_unit_test(
		delete_via_destroy__rcq_delete_ERRNO_subsequent_ERRNO2),
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_RCQ(
		delete_via_destroy__cq_delete_ERRNO),
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_RCQ(
		delete_via_destroy__cq_delete_ERRNO_subsequent_ERRNO2),
	CONN_REQ_NEW_TEST_WITH_AND_WITHOUT_RCQ(
		delete_via_destroy__destroy_id_ERRNO),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_delete, NULL, NULL);
}
