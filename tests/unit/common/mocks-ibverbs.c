/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mock-ibverbs.c -- libibverbs mocks
 */

#include "conn_req.h"
#include "cmocka_headers.h"
#include "test-common.h"

struct ibv_context Ibv_context;	/* mock IBV context */
struct ibv_cq Ibv_cq;		/* mock IBV CQ */

/*
 * ibv_create_cq -- ibv_create_cq() mock
 */
struct ibv_cq *
ibv_create_cq(struct ibv_context *context, int cqe, void *cq_context,
		struct ibv_comp_channel *channel, int comp_vector)
{
	assert_ptr_equal(context, MOCK_VERBS);
	assert_int_equal(cqe, RPMA_DEFAULT_Q_SIZE);
	assert_null(channel);
	assert_int_equal(comp_vector, 0);

	struct ibv_cq *cq = mock_type(struct ibv_cq *);
	if (!cq) {
		errno = mock_type(int);
		return NULL;
	}

	return cq;
}

/*
 * ibv_destroy_cq -- ibv_destroy_cq() mock
 */
int
ibv_destroy_cq(struct ibv_cq *cq)
{
	assert_int_equal(cq, MOCK_IBV_CQ);

	return mock_type(int);
}
