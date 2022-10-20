/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * flush-to-persistent-GSPSPM.h -- a common declarations for the 09 example
 */

#ifndef FLUSH_TO_PERSISTENT_GPSPM
#define FLUSH_TO_PERSISTENT_GPSPM

#define MSG_SIZE_MAX 512

#define SEND_OFFSET	0
#define RECV_OFFSET	(SEND_OFFSET + MSG_SIZE_MAX)

#define RCQ_SIZE	1

#endif /* FLUSH_TO_PERSISTENT_GPSPM */
