// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_write_read.c -- rpma_write with rpma_read verification multithreaded test
 */

#include <librpma.h>
#include "mtt.h"
#include "rpma_common.h"

/*
 * thread -- main function of rpma_write with rpma_read verification MT test
 *           (write and verify the data using rpma_read)
 */
void
thread(unsigned id, void *prestate, void *state, struct mtt_result *result)
{
	struct thread_state *ts = (struct thread_state *)state;
	struct ibv_wc wc;

	/* post an RDMA write operation */
	int ret = rpma_write(ts->conn, ts->mr_remote_ptr, 0, ts->mr_local_ptr, 0,
				ts->mr_remote_size, RPMA_F_COMPLETION_ALWAYS, NULL);
	if (ret) {
		MTT_ERR_MSG(result, "rpma_write() failed", ret);
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
		MTT_ERR_MSG(result, "rpma_write() failed", -1);
		return;
	}

	if (wc.opcode != IBV_WC_RDMA_WRITE) {
		MTT_ERR_MSG(result, "unexpected wc.opcode value", -1);
		return;
	}

	/* zero the local memory as the destination for reading */
	memset(ts->local_ptr, 0, ts->mr_local_size);

	/* post an RDMA read operation */
	ret = rpma_read(ts->conn, ts->mr_local_ptr, 0, ts->mr_remote_ptr, 0,
				ts->mr_remote_size, RPMA_F_COMPLETION_ALWAYS, NULL);
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

	if (memcmp(ts->local_ptr, STRING_TO_WRITE, LEN_STRING_TO_WRITE) != 0)
		MTT_ERR_MSG(result, "write string mismatch", -1);
}
