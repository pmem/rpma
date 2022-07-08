// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021-2022, Intel Corporation */

/*
 * rpma_conn_cfg_get_cq_size.c -- rpma_conn_cfg_get_cq_size multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "rpma_conn_cfg_common.h"

/*
 * thread -- get connection configured cq size and check if its value is
 * as expected
 */
void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct rpma_conn_cfg_common_prestate *pr =
	(struct rpma_conn_cfg_common_prestate *)prestate;
	uint32_t cq_size;
	int ret;

	if ((ret = rpma_conn_cfg_get_cq_size(pr->cfg_ptr, &cq_size))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_cq_size", ret);
		return;
	}

	if (cq_size != RPMA_CONN_CFG_COMMON_Q_SIZE_EXP)
		MTT_ERR(tr, "cq_size != RPMA_CONN_CFG_COMMON_Q_SIZE_EXP",
				EINVAL);
}
