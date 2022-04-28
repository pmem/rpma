/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2022, Intel Corporation */

/*
 * receive-completion-queue-common.h -- a common declarations for the 12 example
 */

#ifndef EXAMPLES_RECEIVE_COMPLETION_QUEUE_COMMON
#define EXAMPLES_RECEIVE_COMPLETION_QUEUE_COMMON

#define MSG_SIZE sizeof(uint64_t)

/* Both buffers are allocated one after another. */
#define RECV_OFFSET	0
#define SEND_OFFSET	MSG_SIZE

#define RCQ_SIZE	10

#define I_M_DONE	(uint64_t)UINT64_MAX

#endif /* EXAMPLES_RECEIVE_COMPLETION_QUEUE_COMMON */
