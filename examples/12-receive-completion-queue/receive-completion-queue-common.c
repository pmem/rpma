// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * receive-completion-queue-common.c -- a common declarations for the 12 example
 */

#include <librpma.h>
#include <inttypes.h>

#include "receive-completion-queue-common.h"

int
get_wc_and_validate(struct rpma_cq *cq, enum ibv_wc_opcode opcode,
		char *func_name)
{
	struct ibv_wc wc;

	int ret = rpma_cq_wait(cq);
	if (ret)
			return ret;

	ret = rpma_cq_get_wc(cq, 1, &wc, NULL);
	if (ret)
			return ret;

	if (wc.status != IBV_WC_SUCCESS) {
		(void) fprintf(stderr, "%s failed: %s\n",
			func_name, ibv_wc_status_str(wc.status));
		return -1;
	}

	if (wc.opcode != opcode) {
		(void) fprintf(stderr, "unexpected wc.opcode value "
			"(0x%" PRIXPTR " != 0x%" PRIXPTR ")\n",
			(uintptr_t)wc.opcode, (uintptr_t)opcode);
		return -1;
	}
	return 0;
}
