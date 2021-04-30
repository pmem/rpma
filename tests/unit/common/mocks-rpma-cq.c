// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Fujitsu */

/*
 * mocks-rpma-cq.c -- librpma cq.c module mocks
 */

#include "librpma.h"

#include "cmocka_headers.h"
#include "mocks-ibverbs.h"
#include "cq.h"
#include "mocks-rpma-cq.h"

struct rpma_cq Rpma_cq = {
	.channel = MOCK_COMP_CHANNEL,
	.cq = MOCK_IBV_CQ,
};

/*
 * rpma_cq_get_fd -- rpma_cq_get_fd() mock
 */
int
rpma_cq_get_fd(struct rpma_cq *cq, int *fd)
{
	assert_ptr_equal(cq, MOCK_RPMA_CQ);
	assert_non_null(fd);

	*fd = mock_type(int);

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

	return mock_type(int);
}

/*
 * rpma_cq_create -- rpma_cq_create() mock
 */
int
rpma_cq_create(struct ibv_context *verbs, int cqe, struct rpma_cq **cq_ptr)
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
 * rpma_cq_destroy -- rpma_cq_destroy() mock
 */
int
rpma_cq_destroy(struct rpma_cq **cq_ptr)
{
	assert_non_null(cq_ptr);

	int result = mock_type(int);
	/* XXX validate the errno handling */
	if (result == RPMA_E_PROVIDER)
		errno = mock_type(int);

	return result;
}
