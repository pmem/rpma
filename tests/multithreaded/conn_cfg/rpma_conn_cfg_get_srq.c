// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_conn_cfg_get_srq.c -- rpma_conn_cfg_get_srq multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "rpma_conn_cfg_common.h"

/*
 * thread -- get the shared RQ object and check if its value is as expected
 */
void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct rpma_conn_cfg_common_prestate *pr = (struct rpma_conn_cfg_common_prestate *)prestate;
	int ret;
	struct rpma_srq *srq;

	if ((ret = rpma_conn_cfg_get_srq(pr->cfg_ptr, &srq))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_srq", ret);
		return;
	}

	if (srq != RPMA_SRQ_EXP)
		MTT_ERR_MSG(tr, "Invalid shared RQ value", -1);
}
