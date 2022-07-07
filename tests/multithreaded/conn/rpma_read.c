// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_read.c -- rpma_read multithreaded test
 */

#include <librpma.h>
#include "mtt.h"
#include "rpma_conn_common.h"

/*
 * thread -- main function of rpma_read multithreaded test (read and verify the data)
 */
void
thread(unsigned id, void *prestate, void *state, struct mtt_result *result)
{
	struct thread_state *ts = (struct thread_state *)state;

	/* zero the destination memory */
	memset(ts->local_ptr, 0, ts->mr_local_size);

	/* post an RDMA read operation */
	int ret = rpma_read(ts->conn, ts->mr_local_ptr, 0, ts->mr_remote_ptr, 0, ts->mr_remote_size,
			RPMA_F_COMPLETION_ALWAYS, (void *)WR_ID_READ);
	if (ret) {
		MTT_ERR_MSG(result, "rpma_read() failed", ret);
		return;
	}

	ret = wait_and_validate_completion(ts->cq, IBV_WC_RDMA_READ, WR_ID_READ, result);
	if (ret)
		return;

	if (strncmp(ts->local_ptr, STRING_TO_READ_RECV, LEN_STRING_TO_READ_RECV) != 0)
		MTT_ERR_MSG(result, "read string mismatch", -1);
}
