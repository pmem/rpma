// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_send.c -- rpma_send multithreaded test
 */

#include <librpma.h>
#include "mtt.h"
#include "rpma_conn_common.h"

/*
 * thread -- main function of rpma_send multithreaded test
 */
void
thread(unsigned id, void *prestate, void *state, struct mtt_result *result)
{
	struct thread_state *ts = (struct thread_state *)state;

	/* post an RDMA send operation */
	int ret = rpma_send(ts->conn, ts->mr_local_ptr, 0, ts->mr_remote_size,
				RPMA_F_COMPLETION_ALWAYS, (void *)WR_ID_SEND);
	if (ret) {
		MTT_ERR_MSG(result, "rpma_send() failed", ret);
		return;
	}

	(void) wait_and_validate_completion(ts->cq, IBV_WC_SEND, WR_ID_SEND, result);
}
