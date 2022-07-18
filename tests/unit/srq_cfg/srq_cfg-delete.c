// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Fujitsu */

/*
 * srq_cfg-delete.c -- the rpma_srq_cfg_delete() unit tests
 *
 * APIs covered:
 * - rpma_srq_cfg_delete()
 */

#include "srq_cfg-common.h"

/*
 * delete__cfg_ptr_NULL -- NULL cfg_ptr is invalid
 */
static void
delete__cfg_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_srq_cfg_delete(NULL);

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
	struct rpma_srq_cfg *cfg = NULL;
	int ret = rpma_srq_cfg_delete(&cfg);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cfg);
}

static const struct CMUnitTest test_delete[] = {
	/* rpma_srq_cfg_delete() unit tests */
	cmocka_unit_test(delete__cfg_ptr_NULL),
	cmocka_unit_test(delete__cfg_NULL),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_delete, NULL, NULL);
}
