// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_conn_cfg_get_compl_channel.c -- rpma_conn_cfg_get_compl_channel multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "rpma_conn_cfg_common.h"

/*
 * thread -- get connection configured completion channel and check if its value is as expected
 */
void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct rpma_conn_cfg_common_prestate *pr =
		(struct rpma_conn_cfg_common_prestate *)prestate;
	int ret;
	bool shared;

	if ((ret = rpma_conn_cfg_get_compl_channel(pr->cfg_ptr, &shared))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_compl_channel", ret);
		return;
	}

	if (shared != RPMA_CONN_CFG_COMMON_IS_SHARED)
		MTT_ERR_MSG(tr, "Invalid completion channel's value: %d instead of %d", -1,
			shared, RPMA_CONN_CFG_COMMON_IS_SHARED);
}
