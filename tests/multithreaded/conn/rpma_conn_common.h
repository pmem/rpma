/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2021-2022, Intel Corporation */

/*
 * rpma_conn_common.h -- common definition of multi-connection MT tests
 */

#ifndef MTT_RPMA_RW_COMMON
#define MTT_RPMA_RW_COMMON

#define DIRECT_WRITE_TO_PMEM	true

#define MAX(a, b) ( \
	{ \
		__typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a > _b ? _a : _b; \
	})

#define STRING_TO_READ_RECV "This string was read/received using RDMA read/receive"
#define STRING_TO_WRITE_SEND "This string was written/sent using RDMA write/send"
#define LEN_STRING_TO_READ_RECV (strlen(STRING_TO_READ_RECV))
#define LEN_STRING_TO_WRITE_SEND (strlen(STRING_TO_WRITE_SEND))
#define MAX_STR_LEN (MAX(LEN_STRING_TO_READ_RECV, LEN_STRING_TO_WRITE_SEND))

#define WR_ID_READ	((uint64_t)0x1111111111111111)
#define WR_ID_WRITE	((uint64_t)0x2222222222222222)
#define WR_ID_SEND	((uint64_t)0x3333333333333333)
#define WR_ID_RECV	((uint64_t)0x4444444444444444)

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

struct prestate {
	char *addr;
	unsigned port;
	struct rpma_peer *peer;
	struct rpma_peer_cfg *pcfg;
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
