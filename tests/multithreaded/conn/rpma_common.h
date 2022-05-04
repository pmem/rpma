/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2021-2022, Intel Corporation */

/*
 * rpma_common.h -- common definition of multi-connection MT tests
 */

#ifndef MTT_RPMA_RW_COMMON
#define MTT_RPMA_RW_COMMON

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define STRING_TO_READ "This string was read using RDMA read"
#define STRING_TO_WRITE "This string was written using RDMA write"
#define LEN_STRING_TO_READ (strlen(STRING_TO_READ) + 1)
#define LEN_STRING_TO_WRITE (strlen(STRING_TO_WRITE) + 1)
#define MAX_STR_LEN (MAX(LEN_STRING_TO_WRITE, LEN_STRING_TO_READ))

#define WR_ID_READ	((uint64_t)0x1111111111111111)
#define WR_ID_WRITE	((uint64_t)0x2222222222222222)

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

/*
 * thread - test-specific function implemented by each MT test separately
 */
void thread(unsigned id, void *prestate, void *state, struct mtt_result *result);

/*
 * wait_and_validate_completion -- wait for the completion to be ready and validate it
 */
int wait_and_validate_completion(struct rpma_cq *cq, enum ibv_wc_opcode expected_opcode,
				uint64_t wr_id, struct mtt_result *result);

#endif /* MTT_RPMA_RW_COMMON */
