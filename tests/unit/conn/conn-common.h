/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * conn-common.c -- the connection unit tests common definitions
 */

#ifndef CONN_COMMON_H
#define CONN_COMMON_H 1

#include "cmocka_headers.h"
#include "test-common.h"
#include "conn.h"

#define MOCK_RPMA_MR_REMOTE	((struct rpma_mr_remote *)0xC412)
#define MOCK_REMOTE_OFFSET	(size_t)0xC414
#define MOCK_OFFSET_ALIGNED	(size_t)((MOCK_REMOTE_OFFSET / \
		RPMA_ATOMIC_WRITE_ALIGNMENT) * RPMA_ATOMIC_WRITE_ALIGNMENT)
#define MOCK_WC_STATUS		(int)0x51A5
#define MOCK_FD			0x00FD
#define MOCK_COMPLETION_FD	0x00FE

/* all the resources used between setup__conn_new and teardown__conn_delete */
struct conn_test_state {
	struct rpma_conn *conn;
	struct rpma_conn_private_data data;
};

int setup__conn_new(void **cstate_ptr);
int teardown__conn_delete(void **cstate_ptr);

#endif /* CONN_COMMON_H */
