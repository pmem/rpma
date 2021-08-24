/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn-common.c -- the connection unit tests common definitions
 */

#ifndef CONN_COMMON_H
#define CONN_COMMON_H 1

#include "cmocka_headers.h"
#include "conn.h"
#include "mocks-rpma-cq.h"

#define MOCK_RPMA_MR_REMOTE	((struct rpma_mr_remote *)0xC412)
#define MOCK_REMOTE_OFFSET	(size_t)0xC414
#define MOCK_OFFSET_ALIGNED	(size_t)((MOCK_REMOTE_OFFSET / \
		RPMA_ATOMIC_WRITE_ALIGNMENT) * RPMA_ATOMIC_WRITE_ALIGNMENT)
#define MOCK_FD			0x00FD
#define CONN_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_RCQ(test_func, \
		setup_func, teardown_func) \
	{#test_func "__without_rcq", (test_func), (setup_func), \
		(teardown_func), &Conn_without_rcq}, \
	{#test_func "__with_rcq", (test_func), (setup_func), \
		(teardown_func), &Conn_with_rcq}
#define CONN_TEST_WITH_AND_WITHOUT_RCQ(test_func) \
	CONN_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_RCQ(test_func, NULL, NULL)

/* all the resources used between setup__conn_new and teardown__conn_delete */
struct conn_test_state {
	struct rpma_conn *conn;
	struct rpma_conn_private_data data;
	struct rpma_cq *rcq;
};

extern struct conn_test_state Conn_without_rcq;
extern struct conn_test_state Conn_with_rcq;

int setup__conn_new(void **cstate_ptr);
int teardown__conn_delete(void **cstate_ptr);

#endif /* CONN_COMMON_H */
