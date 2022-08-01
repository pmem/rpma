// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */
/* Copyright 2021-2022, Fujitsu */

/*
 * mock-ibverbs.c -- libibverbs mocks
 */

#include <string.h>

#include "cmocka_headers.h"
#include "mocks-ibverbs.h"
#include "mocks-rpma-conn_cfg.h"
#include "test-common.h"

/* mocked IBV entities */
struct verbs_context Verbs_context;
struct ibv_comp_channel Ibv_comp_channel;
struct ibv_device Ibv_device;
struct ibv_context Ibv_context = {&Ibv_device};
struct ibv_pd Ibv_pd = {&Ibv_context, 0};
struct ibv_cq Ibv_cq;
struct ibv_cq Ibv_rcq;
struct ibv_cq Ibv_srq_rcq;
struct ibv_cq Ibv_cq_unknown;
struct ibv_qp Ibv_qp;
struct ibv_mr Ibv_mr;
struct ibv_srq Ibv_srq;

/*
 * ibv_query_device -- ibv_query_device() mock
 */
int
ibv_query_device(struct ibv_context *ibv_ctx,
		struct ibv_device_attr *device_attr)
{
	assert_ptr_equal(ibv_ctx, MOCK_VERBS);
	assert_non_null(device_attr);

	int ret = mock_type(int);
	if (ret)
		return ret;

	memset(device_attr, 0, sizeof(struct ibv_device_attr));

	return 0;
}

#ifdef ON_DEMAND_PAGING_SUPPORTED
/*
 * ibv_query_device_ex_mock -- ibv_query_device_ex() mock
 */
int
ibv_query_device_ex_mock(struct ibv_context *ibv_ctx,
		const struct ibv_query_device_ex_input *input,
		struct ibv_device_attr_ex *attr,
		size_t attr_size)
{
	assert_ptr_equal(ibv_ctx, MOCK_VERBS);
	assert_null(input);
	assert_non_null(attr);
	/* attr_size is provided by ibverbs - no validation needed */

	struct ibv_odp_caps *caps = mock_type(struct ibv_odp_caps *);
	if (caps == NULL)
		return mock_type(int);

	memcpy(&attr->odp_caps, caps, sizeof(struct ibv_odp_caps));

	return 0;
}
#endif

/*
 * ibv_create_cq -- ibv_create_cq() mock
 */
struct ibv_cq *
ibv_create_cq(struct ibv_context *ibv_ctx, int cqe, void *cq_context,
		struct ibv_comp_channel *channel, int comp_vector)
{
	assert_ptr_equal(ibv_ctx, MOCK_VERBS);
	check_expected(cqe);
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
ibv_create_comp_channel(struct ibv_context *ibv_ctx)
{
	assert_ptr_equal(ibv_ctx, MOCK_VERBS);

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

#if defined(ibv_reg_mr)
/*
 * Since rdma-core v27.0-105-g5a750676
 * ibv_reg_mr() has been defined as a macro
 * in <infiniband/verbs.h>:
 *
 * https://github.com/linux-rdma/rdma-core/commit/5a750676e8312715100900c6336bbc98577e082b
 *
 * In order to mock the ibv_reg_mr() function
 * the `ibv_reg_mr` symbol has to be undefined first
 * and the additional ibv_reg_mr_iova2() function
 * has to be mocked, because it is called
 * by the 'ibv_reg_mr' macro.
 */
#undef ibv_reg_mr

/*
 * ibv_reg_mr_iova2 -- ibv_reg_mr_iova2() mock
 */
struct ibv_mr *
ibv_reg_mr_iova2(struct ibv_pd *pd, void *addr, size_t length,
			uint64_t iova, unsigned access)
{
	return ibv_reg_mr(pd, addr, length, (int)access);
}
#endif

/*
 * ibv_reg_mr -- ibv_reg_mr() mock
 */
struct ibv_mr *
ibv_reg_mr(struct ibv_pd *pd, void *addr, size_t length, int access)
{
	check_expected_ptr(pd);
	check_expected_ptr(addr);
	check_expected(length);
	check_expected(access);

	struct ibv_mr *mr = mock_type(struct ibv_mr *);
	if (mr == NULL) {
		errno = mock_type(int);
		return NULL;
	}

	return mr;
}

/*
 * ibv_req_notify_cq_mock -- ibv_req_notify_cq() mock
 */
int
ibv_req_notify_cq_mock(struct ibv_cq *cq, int solicited_only)
{
	check_expected_ptr(cq);
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
	 * rpma_peer_setup_mr_reg() and malloc() may be called in any order.
	 * If the first one fails, then the second one won't be called.
	 * ibv_dereg_mr() will be called in rpma_mr_reg() only if:
	 * 1) rpma_peer_setup_mr_reg() succeeded and
	 * 2) malloc() failed.
	 * In the opposite case, when:
	 * 1) malloc() succeeded and
	 * 2) rpma_peer_setup_mr_reg() failed,
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
	/*
	 * XXX all wr fields should be validated to avoid
	 * posting uninitialized values
	 */
	assert_int_equal(wr->opcode, args->opcode);
	assert_int_equal(wr->send_flags, args->send_flags);
	assert_int_equal(wr->wr_id, args->wr_id);
	if (args->opcode != IBV_WR_SEND &&
	    args->opcode != IBV_WR_SEND_WITH_IMM) {
		assert_int_equal(wr->wr.rdma.remote_addr, args->remote_addr);
		assert_int_equal(wr->wr.rdma.rkey, args->rkey);
	}
	if (args->opcode == IBV_WR_SEND_WITH_IMM ||
	    args->opcode == IBV_WR_RDMA_WRITE_WITH_IMM)
		assert_int_equal(wr->imm_data, args->imm_data);
	assert_null(wr->next);

	return args->ret;
}

/*
 * ibv_post_recv_mock -- mock of ibv_post_recv()
 */
int
ibv_post_recv_mock(struct ibv_qp *qp, struct ibv_recv_wr *wr,
			struct ibv_recv_wr **bad_wr)
{
	struct ibv_post_recv_mock_args *args =
		mock_type(struct ibv_post_recv_mock_args *);

	assert_non_null(qp);
	assert_non_null(wr);
	assert_non_null(bad_wr);

	assert_int_equal(qp, args->qp);
	assert_int_equal(wr->wr_id, args->wr_id);
	assert_null(wr->next);

	return args->ret;
}

/*
 * ibv_post_srq_recv_mock -- mock of ibv_post_srq_recv()
 */
int
ibv_post_srq_recv_mock(struct ibv_srq *srq, struct ibv_recv_wr *wr,
		struct ibv_recv_wr **bad_wr)
{
	struct ibv_post_srq_recv_mock_args *args =
		mock_type(struct ibv_post_srq_recv_mock_args *);

	assert_non_null(srq);
	assert_non_null(wr);
	assert_non_null(bad_wr);

	assert_int_equal(srq, args->srq);
	assert_int_equal(wr->wr_id, args->wr_id);
	assert_null(wr->next);

	return args->ret;
}

/*
 * ibv_alloc_pd -- ibv_alloc_pd() mock
 */
struct ibv_pd *
ibv_alloc_pd(struct ibv_context *ibv_ctx)
{
	struct ibv_alloc_pd_mock_args *args =
			mock_type(struct ibv_alloc_pd_mock_args *);
	if (args->validate_params == MOCK_VALIDATE)
		check_expected_ptr(ibv_ctx);

	if (args->pd != NULL)
		return args->pd;

	/*
	 * The ibv_alloc_pd(3) manual page does not document that this function
	 * returns any error via errno but seemingly it is. For the usability
	 * sake, in librpma we try to deduce what really happened using
	 * the errno value.
	 */
	errno = mock_type(int);

	return NULL;
}

/*
 * ibv_dealloc_pd -- ibv_dealloc_pd() mock
 */
int
ibv_dealloc_pd(struct ibv_pd *pd)
{
	struct ibv_dealloc_pd_mock_args *args =
			mock_type(struct ibv_dealloc_pd_mock_args *);
	if (args->validate_params == MOCK_VALIDATE)
		check_expected_ptr(pd);

	return args->ret;
}

/*
 * ibv_wc_status_str -- ibv_wc_status_str() mock
 */
const char *
ibv_wc_status_str(enum ibv_wc_status status)
{
	return "";
}

#ifdef IBV_ADVISE_MR_SUPPORTED
/*
 * ibv_advise_mr_mock -- mock of ibv_advise_mr()
 */
int
ibv_advise_mr_mock(struct ibv_pd *pd,
				enum ibv_advise_mr_advice advice,
				uint32_t flags,
				struct ibv_sge *sg_list,
				uint32_t num_sge)
{
	check_expected_ptr(pd);
	check_expected(advice);
	check_expected(flags);

	assert_non_null(sg_list);
	check_expected(sg_list->lkey);
	check_expected_ptr(sg_list->addr);
	check_expected(sg_list->length);
	check_expected(num_sge);

	return mock_type(int);
}
#endif

/*
 * ibv_create_srq -- ibv_create_srq() mock
 */
struct ibv_srq *
ibv_create_srq(struct ibv_pd *pd, struct ibv_srq_init_attr *srq_init_attr)
{
	assert_ptr_equal(pd, MOCK_IBV_PD);
	assert_non_null(srq_init_attr);
	assert_null(srq_init_attr->srq_context);
	check_expected(srq_init_attr->attr.max_wr);
	assert_int_equal(srq_init_attr->attr.max_sge, 1);
	assert_int_equal(srq_init_attr->attr.srq_limit, 0);

	struct ibv_srq *srq = mock_type(struct ibv_srq *);
	if (!srq) {
		errno = mock_type(int);
		return NULL;
	}

	return srq;
}

/*
 * ibv_destroy_srq -- ibv_destroy_srq() mock
 */
int
ibv_destroy_srq(struct ibv_srq *srq)
{
	check_expected_ptr(srq);

	return mock_type(int);
}
