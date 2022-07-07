// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_conn_apply_remote_peer_cfg.c -- rpma_conn_apply_remote_peer_cfg multithreaded test
 */

#include <librpma.h>
#include "mtt.h"
#include "rpma_common.h"

/*
 * thread -- main function of rpma_conn_apply_remote_peer_cfg multithreaded test
 */
void
thread(unsigned id, void *prestate, void *state, struct mtt_result *result)
{
	struct thread_state *ts = (struct thread_state *)state;

	/* post an RDMA read operation */
	int ret = rpma_conn_apply_remote_peer_cfg(ts->conn, ts->pcfg);
	if (ret) {
		MTT_ERR_MSG(result, "rpma_conn_apply_remote_peer_cfg() failed", ret);
		return;
	}
}
