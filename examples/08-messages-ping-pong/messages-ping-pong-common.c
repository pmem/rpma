// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * messages-ping-pong-common.c -- a common definitions for the 08 example
 */

#include <librpma.h>
#include <inttypes.h>

#include "messages-ping-pong-common.h"

/* the maximum number of completions expected (1 of send + 1 of receive) */
#define MAX_N_WC	2

static int
validate_wc(struct ibv_wc *wc, uint64_t *recv,
		int *send_cmpl, int *recv_cmpl)
{
	if (wc->status != IBV_WC_SUCCESS) {
		char *func = (wc->opcode == IBV_WC_SEND)? "send" : "recv";
		(void) fprintf(stderr, "rpma_%s() failed: %s\n",
			func, ibv_wc_status_str(wc->status));
		return -1;
	}

	if (wc->opcode == IBV_WC_SEND) {
		*send_cmpl = 1;
	} else if (wc->opcode == IBV_WC_RECV) {
		if (wc->wr_id != (uintptr_t)recv ||
				wc->byte_len != MSG_SIZE) {
			(void) fprintf(stderr,
				"received completion is not as expected (0x%"
				PRIXPTR " != 0x%" PRIXPTR " [wc.wr_id] || %"
				PRIu32 " != %ld [wc.byte_len])\n", wc->wr_id,
				(uintptr_t)recv, wc->byte_len, MSG_SIZE);
			return -1;
		}
		*recv_cmpl = 1;
	}

	return 0;
}

int
wait_and_process_completions(struct rpma_cq *cq, uint64_t *recv,
		int *send_cmpl, int *recv_cmpl)
{
	struct ibv_wc wc[MAX_N_WC];
	int num_got;
	int ret;

	do {
		/* wait for the completion to be ready */
		if ((ret = rpma_cq_wait(cq)))
			return ret;

		/* get two next completions at most (1 of send + 1 of recv) */
		if ((ret = rpma_cq_get_wc(cq, MAX_N_WC, wc, &num_got))) {
			/* lack of completions is not an error here */
			if (ret && ret != RPMA_E_NO_COMPLETION)
				return ret;
		}

		/* validate received completions */
		for (int i = 0; i < num_got; i++) {
			ret = validate_wc(&wc[i], recv, send_cmpl, recv_cmpl);
			if (ret)
				return ret;
		}
	} while (*send_cmpl == 0 || *recv_cmpl == 0);

	return 0;
}
