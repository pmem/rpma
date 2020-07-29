/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * mock-ibverbs.c -- libibverbs mocks
 */

#include "conn_req.h"
#include "cmocka_headers.h"
#include "mocks-ibverbs.h"

/* mocked IBV entities */
struct ibv_comp_channel Ibv_comp_channel;
struct ibv_context Ibv_context;
struct ibv_cq Ibv_cq;
struct ibv_qp Ibv_qp;
struct ibv_mr Ibv_mr;

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

/*
 * ibv_dereg_mr -- a mock of ibv_dereg_mr()
 */
int
ibv_dereg_mr(struct ibv_mr *mr)
{
	/*
	 * rpma_peer_mr_reg() and malloc() may be called in any order.
	 * If the first one fails, then the second one won't be called.
	 * ibv_dereg_mr() will be called in rpma_mr_reg() only if:
	 * 1) rpma_peer_mr_reg() succeeded and
	 * 2) malloc() failed.
	 * In the opposite case, when:
	 * 1) malloc() succeeded and
	 * 2) rpma_peer_mr_reg() failed,
	 * ibv_dereg_mr() will not be called,
	 * so we cannot add cmocka's expects here.
	 * Otherwise, unconsumed expects would cause a test failure.
	 */
	assert_int_equal(mr, MOCK_MR);

	return mock_type(int); /* errno */
}

/*
 * ibv_post_send_mock -- mock of ibv_post_send()
 */
int
ibv_post_send_mock(struct ibv_qp *qp, struct ibv_send_wr *wr,
			struct ibv_send_wr **bad_wr)
{
	struct ibv_post_send_mock_args *args =
		mock_type(struct ibv_post_send_mock_args *);

	assert_non_null(qp);
	assert_non_null(wr);
	assert_non_null(bad_wr);

	assert_int_equal(qp, args->qp);
	assert_int_equal(wr->opcode, args->opcode);
	assert_int_equal(wr->send_flags, args->send_flags);
	assert_int_equal(wr->wr_id, args->wr_id);
	assert_null(wr->next);

	return args->ret;
}

