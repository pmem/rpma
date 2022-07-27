// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_conn_cfg_set_srq.c -- rpma_conn_cfg_set_srq multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "rpma_conn_cfg_common.h"

#define SET_RPMA_SRQ	(struct rpma_srq *)0xCD15

/*
 * thread -- set the shared RQ object and check if its value is as expected
 */
void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct rpma_conn_cfg_common_prestate *pr = (struct rpma_conn_cfg_common_prestate *)prestate;
	int ret;
	struct rpma_srq *srq;

	if ((ret = rpma_conn_cfg_set_srq(pr->cfg_ptr, SET_RPMA_SRQ))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_set_compl_channel", ret);
		return;
	}

	if ((ret = rpma_conn_cfg_get_srq(pr->cfg_ptr, &srq))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_get_compl_channel", ret);
		return;
	}

	if (srq != SET_RPMA_SRQ)
		MTT_ERR_MSG(tr, "Invalid shared RQ value", -1);
}
