/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * test-common.h -- a test's common header
 */

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#define MOCK_IP_ADDRESS		"127.0.0.1"
#define MOCK_SERVICE		"1234" /* a random port number */

extern const char Private_data[];
extern const char Private_data_2[];

/* random values */
#define MOCK_PEER		(struct rpma_peer *)0xFEEF
#define MOCK_INFO		(struct rpma_info *)0xE6B2
#define MOCK_CONN		(struct rpma_conn *)0xC004
#define MOCK_PRIVATE_DATA	((void *)Private_data)
#define MOCK_PDATA_LEN		(strlen(MOCK_PRIVATE_DATA) + 1)
#define MOCK_PRIVATE_DATA_2	((void *)Private_data_2)
#define MOCK_PDATA_LEN_2	(strlen(MOCK_PRIVATE_DATA_2) + 1)
#define MOCK_ERRNO		123456

#define MOCK_OK 0

#define MOCK_PASSTHROUGH	0
#define MOCK_VALIDATE		1

#endif /* TEST_COMMON_H */
