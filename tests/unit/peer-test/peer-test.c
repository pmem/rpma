/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * peer-test.c -- a peer unit test
 */

#include <infiniband/verbs.h>

#include "cmocka_headers.h"
#include "conn_req.h"
#include "mocks-ibverbs.h"
#include "peer.h"
#include "test-common.h"

#define MOCK_CM_ID		(struct rdma_cm_id *)0xC41D
#define MOCK_ADDR		(void *)0x2B6A
#define MOCK_LEN		(size_t)627
#define MOCK_ACCESS		(unsigned)823

/*
 * rdma_create_qp -- rdma_create_qp() mock
 */
int
rdma_create_qp(struct rdma_cm_id *id, struct ibv_pd *pd,
		struct ibv_qp_init_attr *qp_init_attr)
{
	check_expected_ptr(id);
	check_expected_ptr(pd);
	assert_non_null(qp_init_attr);
	check_expected(qp_init_attr->qp_context);
	check_expected(qp_init_attr->send_cq);
	check_expected(qp_init_attr->recv_cq);
	assert_null(qp_init_attr->srq);
	check_expected(qp_init_attr->cap.max_send_wr);
	check_expected(qp_init_attr->cap.max_recv_wr);
	check_expected(qp_init_attr->cap.max_send_sge);
	check_expected(qp_init_attr->cap.max_recv_sge);
	check_expected(qp_init_attr->cap.max_inline_data);
	assert_int_equal(qp_init_attr->qp_type, IBV_QPT_RC);
	assert_int_equal(qp_init_attr->sq_sig_all, 0);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

void *__real__test_malloc(size_t size);

/*
 * __wrap__test_malloc -- malloc() mock
 */
void *
__wrap__test_malloc(size_t size)
{
	int err = mock_type(int);

	if (err) {
		errno = err;
		return NULL;
	}

	return __real__test_malloc(size);
}

/*
 * peer_new_test_ibv_ctx_eq_NULL -- NULL ibv_ctx is not valid
 */
static void
peer_new_test_ibv_ctx_eq_NULL(void **unused)
{
	/*
	 * NOTE: it is not allowed for peer to allocate any resource before
	 * validating arguments.
	 */

	/* run test */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_new(NULL, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(peer);
}

/*
 * peer_new_test_peer_ptr_eq_NULL -- NULL **peer is not valid
 */
static void
peer_new_test_peer_ptr_eq_NULL(void **unused)
{
	/*
	 * NOTE: it is not allowed for peer to allocate any resource before
	 * validating arguments.
	 */

	/* run test */
	int ret = rpma_peer_new(MOCK_VERBS, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * peer_new_test_ibv_ctx_and_peer_ptr_eq_NULL -- NULL ibv_ctx and NULL **peer
 * are not valid
 */
static void
peer_new_test_ibv_ctx_and_peer_ptr_eq_NULL(void **unused)
{
	/*
	 * NOTE: it is not allowed for peer to allocate any resource before
	 * validating arguments.
	 */

	/* run test */
	int ret = rpma_peer_new(NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * peer_new_test_alloc_pd_fail_ENOMEM -- ibv_alloc_pd() fails with ENOMEM
 */
static void
peer_new_test_alloc_pd_fail_ENOMEM(void **unused)
{
	/*
	 * configure mocks:
	 * - NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd()
	 * has failed.
	 */
	struct ibv_alloc_pd_mock_args alloc_args = {MOCK_VALIDATE, NULL};
	will_return(ibv_alloc_pd, &alloc_args);
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_VERBS);
	will_return(ibv_alloc_pd, ENOMEM);
	will_return_maybe(rpma_utils_ibv_context_is_odp_capable, 1);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_new(MOCK_VERBS, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(peer);
}

/*
 * peer_new_test_alloc_pd_fail_EAGAIN -- ibv_alloc_pd() fails with EAGAIN
 * (arbitrarily picked error which is not ENOMEM)
 */
static void
peer_new_test_alloc_pd_fail_EAGAIN(void **unused)
{
	/*
	 * configure mocks:
	 * - NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd()
	 * has failed.
	 */
	struct ibv_alloc_pd_mock_args alloc_args = {MOCK_VALIDATE, NULL};
	will_return(ibv_alloc_pd, &alloc_args);
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_VERBS);
	will_return(ibv_alloc_pd, EAGAIN);
	will_return_maybe(rpma_utils_ibv_context_is_odp_capable, 1);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_new(MOCK_VERBS, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(peer);
}

/*
 * peer_new_test_alloc_pd_fail_no_error -- ibv_alloc_pd() fails without error
 */
static void
peer_new_test_alloc_pd_fail_no_error(void **unused)
{
	/*
	 * configure mocks:
	 * - NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd()
	 * has failed.
	 */
	struct ibv_alloc_pd_mock_args alloc_args = {MOCK_VALIDATE, NULL};
	will_return(ibv_alloc_pd, &alloc_args);
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_VERBS);
	will_return(ibv_alloc_pd, MOCK_OK);
	will_return_maybe(rpma_utils_ibv_context_is_odp_capable, 1);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_new(MOCK_VERBS, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_UNKNOWN);
	assert_null(peer);
}

/*
 * peer_new_test_malloc_fail -- malloc() fail
 */
static void
peer_new_test_malloc_fail(void **unused)
{
	/* configure mocks */
	will_return(__wrap__test_malloc, ENOMEM);
	struct ibv_alloc_pd_mock_args alloc_args =
		{MOCK_PASSTHROUGH, MOCK_IBV_PD};
	will_return_maybe(ibv_alloc_pd, &alloc_args);
	struct ibv_dealloc_pd_mock_args dealloc_args =
		{MOCK_PASSTHROUGH, MOCK_OK};
	will_return_maybe(ibv_dealloc_pd, &dealloc_args);
	will_return_maybe(rpma_utils_ibv_context_is_odp_capable, 1);

	/* run test */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_new(MOCK_VERBS, &peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(peer);
}

/*
 * peer_new_test_success -- happy day scenario
 */
static void
peer_new_test_success(void **unused)
{
	/*
	 * configure mocks for rpma_peer_new():
	 * NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd()
	 * succeeded.
	 */
	will_return(rpma_utils_ibv_context_is_odp_capable, 1);
	struct ibv_alloc_pd_mock_args alloc_args = {MOCK_VALIDATE, MOCK_IBV_PD};
	will_return(ibv_alloc_pd, &alloc_args);
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_VERBS);
	will_return(__wrap__test_malloc, MOCK_OK);

	/* run test - step 1 */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_new(MOCK_VERBS, &peer);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(peer);

	/*
	 * configure mocks for rpma_peer_delete():
	 * NOTE: it is not allowed to call ibv_alloc_pd() nor malloc() in
	 * rpma_peer_delete().
	 */
	struct ibv_dealloc_pd_mock_args dealloc_args =
		{MOCK_VALIDATE, MOCK_OK};
	will_return(ibv_dealloc_pd, &dealloc_args);
	expect_value(ibv_dealloc_pd, pd, MOCK_IBV_PD);

	/* run test - step 2 */
	ret = rpma_peer_delete(&peer);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(peer);
}

/*
 * peer_delete_test_invalid_peer_ptr -- NULL **peer is not valid
 */
static void
peer_delete_test_invalid_peer_ptr(void **unused)
{
	/*
	 * NOTE: it is not allowed for peer to allocate any resource before
	 * validating arguments.
	 */

	/* run test */
	int ret = rpma_peer_delete(NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * peer_delete_test_null_peer -- valid NULL *peer
 */
static void
peer_delete_test_null_peer(void **unused)
{
	/*
	 * NOTE: it is not allowed for peer to allocate any resource when
	 * quick-exiting.
	 */

	/* run test */
	struct rpma_peer *peer = NULL;
	int ret = rpma_peer_delete(&peer);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_null(peer);
}

/*
 * peer_setup -- prepare a valid rpma_peer object
 * (encapsulating the MOCK_IBV_PD)
 */
static int
peer_setup(void **peer_ptr)
{
	/*
	 * configure mocks for rpma_peer_new():
	 * NOTE: it is not allowed to call ibv_dealloc_pd() if ibv_alloc_pd()
	 * succeeded.
	 */
	will_return(rpma_utils_ibv_context_is_odp_capable, 1);
	struct ibv_alloc_pd_mock_args alloc_args = {MOCK_VALIDATE, MOCK_IBV_PD};
	will_return(ibv_alloc_pd, &alloc_args);
	expect_value(ibv_alloc_pd, ibv_ctx, MOCK_VERBS);
	will_return(__wrap__test_malloc, MOCK_OK);

	/* setup */
	int ret = rpma_peer_new(MOCK_VERBS, (struct rpma_peer **)peer_ptr);
	assert_int_equal(ret, 0);
	assert_non_null(*peer_ptr);

	return 0;
}

/*
 * peer_delete_test_teardown -- delete the rpma_peer object
 */
static int
peer_teardown(void **peer_ptr)
{
	/*
	 * configure mocks for rpma_peer_delete():
	 * NOTE: it is not allowed to call ibv_alloc_pd() nor malloc() in
	 * rpma_peer_delete().
	 */
	struct ibv_dealloc_pd_mock_args dealloc_args =
		{MOCK_VALIDATE, MOCK_OK};
	will_return(ibv_dealloc_pd, &dealloc_args);
	expect_value(ibv_dealloc_pd, pd, MOCK_IBV_PD);

	/* teardown */
	int ret = rpma_peer_delete((struct rpma_peer **)peer_ptr);
	assert_int_equal(ret, MOCK_OK);
	assert_null(*peer_ptr);

	return 0;
}

/*
 * peer_delete_test_dealloc_pd_fail -- ibv_dealloc_pd() fail
 */
static void
peer_delete_test_dealloc_pd_fail(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;

	/*
	 * configure mocks for rpma_peer_delete():
	 * NOTE: it is not allowed to call ibv_alloc_pd() nor malloc() in
	 * rpma_peer_delete().
	 */
	struct ibv_dealloc_pd_mock_args dealloc_args = {MOCK_VALIDATE, EBUSY};
	will_return(ibv_dealloc_pd, &dealloc_args);
	expect_value(ibv_dealloc_pd, pd, MOCK_IBV_PD);

	/* run test */
	int ret = rpma_peer_delete(&peer);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EBUSY);
}

/*
 * create_qp_test_peer_NULL -- NULL peer is invalid
 */
static void
create_qp_test_peer_NULL(void **unused)
{
	/* run test */
	struct rdma_cm_id *id = MOCK_CM_ID;
	struct ibv_cq *cq = MOCK_IBV_CQ;
	int ret = rpma_peer_create_qp(NULL, id, cq);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * create_qp_test_id_NULL -- NULL id is invalid
 */
static void
create_qp_test_id_NULL(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;

	/* run test */
	struct ibv_cq *cq = MOCK_IBV_CQ;
	int ret = rpma_peer_create_qp(peer, NULL, cq);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * create_qp_test_cq_NULL -- NULL cq is invalid
 */
static void
create_qp_test_cq_NULL(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;

	/* run test */
	struct rdma_cm_id *id = MOCK_CM_ID;
	int ret = rpma_peer_create_qp(peer, id, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * create_qp_test_rdma_create_qp_EAGAIN -- rdma_create_qp() fails with EAGAIN
 */
static void
create_qp_test_rdma_create_qp_EAGAIN(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;

	/* configure mock: */
	expect_value(rdma_create_qp, id, MOCK_CM_ID);
	expect_value(rdma_create_qp, pd, MOCK_IBV_PD);
	expect_value(rdma_create_qp, qp_init_attr->qp_context, NULL);
	expect_value(rdma_create_qp, qp_init_attr->send_cq, MOCK_IBV_CQ);
	expect_value(rdma_create_qp, qp_init_attr->recv_cq, MOCK_IBV_CQ);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_send_wr,
		RPMA_DEFAULT_Q_SIZE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_recv_wr,
		RPMA_DEFAULT_Q_SIZE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_send_sge,
		RPMA_MAX_SGE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_recv_sge,
		RPMA_MAX_SGE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_inline_data,
		RPMA_MAX_INLINE_DATA);
	will_return(rdma_create_qp, EAGAIN);

	/* run test */
	struct rdma_cm_id *id = MOCK_CM_ID;
	struct ibv_cq *cq = MOCK_IBV_CQ;
	int ret = rpma_peer_create_qp(peer, id, cq);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
}

/*
 * create_qp_test_success -- happy day scenario
 */
static void
create_qp_test_success(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;

	/* configure mock: */
	expect_value(rdma_create_qp, id, MOCK_CM_ID);
	expect_value(rdma_create_qp, pd, MOCK_IBV_PD);
	expect_value(rdma_create_qp, qp_init_attr->qp_context, NULL);
	expect_value(rdma_create_qp, qp_init_attr->send_cq, MOCK_IBV_CQ);
	expect_value(rdma_create_qp, qp_init_attr->recv_cq, MOCK_IBV_CQ);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_send_wr,
		RPMA_DEFAULT_Q_SIZE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_recv_wr,
		RPMA_DEFAULT_Q_SIZE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_send_sge,
		RPMA_MAX_SGE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_recv_sge,
		RPMA_MAX_SGE);
	expect_value(rdma_create_qp, qp_init_attr->cap.max_inline_data,
		RPMA_MAX_INLINE_DATA);
	will_return(rdma_create_qp, 0);

	/* run test */
	struct rdma_cm_id *id = MOCK_CM_ID;
	struct ibv_cq *cq = MOCK_IBV_CQ;
	int ret = rpma_peer_create_qp(peer, id, cq);

	/* verify the results */
	assert_int_equal(ret, 0);
}

/*
 * mr_reg_test_fail_ENOMEM -- ibv_reg_mr() failed with ENOMEM
 */
static void
mr_reg_test_fail_ENOMEM(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;

	/* configure mocks */
	expect_value(ibv_reg_mr, pd, MOCK_IBV_PD);
	expect_value(ibv_reg_mr, addr, MOCK_ADDR);
	expect_value(ibv_reg_mr, length, MOCK_LEN);
	expect_value(ibv_reg_mr, access, MOCK_ACCESS);
	will_return(ibv_reg_mr, NULL);
	will_return(ibv_reg_mr, ENOMEM);

	/* run test */
	struct ibv_mr *mr = NULL;
	int ret = rpma_peer_mr_reg(peer, &mr, MOCK_ADDR,
				MOCK_LEN, MOCK_ACCESS);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), ENOMEM);
	assert_null(mr);
}

/*
 * mr_reg_test_success -- happy day scenario
 */
static void
mr_reg_test_success(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;

	/* configure mocks */
	expect_value(ibv_reg_mr, pd, MOCK_IBV_PD);
	expect_value(ibv_reg_mr, addr, MOCK_ADDR);
	expect_value(ibv_reg_mr, length, MOCK_LEN);
	expect_value(ibv_reg_mr, access, MOCK_ACCESS);
	will_return(ibv_reg_mr, MOCK_MR);

	/* run test */
	struct ibv_mr *mr;
	int ret = rpma_peer_mr_reg(peer, &mr, MOCK_ADDR,
				MOCK_LEN, MOCK_ACCESS);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(mr, MOCK_MR);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_peer_new() unit tests */
		cmocka_unit_test(peer_new_test_ibv_ctx_eq_NULL),
		cmocka_unit_test(peer_new_test_peer_ptr_eq_NULL),
		cmocka_unit_test(peer_new_test_ibv_ctx_and_peer_ptr_eq_NULL),
		cmocka_unit_test(peer_new_test_alloc_pd_fail_ENOMEM),
		cmocka_unit_test(peer_new_test_alloc_pd_fail_EAGAIN),
		cmocka_unit_test(peer_new_test_alloc_pd_fail_no_error),
		cmocka_unit_test(peer_new_test_malloc_fail),
		cmocka_unit_test(peer_new_test_success),

		/* rpma_peer_delete() unit tests */
		cmocka_unit_test(peer_delete_test_invalid_peer_ptr),
		cmocka_unit_test(peer_delete_test_null_peer),
		cmocka_unit_test_setup_teardown(
				peer_delete_test_dealloc_pd_fail,
				peer_setup, peer_teardown),

		/* rpma_peer_create_qp() unit tests */
		cmocka_unit_test(create_qp_test_peer_NULL),
		cmocka_unit_test_setup_teardown(create_qp_test_id_NULL,
				peer_setup, peer_teardown),
		cmocka_unit_test_setup_teardown(create_qp_test_cq_NULL,
				peer_setup, peer_teardown),
		cmocka_unit_test_setup_teardown(
				create_qp_test_rdma_create_qp_EAGAIN,
				peer_setup, peer_teardown),
		cmocka_unit_test_setup_teardown(create_qp_test_success,
				peer_setup, peer_teardown),

		/* rpma_peer_mr_reg() unit tests */
		cmocka_unit_test_setup_teardown(
				mr_reg_test_fail_ENOMEM,
				peer_setup, peer_teardown),
		cmocka_unit_test_setup_teardown(
				mr_reg_test_success,
				peer_setup, peer_teardown),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
