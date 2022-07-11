// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021-2022, Intel Corporation */

/*
 * rpma_conn_cfg_set_rq_size.c -- rpma_conn_cfg_set_rq_size multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "rpma_conn_cfg_common.h"

/*
 * thread -- set connection establishment rq size and check if its value is
 * as expected
 */
void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct rpma_conn_cfg_common_prestate *pr =
		(struct rpma_conn_cfg_common_prestate *)prestate;
	uint32_t rq_size = 0;
	int ret;

	if ((ret = rpma_conn_cfg_set_rq_size(pr->cfg_ptr, RPMA_CONN_CFG_COMMON_Q_SIZE_EXP))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_set_rq_size", ret);
		return;
	}

	if ((ret = rpma_conn_cfg_get_rq_size(pr->cfg_ptr, &rq_size))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_rq_size", ret);
		return;
	}

	if (rq_size != RPMA_CONN_CFG_COMMON_Q_SIZE_EXP)
		MTT_ERR_MSG(tr, "Invaild rq_size: %d instead of %d", -1, rq_size,
			RPMA_CONN_CFG_COMMON_Q_SIZE_EXP);
}
