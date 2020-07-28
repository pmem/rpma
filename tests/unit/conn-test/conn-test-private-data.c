/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn-test-private-data.c -- the connection's private data unit tests
 *
 * APIs covered:
 * - rpma_conn_set_private_data()
 * - rpma_conn_get_private_data()
 */

#include "conn-test-common.h"
#include "test-common.h"

/*
 * get_private_data_test_conn_NULL - NULL conn is invalid
 */
static void
get_private_data_test_conn_NULL(void **unused)
{
	/* get private data */
	struct rpma_conn_private_data data = {0};
	int ret = rpma_conn_get_private_data(NULL, &data);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_ptr_equal(data.ptr, NULL);
	assert_int_equal(data.len, 0);
}

/*
 * get_private_data_test_pdata_NULL - NULL pdata is invalid
 */
static void
get_private_data_test_pdata_NULL(void **unused)
{
	/* get private data */
	struct rpma_conn *conn = MOCK_CONN;
	int ret = rpma_conn_get_private_data(conn, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_private_data_test_conn_NULL_pdata_NULL -
 * (conn == NULL && pdata == NULL) is invalid
 */
static void
get_private_data_test_conn_NULL_pdata_NULL(void **unused)
{
	/* get private data */
	int ret = rpma_conn_get_private_data(NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * set_private_data_test_failed_ENOMEM - rpma_conn_set_private_data() failed
 *                                       with RPMA_E_NOMEM
 */
static void
set_private_data_test_failed_ENOMEM(void **cstate_ptr)
{
	/*
	 * Common things are done by conn_setup()
	 * and conn_teardown().
	 */
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks for rpma_conn_set_private_data() */
	struct rpma_conn_private_data data;
	data.ptr = MOCK_PRIVATE_DATA;
	data.len = MOCK_PDATA_LEN;
	will_return(rpma_private_data_copy, RPMA_E_NOMEM);

	/* set private data */
	int ret = rpma_conn_set_private_data(cstate->conn, &data);

	/* verify the results of rpma_conn_set_private_data() */
	assert_int_equal(ret, RPMA_E_NOMEM);

	/* get private data */
	struct rpma_conn_private_data check_data;
	ret = rpma_conn_get_private_data(cstate->conn, &check_data);

	/* verify the results of rpma_conn_get_private_data() */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(check_data.ptr, NULL);
	assert_int_equal(check_data.len, 0);
}

/*
 * set_private_data_test_success - rpma_conn_set_private_data() succeeds
 */
static void
set_private_data_test_success(void **cstate_ptr)
{
	/*
	 * Common things are done by conn_setup()
	 * and conn_teardown().
	 */
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks for rpma_conn_set_private_data() */
	cstate->data.ptr = MOCK_PRIVATE_DATA;
	cstate->data.len = MOCK_PDATA_LEN;
	will_return(rpma_private_data_copy, 0);
	will_return(rpma_private_data_copy, MOCK_PRIVATE_DATA);

	/* set private data */
	int ret = rpma_conn_set_private_data(cstate->conn, &cstate->data);

	/* verify the results of rpma_conn_set_private_data() */
	assert_int_equal(ret, MOCK_OK);

	/* get private data */
	struct rpma_conn_private_data check_data;
	ret = rpma_conn_get_private_data(cstate->conn, &check_data);

	/* verify the results of rpma_conn_get_private_data() */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(check_data.ptr, cstate->data.ptr);
	assert_int_equal(check_data.len, cstate->data.len);
}

const struct CMUnitTest tests_private_data[] = {
	/* rpma_conn_set_private_data() unit tests */
	cmocka_unit_test_setup_teardown(
		set_private_data_test_failed_ENOMEM,
		conn_setup, conn_teardown),
	cmocka_unit_test_setup_teardown(
		set_private_data_test_success,
		conn_setup, conn_teardown),

	/* rpma_conn_get_private_data() unit tests */
	cmocka_unit_test(get_private_data_test_conn_NULL),
	cmocka_unit_test(get_private_data_test_pdata_NULL),
	cmocka_unit_test(get_private_data_test_conn_NULL_pdata_NULL),
	cmocka_unit_test(NULL)
};
