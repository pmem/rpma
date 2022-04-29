/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2021-2022, Intel Corporation */

/*
 * rpma_rw_common.h -- common definition for rpma_read or rpma_write MT tests
 */

#ifndef MTT_RPMA_RW_COMMON
#define MTT_RPMA_RW_COMMON

#define STRING_TO_READ "This string was read using RDMA read"
#define STRING_TO_WRITE "This string was written using RDMA write"
#define LEN_STRING_TO_READ (strlen(STRING_TO_READ) + 1)
#define LEN_STRING_TO_WRITE (strlen(STRING_TO_WRITE) + 1)
#define MAX_STR_LEN \
	((LEN_STRING_TO_WRITE > LEN_STRING_TO_READ) ? LEN_STRING_TO_WRITE : LEN_STRING_TO_READ)

struct thread_state {
	struct rpma_conn *conn;
	struct rpma_cq *cq;

	void *local_ptr;
	struct rpma_mr_local *mr_local_ptr;
	struct rpma_mr_remote *mr_remote_ptr;
	size_t mr_local_size;
	size_t mr_remote_size;

	/* the expected value of the private data */
	struct rpma_conn_private_data pdata;
};

#endif /* MTT_RPMA_RW_COMMON */
