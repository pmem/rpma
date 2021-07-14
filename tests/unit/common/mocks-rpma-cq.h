/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2021, Fujitsu */

/*
 * mocks-rpma-cq.h -- librpma cq.c module mocks
 */

#ifndef MOCKS_RPMA_CQ_H
#define MOCKS_RPMA_CQ_H

#include "test-common.h"
#include "cq.h"

#define MOCK_RPMA_CQ		(struct rpma_cq *)0xD418
#define MOCK_RPMA_RCQ		(struct rpma_cq *)0xD419

static const struct rpma_completion Completion = {
	.op_context = MOCK_OP_CONTEXT,
	.op = RPMA_OP_RECV,
	.byte_len = MOCK_LEN,
	.op_status = IBV_WC_SUCCESS,
	.flags = IBV_WC_WITH_IMM,
	.imm = MOCK_IMM_DATA,
};

#define MOCK_COMPLETION		&Completion

#endif /* MOCKS_RPMA_CQ_H */
