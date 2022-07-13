// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_write_read.c -- rpma_write with rpma_read verification multithreaded test
 */

#include <librpma.h>
#include "mtt.h"
#include "rpma_conn_common.h"

/*
 * thread -- main function of rpma_write with rpma_read verification MT test
 *           (write and verify the data using rpma_read)
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

	ret = wait_and_validate_completion(ts->cq, IBV_WC_RDMA_WRITE, WR_ID_WRITE, result);
	if (ret)
		return;

	/* zero the local memory as the destination for reading */
	memset(ts->local_ptr, 0, ts->mr_local_size);

	/* post an RDMA read operation */
	ret = rpma_read(ts->conn, ts->mr_local_ptr, 0, ts->mr_remote_ptr, 0,
				ts->mr_remote_size, RPMA_F_COMPLETION_ALWAYS, (void *)WR_ID_READ);
	if (ret) {
		MTT_ERR_MSG(result, "rpma_read() failed", ret);
		return;
	}

	ret = wait_and_validate_completion(ts->cq, IBV_WC_RDMA_READ, WR_ID_READ, result);
	if (ret)
		return;

	if (strncmp(ts->local_ptr, STRING_TO_WRITE_SEND, LEN_STRING_TO_WRITE_SEND) != 0)
		MTT_ERR_MSG(result, "write string mismatch", -1);
}
