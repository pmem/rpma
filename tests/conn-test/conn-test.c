/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn-test.c -- the connection unit tests
 */

#include "cmocka_headers.h"
#include "conn.h"

#define MOCK_EVCH	(struct rdma_event_channel *)0xE4C4
#define MOCK_CQ		(struct ibv_cq *)0x00C0
#define MOCK_CM_ID	(struct rdma_cm_id *)0xC41D

#define NO_ERROR	0

/*
 * rdma_destroy_qp -- rdma_destroy_qp() mock
 */
void
rdma_destroy_qp(struct rdma_cm_id *id)
{
	check_expected_ptr(id);
}

/*
 * ibv_destroy_cq -- ibv_destroy_cq() mock
 */
int
ibv_destroy_cq(struct ibv_cq *cq)
{
	check_expected_ptr(cq);

	return mock_type(int);
}

/*
 * rdma_destroy_id -- rdma_destroy_id() mock
 */
int
rdma_destroy_id(struct rdma_cm_id *id)
{
	check_expected_ptr(id);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_destroy_event_channel -- rdma_destroy_event_channel() mock
 */
void
rdma_destroy_event_channel(struct rdma_event_channel *channel)
{
	check_expected_ptr(channel);
}

void *__real__test_malloc(size_t size);

/*
 * __wrap__test_malloc -- malloc() mock
 */
void *
__wrap__test_malloc(size_t size)
{
	errno = mock_type(int);

	if (errno)
		return NULL;

	return __real__test_malloc(size);
}

/*
 * new_test_id_NULL - NULL id is invalid
 */
static void
new_test_id_NULL(void **unused)
{
	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(NULL, MOCK_EVCH, MOCK_CQ, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(conn);
}

/*
 * new_test_evch_NULL - NULL evch is invalid
 */
static void
new_test_evch_NULL(void **unused)
{
	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_CM_ID, NULL, MOCK_CQ, &conn);

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
	int ret = rpma_conn_new(MOCK_CM_ID, MOCK_EVCH, NULL, &conn);

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
	int ret = rpma_conn_new(MOCK_CM_ID, MOCK_EVCH, MOCK_CQ, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new_test_id_evch_cq_conn_ptr_NULL - NULL id, evch, cq and conn_ptr are
 * invalid
 */
static void
new_test_id_evch_cq_conn_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_new(NULL, NULL, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new_test_malloc_ENOMEM - malloc() fails with ENOMEM
 */
static void
new_test_malloc_ENOMEM(void **unused)
{
	/* configure mock */
	will_return(__wrap__test_malloc, ENOMEM);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_CM_ID, MOCK_EVCH, MOCK_CQ, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(conn);
}

/*
 * conn_setup - prepare a valid rpma_conn object
 */
static int
conn_setup(void **conn_ptr)
{
	/* configure mock: */
	will_return(__wrap__test_malloc, 0);

	/* prepare an object */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_CM_ID, MOCK_EVCH, MOCK_CQ, &conn);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
	assert_non_null(conn);

	*conn_ptr = conn;

	return 0;
}

/*
 * conn_teardown - delete the rpma_conn object
 */
static int
conn_teardown(void **conn_ptr)
{
	struct rpma_conn *conn = *conn_ptr;

	/* configure mocks: */
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(ibv_destroy_cq, cq, MOCK_CQ);
	will_return(ibv_destroy_cq, NO_ERROR);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, NO_ERROR);
	expect_value(rdma_destroy_event_channel, channel, MOCK_EVCH);

	/* delete the object */
	int ret = rpma_conn_delete(&conn);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
	assert_null(conn);

	*conn_ptr = NULL;

	return 0;
}

/*
 * conn_test_lifecycle - happy day scenario
 */
static void
conn_test_lifecycle(void **unused)
{
	/* the thing is done by conn_setup() and conn_teardown() */
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
delete_test_destroy_cq_EAGAIN(void **conn_ptr)
{
	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	struct rpma_conn *conn = NULL;
	int ret = conn_setup((void **)&conn);
	assert_int_equal(ret, 0);
	assert_non_null(conn);

	/* configure mocks: */
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(ibv_destroy_cq, cq, MOCK_CQ);
	will_return(ibv_destroy_cq, EAGAIN);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, NO_ERROR);
	expect_value(rdma_destroy_event_channel, channel, MOCK_EVCH);

	/* run test */
	ret = rpma_conn_delete(&conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(conn);
}

/*
 * delete_test_destroy_cq_EAGAIN_destroy_id_EAGAIN - rdma_destroy_id() fails
 * with EIO when exiting after ibv_destroy_cq() fail (EAGAIN)
 */
static void
delete_test_destroy_cq_EAGAIN_destroy_id_EAGAIN(void **conn_ptr)
{
	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	struct rpma_conn *conn = NULL;
	int ret = conn_setup((void **)&conn);
	assert_int_equal(ret, 0);
	assert_non_null(conn);

	/* configure mocks: */
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(ibv_destroy_cq, cq, MOCK_CQ);
	will_return(ibv_destroy_cq, EAGAIN); /* first error */
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, EIO); /* second error */
	expect_value(rdma_destroy_event_channel, channel, MOCK_EVCH);

	/* run test */
	ret = rpma_conn_delete(&conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	/*
	 * We cannot assume which fails first either ibv_destroy_cq() or
	 * rdma_destroy_id().
	 */
	int provider_error = rpma_err_get_provider_error();
	assert_true(provider_error == EAGAIN || provider_error == EIO);
	assert_null(conn);
}

/*
 * delete_test_destroy_id_EAGAIN - ibv_destroy_cq() fails with EAGAIN
 */
static void
delete_test_destroy_id_EAGAIN(void **conn_ptr)
{
	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	struct rpma_conn *conn = NULL;
	int ret = conn_setup((void **)&conn);
	assert_int_equal(ret, 0);
	assert_non_null(conn);

	/* configure mocks: */
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(ibv_destroy_cq, cq, MOCK_CQ);
	will_return(ibv_destroy_cq, NO_ERROR);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, EAGAIN);
	expect_value(rdma_destroy_event_channel, channel, MOCK_EVCH);

	/* run test */
	ret = rpma_conn_delete(&conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(conn);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_conn_new() unit tests */
		cmocka_unit_test(new_test_id_NULL),
		cmocka_unit_test(new_test_evch_NULL),
		cmocka_unit_test(new_test_cq_NULL),
		cmocka_unit_test(new_test_conn_ptr_NULL),
		cmocka_unit_test(new_test_id_evch_cq_conn_ptr_NULL),
		cmocka_unit_test(new_test_malloc_ENOMEM),

		/* rpma_conn_new()/_delete() lifecycle */
		cmocka_unit_test_setup_teardown(conn_test_lifecycle,
			conn_setup, conn_teardown),

		/* rpma_conn_delete() unit tests */
		cmocka_unit_test(delete_test_conn_ptr_NULL),
		cmocka_unit_test(delete_test_conn_NULL),
		cmocka_unit_test(delete_test_destroy_cq_EAGAIN),
		cmocka_unit_test(
			delete_test_destroy_cq_EAGAIN_destroy_id_EAGAIN),
		cmocka_unit_test(delete_test_destroy_id_EAGAIN),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
