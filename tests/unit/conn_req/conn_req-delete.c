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

static struct conn_req_new_test_state prestate_conn_cfg_default;

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
 * delete_via_reject__cq_delete_ERRNO - rpma_cq_delete() fails with MOCK_ERRNO
 */
static void
delete_via_reject__cq_delete_ERRNO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
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
 * rpma_cq_delete() fails with MOCK_ERRNO whereas subsequent (rdma_reject(),
 * rdma_ack_cm_event()) fail with MOCK_ERRNO2
 */
static void
delete_via_reject__cq_delete_ERRNO_subsequent_ERRNO2(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
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
delete_via_reject__reject_ERRNO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
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
delete_via_reject__reject_ERRNO_ack_ERRNO2(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, MOCK_ERRNO);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, MOCK_ERRNO2);
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
delete_via_reject__ack_ERRNO(void **unused)
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
	will_return(rdma_ack_cm_event, MOCK_ERRNO);
	expect_function_call(rpma_private_data_discard);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->req);
}

/*
 * delete_via_destroy__cq_delete_ERRNO - rpma_cq_delete()
 * fails with MOCK_ERRNO
 */
static void
delete_via_destroy__cq_delete_ERRNO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = &prestate_conn_cfg_default;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
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
 * rdma_destroy_id() fails with MOCK_ERRNO2 after rpma_cq_delete()
 * failed with MOCK_ERRNO
 */
static void
delete_via_destroy__cq_delete_ERRNO_subsequent_ERRNO2(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = &prestate_conn_cfg_default;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
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
delete_via_destroy__destroy_id_ERRNO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = &prestate_conn_cfg_default;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
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
	cmocka_unit_test(delete_via_reject__cq_delete_ERRNO),
	cmocka_unit_test(
		delete_via_reject__cq_delete_ERRNO_subsequent_ERRNO2),
	cmocka_unit_test(delete_via_reject__reject_ERRNO),
	cmocka_unit_test(delete_via_reject__reject_ERRNO_ack_ERRNO2),
	cmocka_unit_test(delete_via_reject__ack_ERRNO),
	/* delete via rdma_destroy_id() */
	cmocka_unit_test(delete_via_destroy__cq_delete_ERRNO),
	cmocka_unit_test(
		delete_via_destroy__cq_delete_ERRNO_subsequent_ERRNO2),
	cmocka_unit_test(delete_via_destroy__destroy_id_ERRNO),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	/* prepare prestate - default conn_cfg */
	prestate_init(&prestate_conn_cfg_default, MOCK_CONN_CFG_DEFAULT,
			RPMA_DEFAULT_TIMEOUT_MS, MOCK_CQ_SIZE_DEFAULT);

	return cmocka_run_group_tests(test_delete, NULL, NULL);
}
