// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Fujitsu */

/*
 * mocks-rpma-cq.c -- librpma cq.c module mocks
 */

#include "librpma.h"

#include "cmocka_headers.h"
#include "test-common.h"
#include "mocks-rpma-cq.h"

static struct rpma_completion Completion = {
	.op_context = MOCK_OP_CONTEXT,
	.op = RPMA_OP_RECV,
	.byte_len = MOCK_LEN,
	.op_status = IBV_WC_SUCCESS,
	.flags = IBV_WC_WITH_IMM,
	.imm = MOCK_IMM_DATA,
};

#define MOCK_COMPLETION		&Completion

/*
 * rpma_cq_get_fd -- rpma_cq_get_fd() mock
 */
int
rpma_cq_get_fd(struct rpma_cq *cq, int *fd)
{
	assert_ptr_equal(cq, MOCK_RPMA_CQ);
	assert_non_null(fd);

	*fd = mock_type(int);

	/*
	 * XXX so far this function cannot fail.
	 * It will fail when it will become public.
	 */
	return 0;
}

/*
 * rpma_cq_wait -- rpma_cq_wait() mock
 */
int
rpma_cq_wait(struct rpma_cq *cq)
{
	assert_ptr_equal(cq, MOCK_RPMA_CQ);

	return mock_type(int);
}

/*
 * rpma_cq_get -- rpma_cq_get() mock
 */
int
rpma_cq_get(struct rpma_cq *cq, struct rpma_completion *cmpl)
{
	assert_ptr_equal(cq, MOCK_RPMA_CQ);
	assert_non_null(cmpl);

	int result = mock_type(int);
	if (result == MOCK_OK)
		memcpy(cmpl, MOCK_COMPLETION, sizeof(struct rpma_completion));

	return result;
}

/*
 * rpma_cq_new -- rpma_cq_new() mock
 */
int
rpma_cq_new(struct ibv_context *verbs, int cqe, struct rpma_cq **cq_ptr)
{
	assert_non_null(verbs);
	check_expected(cqe);
	assert_non_null(cq_ptr);

	struct rpma_cq *cq = mock_type(struct rpma_cq *);
	if (!cq) {
		int result = mock_type(int);
		/* XXX validate the errno handling */
		if (result == RPMA_E_PROVIDER)
			errno = mock_type(int);

		return result;
	}

	*cq_ptr = cq;
	return 0;
}

/*
 * rpma_cq_delete -- rpma_cq_delete() mock
 */
int
rpma_cq_delete(struct rpma_cq **cq_ptr)
{
	assert_non_null(cq_ptr);

	struct rpma_cq *cq = *cq_ptr;
	assert_ptr_equal(cq, MOCK_RPMA_CQ);

	int result = mock_type(int);
	/* XXX validate the errno handling */
	if (result == RPMA_E_PROVIDER)
		errno = mock_type(int);

	*cq_ptr = NULL;

	return result;
}

/*
 * rpma_cq_get_ibv_cq -- rpma_cq_get_ibv_cq() mock
 */
struct ibv_cq *
rpma_cq_get_ibv_cq(struct rpma_cq *cq)
{
	assert_ptr_equal(cq, MOCK_RPMA_CQ);

	return mock_type(struct ibv_cq *);
}
