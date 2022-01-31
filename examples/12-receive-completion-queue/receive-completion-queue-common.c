/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * receive-completion-queue-common.c -- a common declarations for the 12 example
 */

#include <librpma.h>
#include <inttypes.h>

#include "receive-completion-queue-common.h"

int
get_wc_and_validate(struct ibv_wc wc, struct rpma_cq *cq,
		enum ibv_wc_opcode opcode, char *func_name)
{
	int ret = rpma_cq_get_wc(cq, 1, &wc, NULL);
	if (ret && ret != RPMA_E_NO_COMPLETION)
		return ret;

	if (ret == RPMA_E_NO_COMPLETION) {
		if ((ret = rpma_cq_wait(cq))) {
			return ret;
		} else if ((ret = rpma_cq_get_wc(cq, 1,
				&wc, NULL))) {
			return ret;
		}
	}

	if (wc.status != IBV_WC_SUCCESS) {
		(void) fprintf(stderr,
			"%s failed: %s\n",
			func_name, ibv_wc_status_str(wc.status));
		ret = -1;
		return ret;
	}

	if (wc.opcode != opcode) {
		(void) fprintf(stderr,
			"unexpected wc.opcode value "
			"(0x%" PRIXPTR " != 0x%" PRIXPTR ")\n",
			(uintptr_t)wc.opcode,
			(uintptr_t)opcode);
		ret = -1;
		return ret;
	}
	return 0;
}
