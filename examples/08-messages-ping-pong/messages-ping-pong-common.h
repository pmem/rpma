/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2022, Intel Corporation */

/*
 * messages-ping-pong-common.h -- a common declarations for the 08 example
 */

#ifndef EXAMPLES_MSG_PING_PONG_COMMON
#define EXAMPLES_MSG_PING_PONG_COMMON

#define MSG_SIZE sizeof(uint64_t)

/* Both buffers are allocated one after another. */
#define RECV_OFFSET	0
#define SEND_OFFSET	MSG_SIZE

#define I_M_DONE	(uint64_t)UINT64_MAX

int wait_and_process_completions(struct rpma_cq *cq, uint64_t *recv,
		int *send_cmpl, int *recv_cmpl);

#endif /* EXAMPLES_MSG_PING_PONG_COMMON */
