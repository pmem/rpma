// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn_cfg-cqe.c -- the rpma_conn_cfg_get_cqe() unit tests
 *
 * API covered:
 * - rpma_conn_cfg_get_cqe()
 */

#include <limits.h>

#include "conn_cfg.h"
#include "conn_cfg-common.h"
#include "test-common.h"

/*
 * get__cfg_NULL -- NULL cfg is invalid
 */
static void
get__cfg_NULL(void **unused)
{
	/* run test */
	int cqe, recv_cqe;
	int ret = rpma_conn_cfg_get_cqe(NULL, &cqe, &recv_cqe);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get__cqe_NULL -- NULL cqe is invalid
 */
static void
get__cqe_NULL(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;
	int recv_cqe;

	/* run test */
	int ret = rpma_conn_cfg_get_cqe(cstate->cfg, NULL, &recv_cqe);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get__recv_cqe_NULL -- NULL recv_cqe is invalid
 */
static void
get__recv_cqe_NULL(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;
	int cqe;

	/* run test */
	int ret = rpma_conn_cfg_get_cqe(cstate->cfg, &cqe, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * cqe__lifecycle -- happy day scenario
 */
static void
cqe__lifecycle(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;
	int ret;

	ret = rpma_conn_cfg_set_cq_size(cstate->cfg, MOCK_Q_SIZE);
	assert_int_equal(ret, MOCK_OK);

	ret = rpma_conn_cfg_set_rcq_size(cstate->cfg, MOCK_Q_SIZE);
	assert_int_equal(ret, MOCK_OK);

	/* run test */
	int cqe, recv_cqe;
	ret = rpma_conn_cfg_get_cqe(cstate->cfg, &cqe, &recv_cqe);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(cqe, MOCK_Q_SIZE);
	assert_int_equal(recv_cqe, MOCK_Q_SIZE);
}

/*
 * cqe__clipped -- cq_size/rcq_size > INT_MAX => cqe/recv_cqe = INT_MAX
 */
static void
cqe__clipped(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;
	int ret;

	ret = rpma_conn_cfg_set_cq_size(cstate->cfg, (uint32_t)INT_MAX + 1);
	assert_int_equal(ret, MOCK_OK);

	ret = rpma_conn_cfg_set_rcq_size(cstate->cfg, (uint32_t)INT_MAX + 1);
	assert_int_equal(ret, MOCK_OK);

	/* run test */
	int cqe, recv_cqe;
	ret = rpma_conn_cfg_get_cqe(cstate->cfg, &cqe, &recv_cqe);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(cqe, INT_MAX);
	assert_int_equal(recv_cqe, INT_MAX);
}

static const struct CMUnitTest test_cqe[] = {
	cmocka_unit_test(get__cfg_NULL),
	cmocka_unit_test_setup_teardown(get__cqe_NULL,
		setup__conn_cfg, teardown__conn_cfg),
	cmocka_unit_test_setup_teardown(get__recv_cqe_NULL,
		setup__conn_cfg, teardown__conn_cfg),

	/*
	 * rpma_conn_cfg_set/get_cq_size() and
	 * rpma_conn_cfg_set/get_rcq_size() lifecycle
	 */
	cmocka_unit_test_setup_teardown(cqe__lifecycle,
		setup__conn_cfg, teardown__conn_cfg),
	cmocka_unit_test_setup_teardown(cqe__clipped,
			setup__conn_cfg, teardown__conn_cfg),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_cqe, NULL, NULL);
}
