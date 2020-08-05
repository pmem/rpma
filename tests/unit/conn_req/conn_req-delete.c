// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn_req-delete.c -- the rpma_conn_req_delete() unit tests
 *
 * APIs covered:
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
 * delete_via_reject__destroy_cq_EAGAIN - ibv_destroy_cq() fails with EAGAIN
 */
static void
delete_via_reject__destroy_cq_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
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
 * delete_via_reject__destroy_cq_EAGAIN_subsequent_EIO -
 * rdma_ack_cm_event() fails with EIO after rdma_reject() failed
 * with EIO after ibv_destroy_cq() failed with EAGAIN
 */
static void
delete_via_reject__destroy_cq_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
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
 * delete_via_reject__destroy_comp_channel_EAGAIN -- ibv_destroy_comp_channel()
 * fails with EAGAIN
 */
static void
delete_via_reject__destroy_comp_channel_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
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
 * delete_via_reject__destroy_comp_channel_EAGAIN_subsequent_EIO --
 * ibv_destroy_comp_channel() fails with EAGAIN whereas subsequent fail with EIO
 */
static void
delete_via_reject__destroy_comp_channel_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
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
 * delete_via_reject__reject_EAGAIN -- rdma_reject() fails with EAGAIN
 */
static void
delete_via_reject__reject_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
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
 * delete_via_reject__reject_EAGAIN_ack_EIO - rdma_ack_cm_event()
 * fails with EIO after rdma_reject() failed with EAGAIN
 */
static void
delete_via_reject__reject_EAGAIN_ack_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
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
 * delete_via_reject__ack_EAGAIN - rdma_ack_cm_event() fails with EAGAIN
 */
static void
delete_via_reject__ack_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_from_cm_event((void **)&cstate), 0);
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
 * delete_via_destroy__destroy_cq_EAGAIN - ibv_destroy_cq()
 * fails with EAGAIN
 */
static void
delete_via_destroy__destroy_cq_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
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
 * delete_via_destroy__destroy_cq_EAGAIN_subsequent_EIO --
 * ibv_destroy_comp_channel() and rdma_destroy_id() fail with EIO after
 * ibv_destroy_cq() failed with EAGAIN
 */
static void
delete_via_destroy__destroy_cq_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
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
 * delete_via_destroy__destroy_comp_channel_EAGAIN --
 * ibv_destroy_comp_channel() fails with EAGAIN
 */
static void
delete_via_destroy__destroy_comp_channel_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
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
 * delete_via_destroy__destroy_comp_channel_EAGAIN_subsequent_EIO --
 * ibv_destroy_comp_channel() fails with EAGAIN whereas subsequent fail with EIO
 */
static void
delete_via_destroy__destroy_comp_channel_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
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
 * delete_via_destroy__destroy_id_EAGAIN - rdma_destroy_id()
 * fails with EAGAIN
 */
static void
delete_via_destroy__destroy_id_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(setup__conn_req_new((void **)&cstate), 0);
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

const struct CMUnitTest test_delete[] = {
	/* rpma_conn_req_delete() unit tests */
	cmocka_unit_test(delete__req_ptr_NULL),
	cmocka_unit_test(delete__req_NULL),
	/* delete via rdma_reject() */
	cmocka_unit_test(delete_via_reject__destroy_cq_EAGAIN),
	cmocka_unit_test(
		delete_via_reject__destroy_cq_EAGAIN_subsequent_EIO),
	cmocka_unit_test(
		delete_via_reject__destroy_comp_channel_EAGAIN),
	cmocka_unit_test(
	delete_via_reject__destroy_comp_channel_EAGAIN_subsequent_EIO),
	cmocka_unit_test(delete_via_reject__reject_EAGAIN),
	cmocka_unit_test(delete_via_reject__reject_EAGAIN_ack_EIO),
	cmocka_unit_test(delete_via_reject__ack_EAGAIN),
	/* delete via rdma_destroy_id() */
	cmocka_unit_test(delete_via_destroy__destroy_cq_EAGAIN),
	cmocka_unit_test(
		delete_via_destroy__destroy_cq_EAGAIN_subsequent_EIO),
	cmocka_unit_test(
		delete_via_destroy__destroy_comp_channel_EAGAIN),
	cmocka_unit_test(
	delete_via_destroy__destroy_comp_channel_EAGAIN_subsequent_EIO),
	cmocka_unit_test(delete_via_destroy__destroy_id_EAGAIN),
	cmocka_unit_test(NULL)
};
