/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn-test-common.c -- the connection unit tests common definitions
 */

#ifndef CONN_COMMON
#define CONN_COMMON

#include "cmocka_headers.h"
#include "test-common.h"
#include "conn.h"

#define MOCK_RPMA_MR_LOCAL	(struct rpma_mr_local *)0xC411
#define MOCK_RPMA_MR_REMOTE	(struct rpma_mr_remote *)0xC412
#define MOCK_LOCAL_OFFSET	(size_t)0xC413
#define MOCK_REMOTE_OFFSET	(size_t)0xC414
#define MOCK_FLAGS		(int)0xC416
#define MOCK_WC_STATUS		(int)0x51A5

/* all the resources used between conn_setup and conn_teardown */
struct conn_test_state {
	struct rpma_conn *conn;
	struct rpma_conn_private_data data;
};

int conn_setup(void **cstate_ptr);
int conn_teardown(void **cstate_ptr);

int group_setup_next_completion(void **unused);
int group_setup_read(void **unused);
int group_setup_write(void **unused);

#endif /* CONN_COMMON */
