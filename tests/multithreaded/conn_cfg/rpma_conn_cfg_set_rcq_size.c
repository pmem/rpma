// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_conn_cfg_set_rcq_size.c -- rpma_conn_cfg_set_rcq_size multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "rpma_conn_cfg_common.h"

struct rpma_conn_cfg_common_prestate prestate = {NULL};

/*
 * thread -- set connection establishment rcq size and check if its value is as expected
 */
static void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct rpma_conn_cfg_common_prestate *pr =
		(struct rpma_conn_cfg_common_prestate *)prestate;
	uint32_t rcq_size = 0;
	int ret;

	if ((ret = rpma_conn_cfg_get_rcq_size(pr->cfg_ptr, &rcq_size))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_rcq_size", ret);
		return;
	}

	if (rcq_size != RPMA_CONN_CFG_COMMON_Q_SIZE_EXP)
		MTT_ERR_MSG(tr, "Invaild rcq_size: %d instead of %d", -1, rcq_size,
			RPMA_CONN_CFG_COMMON_Q_SIZE_EXP);
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct mtt_test test = {
			&prestate,
			rpma_conn_cfg_common_prestate_init,
			NULL,
			rpma_conn_cfg_common_init,
			thread,
			rpma_conn_cfg_common_fini,
			NULL,
			rpma_conn_cfg_common_prestate_fini
	};

	return mtt_run(&test, args.threads_num);
}
