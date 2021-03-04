// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * conn-private-data.c -- the connection's private data unit tests
 *
 * APIs covered:
 * - rpma_conn_transfer_private_data()
 * - rpma_conn_get_private_data()
 */

#include "conn-common.h"

/*
 * get_private_data__conn_NULL - NULL conn is invalid
 */
static void
get_private_data__conn_NULL(void **unused)
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
 * get_private_data__pdata_NULL - NULL pdata is invalid
 */
static void
get_private_data__pdata_NULL(void **unused)
{
	/* get private data */
	struct rpma_conn *conn = MOCK_CONN;
	int ret = rpma_conn_get_private_data(conn, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_private_data__conn_NULL_pdata_NULL -
 * (conn == NULL && pdata == NULL) is invalid
 */
static void
get_private_data__conn_NULL_pdata_NULL(void **unused)
{
	/* get private data */
	int ret = rpma_conn_get_private_data(NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * transfer_private_data__success - rpma_conn_transfer_private_data() succeeds
 */
static void
transfer_private_data__success(void **cstate_ptr)
{
	/*
	 * Common things are done by setup__conn_new()
	 * and teardown__conn_delete().
	 */
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks for rpma_conn_transfer_private_data() */
	cstate->data.ptr = MOCK_PRIVATE_DATA;
	cstate->data.len = MOCK_PDATA_LEN;

	/* transfer private data */
	rpma_conn_transfer_private_data(cstate->conn, &cstate->data);

	/* verify the source of the private data is zeroed */
	assert_ptr_equal(cstate->data.ptr, NULL);
	assert_int_equal(cstate->data.len, 0);

	/* get private data */
	struct rpma_conn_private_data check_data;
	int ret = rpma_conn_get_private_data(cstate->conn, &check_data);

	/* verify the results of rpma_conn_get_private_data() */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(check_data.ptr, MOCK_PRIVATE_DATA);
	assert_int_equal(check_data.len, MOCK_PDATA_LEN);

	/* set expected private data which will be used during teardown */
	cstate->data.ptr = MOCK_PRIVATE_DATA;
	cstate->data.len = MOCK_PDATA_LEN;
}

static const struct CMUnitTest tests_private_data[] = {
	/* rpma_conn_transfer_private_data() unit tests */
	cmocka_unit_test_setup_teardown(
		transfer_private_data__success,
		setup__conn_new, teardown__conn_delete),

	/* rpma_conn_get_private_data() unit tests */
	cmocka_unit_test(get_private_data__conn_NULL),
	cmocka_unit_test(get_private_data__pdata_NULL),
	cmocka_unit_test(get_private_data__conn_NULL_pdata_NULL),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_private_data, NULL, NULL);
}
