// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021-2022, Intel Corporation */

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
void
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
