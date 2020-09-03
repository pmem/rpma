// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * conn_cfg-delete.c -- the rpma_conn_cfg_delete() unit tests
 *
 * APIs covered:
 * - rpma_conn_cfg_delete()
 */

#include "conn_cfg-common.h"

/*
 * delete__cfg_ptr_NULL -- NULL cfg_ptr is invalid
 */
static void
delete__cfg_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_cfg_delete(NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * delete__cfg_NULL -- NULL cfg is valid - quick exit
 */
static void
delete__cfg_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_cfg *cfg = NULL;
	int ret = rpma_conn_cfg_delete(&cfg);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cfg);
}

static const struct CMUnitTest test_delete[] = {
	/* rpma_conn_cfg_delete() unit tests */
	cmocka_unit_test(delete__cfg_ptr_NULL),
	cmocka_unit_test(delete__cfg_NULL),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_delete, NULL, NULL);
}
