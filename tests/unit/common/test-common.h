/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * test-common.h -- a test's common header
 */

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <string.h>

#define MOCK_IP_ADDRESS		"127.0.0.1"
#define MOCK_PORT		"1234" /* a random port number */
#define MOCK_TIMEOUT_MS		5678
#define MOCK_Q_SIZE		123
#define MOCK_IMM_DATA		0x87654321

/* random values */
#define MOCK_RPMA_MR_LOCAL	(struct rpma_mr_local *)0xC411
#define MOCK_PEER		(struct rpma_peer *)0xFEEF
#define MOCK_INFO		(struct rpma_info *)0xE6B2
#define MOCK_CONN		(struct rpma_conn *)0xC004
#define MOCK_PEER_PCFG		(struct rpma_peer_cfg *)0xA1D2
#define MOCK_PRIVATE_DATA	((void *)Private_data)
#define MOCK_PDATA_LEN		(strlen(MOCK_PRIVATE_DATA) + 1)
#define MOCK_PRIVATE_DATA_2	((void *)Private_data_2)
#define MOCK_PDATA_LEN_2	(strlen(MOCK_PRIVATE_DATA_2) + 1)
#define MOCK_LOCAL_OFFSET	(size_t)0xC413
#define MOCK_LEN		(size_t)0xC415
#define MOCK_FLAGS		(int)0xC416
#define MOCK_OP_CONTEXT		(void *)0xC417
#define MOCK_NOFENCE		false
#define MOCK_FENCE		true
#define MOCK_COMPLETION_FD	0x00FE
#define MOCK_QP_NUM		1289

#define MOCK_OK			0
#define MOCK_ERRNO		123456
#define MOCK_ERRNO2		234567

#define MOCK_PASSTHROUGH	0
#define MOCK_VALIDATE		1
#define MOCK_ERR_PENDING	(-1)

extern const char Private_data[];
extern const char Private_data_2[];

#endif /* TEST_COMMON_H */
