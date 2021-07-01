// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_conn_cfg_get_timeout.c -- rpma_conn_cfg_get_timeout multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "rpma_conn_cfg_common.h"

/*
 * thread -- get connection configured timeout and check if its value is
 * as expected
 */
static void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct rpma_conn_cfg_common_prestate *pr =
		(struct rpma_conn_cfg_common_prestate *)prestate;
	int ret, timeout_ms;

	if ((ret = rpma_conn_cfg_get_timeout(pr->cfg_ptr, &timeout_ms))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_timeout", ret);
		return;
	}

	if (timeout_ms != RPMA_CONN_CFG_COMMON_TIMEOUT_MS_EXP)
		MTT_ERR(tr, "timeout_ms != RPMA_CONN_CFG_COMMON_TIMEOUT_MS_EXP",
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
