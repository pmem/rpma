// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_write.c -- rpma_write multithreaded test
 */

#include <librpma.h>
#include "mtt.h"
#include "rpma_conn_common.h"

/*
 * thread -- main function of rpma_write multithreaded test
 */
void
thread(unsigned id, void *prestate, void *state, struct mtt_result *result)
{
	struct thread_state *ts = (struct thread_state *)state;

	/* post an RDMA write operation */
	int ret = rpma_write(ts->conn, ts->mr_remote_ptr, 0, ts->mr_local_ptr, 0,
			ts->mr_remote_size, RPMA_F_COMPLETION_ALWAYS, (void *)WR_ID_WRITE);
	if (ret) {
		MTT_ERR_MSG(result, "rpma_write() failed", ret);
		return;
	}

	(void) wait_and_validate_completion(ts->cq, IBV_WC_RDMA_WRITE, WR_ID_WRITE, result);
}
