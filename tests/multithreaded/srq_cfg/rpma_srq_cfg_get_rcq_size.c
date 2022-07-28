// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_srq_cfg_get_rcq_size.c -- rpma_srq_cfg_get_rcq_size multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "rpma_srq_cfg_common.h"

/*
 * thread -- get the srq configured rcq size and check if its value is as expected
 */
void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct rpma_srq_cfg_common_prestate *pr = (struct rpma_srq_cfg_common_prestate *)prestate;
	uint32_t rcq_size;
	int ret;

	if ((ret = rpma_srq_cfg_get_rcq_size(pr->cfg_ptr, &rcq_size))) {
		MTT_RPMA_ERR(tr, "rpma_srq_cfg_get_rcq_size", ret);
		return;
	}

	if (rcq_size != RPMA_SRQ_CFG_COMMON_Q_SIZE_EXP)
		MTT_ERR(tr, "rcq_size != RPMA_SRQ_CFG_COMMON_Q_SIZE_EXP", EINVAL);
}
