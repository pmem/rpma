// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Fujitsu */

/*
 * mocks-rpma-cq.c -- librpma cq.c module mocks
 */

#include "librpma.h"

#include "cmocka_headers.h"
#include "mocks-rpma-cq.h"

/*
 * rpma_cq_get_fd -- rpma_cq_get_fd() mock
 */
int
rpma_cq_get_fd(const struct rpma_cq *cq, int *fd)
{
	assert_ptr_equal(cq, MOCK_RPMA_CQ);
	assert_non_null(fd);

	*fd = mock_type(int);

	/*
	 * XXX so far this function cannot fail.
	 * It will be able to fail when it becomes public.
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
 * rpma_cq_get_completion -- rpma_cq_get_completion() mock
 */
int
rpma_cq_get_completion(struct rpma_cq *cq, struct rpma_completion *cmpl)
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
rpma_cq_new(struct ibv_context *dev, int cqe, struct rpma_cq **cq_ptr)
{
	assert_non_null(dev);
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
	check_expected_ptr(*cq_ptr);

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
rpma_cq_get_ibv_cq(const struct rpma_cq *cq)
{
	check_expected_ptr(cq);

	return mock_type(struct ibv_cq *);
}
