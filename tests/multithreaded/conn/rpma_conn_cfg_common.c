// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_conn_cfg_common.c -- common part for rpma_conn_cfg_xxx mtt
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "rpma_conn_cfg_common.h"

/*
 * rpma_conn_cfg_common_init -- allocate state and create a new connection
 * configuration object
 */
void
rpma_conn_cfg_common_init(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct state *st = (struct state *)calloc(1, sizeof(struct state));
	int ret;

	if (!st) {
		MTT_ERR(tr, "calloc", errno);
		return;
	}

	if ((ret = rpma_conn_cfg_new(&st->cfg_ptr))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_new", ret);
		return;
	}

	*state_ptr = st;
}

/*
 * rpma_conn_cfg_common_fini -- free the connection configuration object
 * and free the state
 */
void
rpma_conn_cfg_common_fini(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;
	int ret;

	if ((ret = rpma_conn_cfg_delete(&st->cfg_ptr)))
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_delete", ret);

	free(st);
	*state_ptr = NULL;
}
