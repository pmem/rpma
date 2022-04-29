// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_read.c -- rpma_read multithreaded test
 */

#include <librpma.h>
#include "mtt.h"
#include "rpma_common.h"

/*
 * thread -- main function of rpma_read multithreaded test (read and verify the data)
 */
void
thread(unsigned id, void *prestate, void *state, struct mtt_result *result)
{
	struct thread_state *ts = (struct thread_state *)state;
	struct ibv_wc wc;

	/* zero the destination memory */
	memset(ts->local_ptr, 0, ts->mr_local_size);

	/* post an RDMA read operation */
	int ret = rpma_read(ts->conn, ts->mr_local_ptr, 0, ts->mr_remote_ptr, 0, ts->mr_remote_size,
			RPMA_F_COMPLETION_ALWAYS, NULL);
	if (ret) {
		MTT_ERR_MSG(result, "rpma_read() failed", ret);
		return;
	}

	/* wait for the completion to be ready */
	ret = rpma_cq_wait(ts->cq);
	if (ret) {
		MTT_ERR_MSG(result, "rpma_cq_wait() failed", ret);
		return;
	}

	/* wait for a completion of the RDMA read */
	ret = rpma_cq_get_wc(ts->cq, 1, &wc, NULL);
	if (ret) {
		MTT_ERR_MSG(result, "rpma_cq_get_wc() failed", ret);
		return;
	}

	if (wc.status != IBV_WC_SUCCESS) {
		MTT_ERR_MSG(result, "rpma_read() failed", -1);
		return;
	}

	if (wc.opcode != IBV_WC_RDMA_READ) {
		MTT_ERR_MSG(result, "unexpected wc.opcode value", -1);
		return;
	}

	if (memcmp(ts->local_ptr, STRING_TO_READ, LEN_STRING_TO_READ) != 0)
		MTT_ERR_MSG(result, "read string mismatch", -1);
}
