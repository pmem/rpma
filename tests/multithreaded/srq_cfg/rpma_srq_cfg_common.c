// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_srq_cfg_common.c -- common part for rpma_srq_cfg_* multithreaded tests
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "rpma_srq_cfg_common.h"

/*
 * rpma_srq_cfg_common_prestate_init -- create a new srq configuration object, set all queue sizes
 */
void
rpma_srq_cfg_common_prestate_init(void *prestate, struct mtt_result *tr)
{
	struct rpma_srq_cfg_common_prestate *pr = (struct rpma_srq_cfg_common_prestate *)prestate;
	int ret;

	ret = rpma_srq_cfg_new(&pr->cfg_ptr);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_srq_cfg_new", ret);
		return;
	}

	ret = rpma_srq_cfg_set_rq_size(pr->cfg_ptr, RPMA_SRQ_CFG_COMMON_Q_SIZE_EXP);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_srq_cfg_set_rq_size", ret);
		return;
	}

	ret = rpma_srq_cfg_set_rcq_size(pr->cfg_ptr, RPMA_SRQ_CFG_COMMON_Q_SIZE_EXP);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_srq_cfg_set_rcq_size", ret);
		return;
	}
}

/*
 * rpma_srq_cfg_common_prestate_fini -- free the srq configuration object
 */
void
rpma_srq_cfg_common_prestate_fini(void *prestate, struct mtt_result *tr)
{
	struct rpma_srq_cfg_common_prestate *pr = (struct rpma_srq_cfg_common_prestate *)prestate;
	int ret;

	ret = rpma_srq_cfg_delete(&pr->cfg_ptr);
	if (ret)
		MTT_RPMA_ERR(tr, "rpma_srq_cfg_delete", ret);
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct rpma_srq_cfg_common_prestate prestate = {NULL};

	struct mtt_test test = {
			&prestate,
			rpma_srq_cfg_common_prestate_init,
			NULL,
			NULL,
			thread,
			NULL,
			NULL,
			rpma_srq_cfg_common_prestate_fini
	};

	return mtt_run(&test, args.threads_num);
}
