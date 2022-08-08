/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020-2022, Intel Corporation */

/*
 * common-messages-ping-pong.h -- a common declarations for messages ping-pong
 */

#ifndef COMMON_MSG_PING_PONG
#define COMMON_MSG_PING_PONG

#define MSG_SIZE sizeof(uint64_t)

/* Both buffers are allocated one after another. */
#define RECV_OFFSET	0
#define SEND_OFFSET	MSG_SIZE

#define I_M_DONE	(uint64_t)UINT64_MAX

/* the maximum number of completions expected (1 of send + 1 of receive) */
#define MAX_N_WC	2

int wait_and_process_completions(struct rpma_cq *cq, uint64_t *recv,
		int *send_cmpl, int *recv_cmpl);

int validate_wc(struct ibv_wc *wc, uint64_t *recv,
		int *send_cmpl, int *recv_cmpl);

#endif /* COMMON_MSG_PING_PONG */
