// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * conn_cfg-cqe.c -- the rpma_conn_cfg_get_cqe() unit tests
 *
 * APIs covered:
 * - rpma_conn_cfg_get_cqe()
 */

#include <limits.h>

#include "conn_cfg.h"
#include "conn_cfg-common.h"
#include "test-common.h"

/*
 * cqe__lifecycle -- happy day scenario
 */
static void
cqe__lifecycle(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_set_cq_size(cstate->cfg, MOCK_Q_SIZE);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	int cqe;
	rpma_conn_cfg_get_cqe(cstate->cfg, &cqe);
	assert_int_equal(cqe, MOCK_Q_SIZE);
}

/*
 * cqe__clipped -- cq_size > INT_MAX => cqe = INT_MAX
 */
static void
cqe__clipped(void **cstate_ptr)
{
	struct conn_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_cfg_set_cq_size(cstate->cfg, (uint32_t)INT_MAX + 1);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	int cqe;
	rpma_conn_cfg_get_cqe(cstate->cfg, &cqe);
	assert_int_equal(cqe, INT_MAX);
}

static const struct CMUnitTest test_cqe[] = {
	/* rpma_conn_cfg_set/get_cq_size() lifecycle */
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
