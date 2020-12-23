/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * messages-ping-pong-use-srq-common.h -- a common declarations
 * for the 10 example
 */

#ifndef EXAMPLES_MSG_PING_PONG_USE_SRQ_COMMON
#define EXAMPLES_MSG_PING_PONG_USE_SRQ_COMMON

#define MSG_SIZE sizeof(uint64_t)

/* Both buffers are allocated one after another. */
#define RECV_OFFSET	0
#define SEND_OFFSET	MSG_SIZE

#define I_M_DONE	(uint64_t)UINT64_MAX

#endif /* EXAMPLES_MSG_PING_USE_SRQ_PONG_COMMON */
