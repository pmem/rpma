// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn-new.c -- the connection new/delete unit tests
 *
 * APIs covered:
 * - rpma_conn_new()
 * - rpma_conn_delete()
 */

#include "conn-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"
#include "test-common.h"

/*
 * new__peer_NULL - NULL peer is invalid
 */
static void
new__peer_NULL(void **unused)
{
	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(NULL, MOCK_CM_ID, MOCK_RPMA_CQ, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(conn);
}

/*
 * new__id_NULL - NULL id is invalid
 */
static void
new__id_NULL(void **unused)
{
	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_PEER, NULL, MOCK_RPMA_CQ, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(conn);
}

/*
 * new__cq_NULL - NULL cq is invalid
 */
static void
new__cq_NULL(void **unused)
{
	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_PEER, MOCK_CM_ID, NULL, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(conn);
}

/*
 * new__conn_ptr_NULL - NULL conn_ptr is invalid
 */
static void
new__conn_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_new(MOCK_PEER, MOCK_CM_ID, MOCK_RPMA_CQ, NULL,
			NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new__peer_id_cq_conn_ptr_NULL - NULL peer, id, cq and conn_ptr are
 * invalid
 */
static void
new__peer_id_cq_conn_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_new(NULL, NULL, NULL, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new__create_evch_ERRNO - rdma_create_event_channel() fails with MOCK_ERRNO
 */
static void
new__create_evch_ERRNO(void **unused)
{
	/* configure mock */
	will_return(rdma_create_event_channel, NULL);
	will_return(rdma_create_event_channel, MOCK_ERRNO);
	will_return_maybe(rpma_flush_new, MOCK_OK);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_PEER, MOCK_CM_ID, MOCK_RPMA_CQ, NULL,
			&conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(conn);
}

/*
 * new__migrate_id_ERRNO - rdma_migrate_id() fails with MOCK_ERRNO
 */
static void
new__migrate_id_ERRNO(void **unused)
{
	/* configure mock */
	will_return(rdma_create_event_channel, MOCK_EVCH);
	Rdma_migrate_id_counter = RDMA_MIGRATE_COUNTER_INIT;
	will_return(rdma_migrate_id, MOCK_ERRNO);
	will_return_maybe(rpma_flush_new, MOCK_OK);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_PEER, MOCK_CM_ID, MOCK_RPMA_CQ, NULL,
			&conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(conn);
}

/*
 * new__flush_E_NOMEM - rpma_flush_new() fails with RPMA_E_NOMEM
 */
static void
new__flush_E_NOMEM(void **unused)
{
	/* configure mock */
	will_return(rpma_flush_new, RPMA_E_NOMEM);
	will_return_maybe(rdma_create_event_channel, MOCK_EVCH);
	Rdma_migrate_id_counter = RDMA_MIGRATE_COUNTER_INIT;
	will_return_maybe(rdma_migrate_id, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_PEER, MOCK_CM_ID, MOCK_RPMA_CQ, NULL,
			&conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(conn);
}

/*
 * new__malloc_ERRNO - malloc() fails with MOCK_ERRNO
 */
static void
new__malloc_ERRNO(void **unused)
{
	/* configure mock */
	will_return(__wrap__test_malloc, MOCK_ERRNO);
	will_return_maybe(rdma_create_event_channel, MOCK_EVCH);
	Rdma_migrate_id_counter = RDMA_MIGRATE_COUNTER_INIT;
	will_return_maybe(rdma_migrate_id, MOCK_OK);
	will_return_maybe(rpma_flush_new, MOCK_OK);
	will_return_maybe(rpma_flush_delete, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_PEER, MOCK_CM_ID, MOCK_RPMA_CQ, NULL,
			&conn);

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
	/*
	 * Main things are done by setup__conn_new()
	 * and teardown__conn_delete().
	 */
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
 * delete__conn_ptr_NULL - conn_ptr NULL is invalid
 */
static void
delete__conn_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_delete(NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * delete__conn_NULL - *conn_ptr NULL should cause quick exit
 */
static void
delete__conn_NULL(void **unused)
{
	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_delete(&conn);

	/* verify the results */
	assert_int_equal(ret, 0);
}

/*
 * delete__flush_delete_ERRNO - rpma_flush_delete() fails with MOCK_ERRNO
 */
static void
delete__flush_delete_ERRNO(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	int ret = setup__conn_new((void **)&cstate);
	assert_int_equal(ret, 0);
	assert_non_null(cstate->conn);

	/* configure mocks: */
	will_return(rpma_flush_delete, RPMA_E_PROVIDER);
	will_return(rpma_flush_delete, MOCK_ERRNO);
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(rpma_cq_delete, *cq_ptr, cstate->rcq);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return_maybe(rdma_destroy_id, MOCK_OK);

	/* run test */
	ret = rpma_conn_delete(&cstate->conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->conn);
}

/*
 * delete__flush_delete_E_INVAL - rpma_flush_delete()
 * fails with RPMA_E_INVAL
 */
static void
delete__flush_delete_E_INVAL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	int ret = setup__conn_new((void **)&cstate);
	assert_int_equal(ret, 0);
	assert_non_null(cstate->conn);

	/* configure mocks */
	will_return(rpma_flush_delete, RPMA_E_INVAL);
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(rpma_cq_delete, *cq_ptr, cstate->rcq);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return_maybe(rdma_destroy_id, MOCK_OK);

	/* run test */
	ret = rpma_conn_delete(&cstate->conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(cstate->conn);
}

/*
 * delete__rcq_delete_ERRNO - rpma_cq_delete(&conn->rcq) fails with MOCK_ERRNO
 */
static void
delete__rcq_delete_ERRNO(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	int ret = setup__conn_new((void **)&cstate);
	assert_int_equal(ret, 0);
	assert_non_null(cstate->conn);

	/* configure mocks: */
	will_return(rpma_flush_delete, MOCK_OK);
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_RCQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	ret = rpma_conn_delete(&cstate->conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->conn);
}

/*
 * delete__rcq_delete_ERRNO_subsequent_ERRNO2 -- rpma_cq_delete(&conn->rcq)
 * fails with MOCK_ERRNO whereas subsequent (rpma_cq_delete(&conn->cq),
 * rdma_destroy_id()) fail with MOCK_ERRNO2
 */
static void
delete__rcq_delete_ERRNO_subsequent_ERRNO2(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	int ret = setup__conn_new((void **)&cstate);
	assert_int_equal(ret, 0);
	assert_non_null(cstate->conn);

	/* configure mocks */
	will_return(rpma_flush_delete, MOCK_OK);
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_RCQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO); /* first error */
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO2); /* second error */
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, MOCK_ERRNO2); /* third error */

	/* run test */
	ret = rpma_conn_delete(&cstate->conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->conn);
}

/*
 * delete__cq_delete_ERRNO - rpma_cq_delete(&conn->cq) fails with MOCK_ERRNO
 */
static void
delete__cq_delete_ERRNO(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	int ret = setup__conn_new((void **)&cstate);
	assert_int_equal(ret, 0);
	assert_non_null(cstate->conn);

	/* configure mocks: */
	will_return(rpma_flush_delete, MOCK_OK);
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(rpma_cq_delete, *cq_ptr, cstate->rcq);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	ret = rpma_conn_delete(&cstate->conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->conn);
}

/*
 * delete__cq_delete_ERRNO_subsequent_ERRNO2 -- rdma_destroy_id() fails
 * with MOCK_ERRNO2 after rpma_cq_delete(&conn->cq) failed with MOCK_ERRNO
 */
static void
delete__cq_delete_ERRNO_subsequent_ERRNO2(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	int ret = setup__conn_new((void **)&cstate);
	assert_int_equal(ret, 0);
	assert_non_null(cstate->conn);

	/* configure mocks: */
	will_return(rpma_flush_delete, MOCK_OK);
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(rpma_cq_delete, *cq_ptr, cstate->rcq);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO); /* first error */
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, MOCK_ERRNO2); /* second error */

	/* run test */
	ret = rpma_conn_delete(&cstate->conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->conn);
}

/*
 * delete__destroy_id_ERRNO -- rdma_destroy_id() fails with MOCK_ERRNO
 */
static void
delete__destroy_id_ERRNO(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	int ret = setup__conn_new((void **)&cstate);
	assert_int_equal(ret, 0);
	assert_non_null(cstate->conn);

	/* configure mocks: */
	will_return(rpma_flush_delete, MOCK_OK);
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(rpma_cq_delete, *cq_ptr, cstate->rcq);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_CQ);
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, MOCK_ERRNO);

	/* run test */
	ret = rpma_conn_delete(&cstate->conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->conn);
}

static const struct CMUnitTest tests_new[] = {
	/* rpma_conn_new() unit tests */
	cmocka_unit_test(new__peer_NULL),
	cmocka_unit_test(new__id_NULL),
	cmocka_unit_test(new__cq_NULL),
	cmocka_unit_test(new__conn_ptr_NULL),
	cmocka_unit_test(new__peer_id_cq_conn_ptr_NULL),
	cmocka_unit_test(new__create_evch_ERRNO),
	cmocka_unit_test(new__migrate_id_ERRNO),
	cmocka_unit_test(new__flush_E_NOMEM),
	cmocka_unit_test(new__malloc_ERRNO),

	/* rpma_conn_new()/_delete() lifecycle */
	CONN_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_RCQ(
		conn_test_lifecycle, setup__conn_new, teardown__conn_delete),

	/* rpma_conn_delete() unit tests */
	cmocka_unit_test(delete__conn_ptr_NULL),
	cmocka_unit_test(delete__conn_NULL),
	CONN_TEST_WITH_AND_WITHOUT_RCQ(delete__flush_delete_ERRNO),
	CONN_TEST_WITH_AND_WITHOUT_RCQ(delete__flush_delete_E_INVAL),
	cmocka_unit_test_prestate(delete__rcq_delete_ERRNO, &Conn_with_rcq),
	cmocka_unit_test_prestate(
		delete__rcq_delete_ERRNO_subsequent_ERRNO2, &Conn_with_rcq),
	CONN_TEST_WITH_AND_WITHOUT_RCQ(delete__cq_delete_ERRNO),
	CONN_TEST_WITH_AND_WITHOUT_RCQ(
		delete__cq_delete_ERRNO_subsequent_ERRNO2),
	CONN_TEST_WITH_AND_WITHOUT_RCQ(delete__destroy_id_ERRNO),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_new, NULL, NULL);
}
