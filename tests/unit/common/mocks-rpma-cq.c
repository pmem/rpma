// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */
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
 * rpma_cq_new -- rpma_cq_new() mock
 */
int
rpma_cq_new(struct ibv_context *ibv_ctx, int cqe,
		struct ibv_comp_channel *shared_channel,
		struct rpma_cq **cq_ptr)
{
	assert_non_null(ibv_ctx);
	check_expected(cqe);
	/*
	 * XXXXXX finish the mock for this function.
	 * check_expected(shared_channel); should be used here.
	 */
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

/*
 * rpma_ibv_create_comp_channel -- rpma_ibv_create_comp_channel() mock
 */
int
rpma_ibv_create_comp_channel(struct ibv_context *ibv_ctx,
		struct ibv_comp_channel **channel_ptr)
{
	struct state_rpma_ibv_create_comp_channel *state;
	state = mock_type(struct state_rpma_ibv_create_comp_channel *);

	assert_ptr_equal(ibv_ctx, state->ibv_ctx);
	assert_non_null(channel_ptr);
	*channel_ptr = state->channel;

	return mock_type(int);
}

/*
 * rpma_ibv_destroy_comp_channel -- rpma_ibv_destroy_comp_channel() mock
 */
int
rpma_ibv_destroy_comp_channel(struct ibv_comp_channel *channel)
{
	struct state_rpma_ibv_destroy_comp_channel *state;
	state = mock_type(struct state_rpma_ibv_destroy_comp_channel *);

	assert_ptr_equal(channel, state->channel);

	return mock_type(int);
}
