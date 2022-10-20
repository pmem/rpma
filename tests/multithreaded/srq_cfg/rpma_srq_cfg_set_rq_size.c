// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_srq_cfg_set_rq_size.c -- rpma_srq_cfg_set_rq_size multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "rpma_srq_cfg_common.h"

/*
 * thread -- set the srq establishment rq size and check if its value is as expected
 */
void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct rpma_srq_cfg_common_prestate *pr = (struct rpma_srq_cfg_common_prestate *)prestate;
	uint32_t rq_size = 0;
	int ret;

	ret = rpma_srq_cfg_set_rq_size(pr->cfg_ptr, RPMA_SRQ_CFG_COMMON_Q_SIZE_EXP);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_srq_cfg_set_rq_size", ret);
		return;
	}

	ret = rpma_srq_cfg_get_rq_size(pr->cfg_ptr, &rq_size);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_srq_cfg_get_rq_size", ret);
		return;
	}

	if (rq_size != RPMA_SRQ_CFG_COMMON_Q_SIZE_EXP)
		MTT_ERR_MSG(tr, "Invalid rq_size: %d instead of %d", -1, rq_size,
			RPMA_SRQ_CFG_COMMON_Q_SIZE_EXP);
}
