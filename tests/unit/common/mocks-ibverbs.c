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

struct ibv_comp_channel Ibv_comp_channel; /* mock IBV completion channel */
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
	assert_ptr_equal(channel, MOCK_COMP_CHANNEL);
	assert_int_equal(comp_vector, 0);

	struct ibv_cq *cq = mock_type(struct ibv_cq *);
	if (!cq) {
		errno = mock_type(int);
		return NULL;
	}

	cq->channel = channel;

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

/*
 * ibv_create_comp_channel -- ibv_create_comp_channel() mock
 */
struct ibv_comp_channel *
ibv_create_comp_channel(struct ibv_context *context)
{
	assert_ptr_equal(context, MOCK_VERBS);

	struct ibv_comp_channel *channel = mock_type(struct ibv_comp_channel *);
	if (!channel) {
		errno = mock_type(int);
		return NULL;
	}

	return channel;
}

/*
 * ibv_destroy_comp_channel -- ibv_destroy_comp_channel() mock
 */
int
ibv_destroy_comp_channel(struct ibv_comp_channel *channel)
{
	assert_ptr_equal(channel, MOCK_COMP_CHANNEL);

	return mock_type(int);
}

/*
 * ibv_req_notify_cq_mock -- ibv_req_notify_cq() mock
 */
int
ibv_req_notify_cq_mock(struct ibv_cq *cq, int solicited_only)
{
	assert_ptr_equal(cq, MOCK_IBV_CQ);
	assert_int_equal(solicited_only, 0);

	return mock_type(int);
}

/*
 * ibv_get_cq_event -- ibv_get_cq_event() mock
 */
int
ibv_get_cq_event(struct ibv_comp_channel *channel, struct ibv_cq **cq,
		void **cq_context)
{
	check_expected_ptr(channel);
	assert_non_null(cq);
	assert_non_null(cq_context);

	errno = mock_type(int);
	if (!errno) {
		*cq = mock_type(struct ibv_cq *);
		*cq_context = NULL;
		return 0;
	}

	return -1;
}

/*
 * ibv_ack_cq_events -- ibv_ack_cq_events() mock
 */
void
ibv_ack_cq_events(struct ibv_cq *cq, unsigned nevents)
{
	check_expected_ptr(cq);
	assert_int_equal(nevents, 1);
}
