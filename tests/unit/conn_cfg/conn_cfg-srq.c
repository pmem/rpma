// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Fujitsu */

/*
 * conn_cfg-srq.c -- the rpma_conn_cfg_set/get_srq() unit tests
 *
 * APIs covered:
 * - rpma_conn_cfg_set_srq()
 * - rpma_conn_cfg_get_srq()
 */

#include "conn_cfg-common.h"
#include "test-common.h"
#include "mocks-rpma-srq.h"

/*
 * set__cfg_NULL -- NULL cfg is invalid
 */
static void
set__cfg_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_cfg_set_srq(NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get__cfg_NULL -- NULL cfg is invalid
 */
static void
get__cfg_NULL(void **unused)
{
	/* run test */
	struct rpma_srq *srq = NULL;
	int ret = rpma_conn_cfg_get_srq(NULL, &srq);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get__srq_ptr_NULL -- NULL srq_ptr is invalid
 */
static void
get__srq_ptr_NULL(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_get_srq(cstate->cfg, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * srq__lifecycle -- happy day scenario
 */
static void
srq__lifecycle(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_set_srq(cstate->cfg, MOCK_RPMA_SRQ);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	struct rpma_srq *srq = NULL;
	ret = rpma_conn_cfg_get_srq(cstate->cfg, &srq);
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(srq, MOCK_RPMA_SRQ);
}


static const struct CMUnitTest test_srq[] = {
	/* rpma_conn_cfg_set_srq() unit tests */
	cmocka_unit_test(set__cfg_NULL),

	/* rpma_conn_cfg_get_srq() unit tests */
	cmocka_unit_test(get__cfg_NULL),
	cmocka_unit_test_setup_teardown(get__srq_ptr_NULL,
		setup__conn_cfg, teardown__conn_cfg),

	/* rpma_conn_cfg_set/get_srq() lifecycle */
	cmocka_unit_test_setup_teardown(srq__lifecycle,
		setup__conn_cfg, teardown__conn_cfg),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_srq, NULL, NULL);
}
