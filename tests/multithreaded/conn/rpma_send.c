// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_send.c -- rpma_send multithreaded test
 */

#include <librpma.h>
#include "mtt.h"
#include "rpma_common.h"

/*
 * thread -- main function of rpma_send multithreaded test
 */
void
thread(unsigned id, void *prestate, void *state, struct mtt_result *result)
{
	struct thread_state *ts = (struct thread_state *)state;
	struct ibv_wc wc;

	/* post an RDMA send operation */
	int ret = rpma_send(ts->conn, ts->mr_local_ptr, 0, ts->mr_remote_size,
				RPMA_F_COMPLETION_ALWAYS, NULL);
	if (ret) {
		MTT_ERR_MSG(result, "rpma_send() failed", ret);
		return;
	}

	/* wait for the completion to be ready */
	ret = rpma_cq_wait(ts->cq);
	if (ret) {
		MTT_ERR_MSG(result, "rpma_cq_wait() failed", ret);
		return;
	}

	/* wait for a completion of the RDMA send */
	ret = rpma_cq_get_wc(ts->cq, 1, &wc, NULL);
	if (ret) {
		MTT_ERR_MSG(result, "rpma_cq_get_wc() failed", ret);
		return;
	}

	if (wc.status != IBV_WC_SUCCESS) {
		MTT_ERR_MSG(result, "rpma_send() failed", -1);
		return;
	}

	if (wc.opcode != IBV_WC_SEND) {
		MTT_ERR_MSG(result, "unexpected wc.opcode value", -1);
		return;
	}
}
