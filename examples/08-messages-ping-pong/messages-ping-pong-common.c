// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * messages-ping-pong-common.c -- a common definitions for the 08 example
 */

#include <librpma.h>
#include <inttypes.h>

#include "messages-ping-pong-common.h"

int
get_wc_and_validate(struct rpma_cq *cq, uint64_t *recv,
		int *send_cmpl, int *recv_cmpl)
{
	struct ibv_wc wc;
	int num_entries_got;
	int ret;

	if ((ret = rpma_cq_wait(cq)))
		return ret;

	do {
		if ((ret = rpma_cq_get_wc(cq, 1, &wc, &num_entries_got))) {
			if (ret == RPMA_E_NO_COMPLETION)
				ret = 0;
			break;
		}

		if (wc.status != IBV_WC_SUCCESS) {
			(void) fprintf(stderr,
				"rpma_send()/rpma_recv() failed: %s\n",
				ibv_wc_status_str(wc.status));
			return -1;
		}

		if (wc.opcode == IBV_WC_SEND) {
			*send_cmpl = 1;
		} else if (wc.opcode == IBV_WC_RECV) {
			if (wc.wr_id != (uintptr_t)recv ||
					wc.byte_len != MSG_SIZE) {
				(void) fprintf(stderr,
					"received completion is not as expected (0x%"
					PRIXPTR " != 0x%" PRIXPTR
					" [wc.wr_id] || %" PRIu32
					" != %ld [wc.byte_len])\n", wc.wr_id,
					(uintptr_t)recv, wc.byte_len, MSG_SIZE);
				return -1;
			}
			*recv_cmpl = 1;
		}
	} while (num_entries_got == 1);

	return ret;
}