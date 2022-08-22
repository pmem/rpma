// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021-2022, Intel Corporation */

/*
 * rpma_conn_cfg_common.c -- common part for rpma_conn_cfg_* multithreaded tests
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "rpma_conn_cfg_common.h"

/*
 * rpma_conn_cfg_common_prestate_init -- create a new connection
 * configuration object, set all queue sizes and timeout value
 */
void
rpma_conn_cfg_common_prestate_init(void *prestate, struct mtt_result *tr)
{
	struct rpma_conn_cfg_common_prestate *pr =
		(struct rpma_conn_cfg_common_prestate *)prestate;
	int ret;

	if ((ret = rpma_conn_cfg_new(&pr->cfg_ptr))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_new", ret);
		return;
	}

	if ((ret = rpma_conn_cfg_set_compl_channel(pr->cfg_ptr, RPMA_CONN_CFG_COMMON_IS_SHARED))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_set_compl_channel", ret);
		return;
	}

	if ((ret = rpma_conn_cfg_set_cq_size(pr->cfg_ptr, RPMA_CONN_CFG_COMMON_Q_SIZE_EXP))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_set_cq_size", ret);
		return;
	}

	if ((ret = rpma_conn_cfg_set_sq_size(pr->cfg_ptr, RPMA_CONN_CFG_COMMON_Q_SIZE_EXP))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_set_sq_size", ret);
		return;
	}

	if ((ret = rpma_conn_cfg_set_rcq_size(pr->cfg_ptr, RPMA_CONN_CFG_COMMON_Q_SIZE_EXP))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_set_rcq_size", ret);
		return;
	}

	if ((ret = rpma_conn_cfg_set_rq_size(pr->cfg_ptr, RPMA_CONN_CFG_COMMON_Q_SIZE_EXP))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_set_rq_size", ret);
		return;
	}

	if ((ret = rpma_conn_cfg_set_timeout(pr->cfg_ptr, RPMA_CONN_CFG_COMMON_TIMEOUT_MS_EXP))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_set_timeout", ret);
		return;
	}

	if ((ret = rpma_conn_cfg_set_srq(pr->cfg_ptr, RPMA_SRQ_EXP))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_set_srq", ret);
		return;
	}
}

/*
 * rpma_conn_cfg_common_prestate_fini -- free the connection configuration
 * object
 */
void
rpma_conn_cfg_common_prestate_fini(void *prestate, struct mtt_result *tr)
{
	struct rpma_conn_cfg_common_prestate *pr =
		(struct rpma_conn_cfg_common_prestate *)prestate;
	int ret;

	if ((ret = rpma_conn_cfg_delete(&pr->cfg_ptr)))
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_delete", ret);
}
