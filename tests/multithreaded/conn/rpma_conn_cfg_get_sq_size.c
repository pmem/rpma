// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_conn_cfg_get_sq_size.c -- rpma_conn_cfg_get_sq_size multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "rpma_conn_cfg_common.h"

/*
 * thread -- get connection configured sq size and check if its value is
 * as expected
 */
static void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct rpma_conn_cfg_common_prestate *pr =
		(struct rpma_conn_cfg_common_prestate *)prestate;
	uint32_t sq_size;
	int ret;

	if ((ret = rpma_conn_cfg_get_sq_size(pr->cfg_ptr, &sq_size))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_sq_size", ret);
		return;
	}

	if (sq_size != RPMA_CONN_CFG_COMMON_Q_SIZE_EXP)
		MTT_ERR(tr, "sq_size != RPMA_CONN_CFG_COMMON_Q_SIZE_EXP",
				EINVAL);
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct rpma_conn_cfg_common_prestate prestate = {NULL};

	struct mtt_test test = {
			&prestate,
			rpma_conn_cfg_common_prestate_init,
			NULL,
			NULL,
			thread,
			NULL,
			NULL,
			rpma_conn_cfg_common_prestate_fini
	};

	return mtt_run(&test, args.threads_num);
}
