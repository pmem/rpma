// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Fujitsu */

/*
 * srq_cfg-rcqe.c -- the rpma_srq_cfg_get_rcqe() unit tests
 *
 * API covered:
 * - rpma_srq_cfg_get_rcqe()
 */

#include <limits.h>

#include "srq_cfg-common.h"
#include "srq_cfg.h"

/*
 * rcqe__lifecycle -- happy day scenario
 */
static void
rcqe__lifecycle(void **cstate_ptr)
{
	struct srq_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_srq_cfg_set_rcq_size(cstate->cfg, MOCK_Q_SIZE);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	int rcqe;
	rpma_srq_cfg_get_rcqe(cstate->cfg, &rcqe);
	assert_int_equal(rcqe, MOCK_Q_SIZE);
}

/*
 * rcqe__clipped -- rcq_size > INT_MAX => rcqe = INT_MAX
 */
static void
rcqe__clipped(void **cstate_ptr)
{
	struct srq_cfg_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_srq_cfg_set_rcq_size(cstate->cfg, (uint32_t)INT_MAX + 1);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	int rcqe;
	rpma_srq_cfg_get_rcqe(cstate->cfg, &rcqe);
	assert_int_equal(rcqe, INT_MAX);
}

static const struct CMUnitTest test_rcqe[] = {
	/* rpma_srq_cfg_set/get_cq_size() lifecycle */
	cmocka_unit_test_setup_teardown(rcqe__lifecycle, setup__srq_cfg, teardown__srq_cfg),
	cmocka_unit_test_setup_teardown(rcqe__clipped, setup__srq_cfg, teardown__srq_cfg),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_rcqe, NULL, NULL);
}
