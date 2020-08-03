/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn-test-new.c -- the connection new/delete unit tests
 *
 * APIs covered:
 * - rpma_conn_new()
 * - rpma_conn_delete()
 */

#include "conn-test-common.h"
#include "test-common.h"

/*
 * new_test_peer_NULL - NULL peer is invalid
 */
static void
new_test_peer_NULL(void **unused)
{
	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(NULL, MOCK_CM_ID, MOCK_IBV_CQ, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(conn);
}

/*
 * new_test_id_NULL - NULL id is invalid
 */
static void
new_test_id_NULL(void **unused)
{
	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_PEER, NULL, MOCK_IBV_CQ, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(conn);
}

/*
 * new_test_cq_NULL - NULL cq is invalid
 */
static void
new_test_cq_NULL(void **unused)
{
	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_PEER, MOCK_CM_ID, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(conn);
}

/*
 * new_test_conn_ptr_NULL - NULL conn_ptr is invalid
 */
static void
new_test_conn_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_new(MOCK_PEER, MOCK_CM_ID, MOCK_IBV_CQ, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new_test_peer_id_cq_conn_ptr_NULL - NULL peer, id, cq and conn_ptr are
 * invalid
 */
static void
new_test_peer_id_cq_conn_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_new(NULL, NULL, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new_test_create_evch_EAGAIN - rdma_create_event_channel() fails with EAGAIN
 */
static void
new_test_create_evch_EAGAIN(void **unused)
{
	/* configure mock */
	will_return(rdma_create_event_channel, NULL);
	will_return(rdma_create_event_channel, EAGAIN);
	will_return_maybe(rpma_flush_new, MOCK_OK);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_PEER, MOCK_CM_ID, MOCK_IBV_CQ, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(conn);
}

/*
 * new_test_migrate_id_EAGAIN - rdma_migrate_id() fails with EAGAIN
 */
static void
new_test_migrate_id_EAGAIN(void **unused)
{
	/* configure mock */
	will_return(rdma_create_event_channel, MOCK_EVCH);
	Rdma_migrate_id_counter = RDMA_MIGRATE_COUNTER_INIT;
	will_return(rdma_migrate_id, EAGAIN);
	will_return_maybe(rpma_flush_new, MOCK_OK);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_PEER, MOCK_CM_ID, MOCK_IBV_CQ, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(conn);
}

/*
 * new_test_flush_ENOMEM - rpma_flush_new() fails with ENOMEM
 */
static void
new_test_flush_ENOMEM(void **unused)
{
	/* configure mock */
	will_return(rpma_flush_new, RPMA_E_NOMEM);
	will_return_maybe(rdma_create_event_channel, MOCK_EVCH);
	Rdma_migrate_id_counter = RDMA_MIGRATE_COUNTER_INIT;
	will_return_maybe(rdma_migrate_id, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_PEER, MOCK_CM_ID, MOCK_IBV_CQ, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(conn);
}

/*
 * new_test_malloc_ENOMEM - malloc() fails with ENOMEM
 */
static void
new_test_malloc_ENOMEM(void **unused)
{
	/* configure mock */
	will_return(__wrap__test_malloc, ENOMEM);
	will_return_maybe(rdma_create_event_channel, MOCK_EVCH);
	Rdma_migrate_id_counter = RDMA_MIGRATE_COUNTER_INIT;
	will_return_maybe(rdma_migrate_id, MOCK_OK);
	will_return(rpma_flush_new, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_PEER, MOCK_CM_ID, MOCK_IBV_CQ, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(conn);
}

/*
 * conn_test_lifecycle - happy day scenario
 */
static void
conn_test_lifecycle(void **cstate_ptr)
{
	/* main things are done by conn_setup() and conn_teardown() */

	struct conn_test_state *cstate = *cstate_ptr;

	/* get private data */
	struct rpma_conn_private_data data;
	int ret = rpma_conn_get_private_data(cstate->conn, &data);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(data.ptr, cstate->data.ptr);
	assert_int_equal(data.len, cstate->data.len);
}

/*
 * delete_test_conn_ptr_NULL - conn_ptr NULL is invalid
 */
static void
delete_test_conn_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_delete(NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * delete_test_conn_NULL - *conn_ptr NULL should cause quick exit
 */
static void
delete_test_conn_NULL(void **unused)
{
	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_delete(&conn);

	/* verify the results */
	assert_int_equal(ret, 0);
}

/*
 * delete_test_destroy_cq_EAGAIN - ibv_destroy_cq() fails with EAGAIN
 */
static void
delete_test_destroy_cq_EAGAIN(void **unused)
{
	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	struct conn_test_state *cstate;
	int ret = conn_setup((void **)&cstate);
	assert_int_equal(ret, 0);
	assert_non_null(cstate->conn);

	/* configure mocks: */
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	will_return(ibv_destroy_cq, EAGAIN);
	will_return(ibv_destroy_comp_channel, MOCK_OK);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	ret = rpma_conn_delete(&cstate->conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->conn);
}

/*
 * delete_test_destroy_cq_EAGAIN_subsequent_EIO -- rdma_destroy_id() and
 * ibv_destroy_comp_channel() fail with EIO when exiting after
 * ibv_destroy_cq() fail (EAGAIN)
 */
static void
delete_test_destroy_cq_EAGAIN_subsequent_EIO(void **unused)
{
	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	struct conn_test_state *cstate;
	int ret = conn_setup((void **)&cstate);
	assert_int_equal(ret, 0);
	assert_non_null(cstate->conn);

	/* configure mocks: */
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	will_return(ibv_destroy_cq, EAGAIN); /* first error */
	will_return(ibv_destroy_comp_channel, EIO);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, EIO); /* second error */

	/* run test */
	ret = rpma_conn_delete(&cstate->conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	/*
	 * We cannot assume which fails first either ibv_destroy_cq() or
	 * rdma_destroy_id().
	 */
	int provider_error = rpma_err_get_provider_error();
	assert_true(provider_error == EAGAIN || provider_error == EIO);
	assert_null(cstate->conn);
}

/*
 * delete_test_destroy_comp_channel_EAGAIN -- ibv_destroy_comp_channel() fails
 * with EAGAIN
 */
static void
delete_test_destroy_comp_channel_EAGAIN(void **unused)
{
	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	struct conn_test_state *cstate;
	int ret = conn_setup((void **)&cstate);
	assert_int_equal(ret, 0);
	assert_non_null(cstate->conn);

	/* configure mocks: */
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, EAGAIN);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	ret = rpma_conn_delete(&cstate->conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->conn);
}

/*
 * delete_test_destroy_comp_channel_EAGAIN_subseqeunt_EIO --
 * ibv_destroy_comp_channel() fails with EAGAIN whereas subsequent fail wit EIO
 */
static void
delete_test_destroy_comp_channel_EAGAIN_subseqeunt_EIO(void **unused)
{
	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	struct conn_test_state *cstate;
	int ret = conn_setup((void **)&cstate);
	assert_int_equal(ret, 0);
	assert_non_null(cstate->conn);

	/* configure mocks: */
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, EAGAIN);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, EIO);

	/* run test */
	ret = rpma_conn_delete(&cstate->conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->conn);
}

/*
 * delete_test_destroy_id_EAGAIN -- ibv_destroy_cq() fails with EAGAIN
 */
static void
delete_test_destroy_id_EAGAIN(void **unused)
{
	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	struct conn_test_state *cstate;
	int ret = conn_setup((void **)&cstate);
	assert_int_equal(ret, 0);
	assert_non_null(cstate->conn);

	/* configure mocks: */
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, EAGAIN);

	/* run test */
	ret = rpma_conn_delete(&cstate->conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->conn);
}

const struct CMUnitTest tests_new[] = {
	/* rpma_conn_new() unit tests */
	cmocka_unit_test(new_test_peer_NULL),
	cmocka_unit_test(new_test_id_NULL),
	cmocka_unit_test(new_test_cq_NULL),
	cmocka_unit_test(new_test_conn_ptr_NULL),
	cmocka_unit_test(new_test_peer_id_cq_conn_ptr_NULL),
	cmocka_unit_test(new_test_create_evch_EAGAIN),
	cmocka_unit_test(new_test_migrate_id_EAGAIN),
	cmocka_unit_test(new_test_flush_ENOMEM),
	cmocka_unit_test(new_test_malloc_ENOMEM),

	/* rpma_conn_new()/_delete() lifecycle */
	cmocka_unit_test_setup_teardown(conn_test_lifecycle,
		conn_setup, conn_teardown),

	/* rpma_conn_delete() unit tests */
	cmocka_unit_test(delete_test_conn_ptr_NULL),
	cmocka_unit_test(delete_test_conn_NULL),
	cmocka_unit_test(delete_test_destroy_cq_EAGAIN),
	cmocka_unit_test(delete_test_destroy_cq_EAGAIN_subsequent_EIO),
	cmocka_unit_test(delete_test_destroy_comp_channel_EAGAIN),
	cmocka_unit_test(
		delete_test_destroy_comp_channel_EAGAIN_subseqeunt_EIO),
	cmocka_unit_test(delete_test_destroy_id_EAGAIN),
	cmocka_unit_test(NULL)
};
