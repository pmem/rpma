/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn-test-next-completion.c -- the rpma_conn_next_completion() unit tests
 *
 * APIs covered:
 * - rpma_conn_next_completion()
 */

#include "conn-test-common.h"

/*
 * poll_cq -- poll_cq() mock
 */
int
poll_cq(struct ibv_cq *cq, int num_entries, struct ibv_wc *wc)
{
	check_expected_ptr(cq);
	assert_int_equal(num_entries, 1);
	assert_non_null(wc);

	int result = mock_type(int);
	if (result != 1)
		return result;

	struct ibv_wc *wc_ret = mock_type(struct ibv_wc *);
	memcpy(wc, wc_ret, sizeof(struct ibv_wc));

	return 1;
}

/*
 * test_next_completion__conn_NULL - NULL conn is invalid
 */
static void
test_next_completion__conn_NULL(void **unused)
{
	/* run test */
	struct rpma_completion cmpl = {0};
	int ret = rpma_conn_next_completion(NULL, &cmpl);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_next_completion__cmpl_NULL - NULL cmpl is invalid
 */
static void
test_next_completion__cmpl_NULL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_next_completion(cstate->conn, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_next_completion__poll_cq_fail_EAGAIN - ibv_poll_cq() fails with EAGAIN
 */
static void
test_next_completion__poll_cq_fail_EAGAIN(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mock */
	expect_value(poll_cq, cq, MOCK_CQ);
	will_return(poll_cq, -EAGAIN);

	/* run test */
	struct rpma_completion cmpl = {0};
	int ret = rpma_conn_next_completion(cstate->conn, &cmpl);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), -EAGAIN);
}

/*
 * test_next_completion__poll_cq_0 - ibv_poll_cq() returns 0 (no data)
 */
static void
test_next_completion__poll_cq_0(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mock */
	expect_value(poll_cq, cq, MOCK_CQ);
	will_return(poll_cq, 0);

	/* run test */
	struct rpma_completion cmpl = {0};
	int ret = rpma_conn_next_completion(cstate->conn, &cmpl);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NO_COMPLETION);
}

/*
 * test_next_completion__poll_cq_2 - ibv_poll_cq() returns 2 which is
 * an abnormal situation
 */
static void
test_next_completion__poll_cq_2(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mock */
	expect_value(poll_cq, cq, MOCK_CQ);
	will_return(poll_cq, 2);

	/* run test */
	struct rpma_completion cmpl = {0};
	int ret = rpma_conn_next_completion(cstate->conn, &cmpl);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_UNKNOWN);
}

/*
 * test_next_completion__poll_cq_opcode_IBV_WC_BIND_MW - ibv_poll_cq() returns
 * IBV_WC_BIND_MW (an unexptected opcode)
 */
static void
test_next_completion__poll_cq_opcode_IBV_WC_BIND_MW(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;
	struct ibv_wc wc = {0};

	/* configure mock */
	expect_value(poll_cq, cq, MOCK_CQ);
	will_return(poll_cq, 1);
	wc.opcode = IBV_WC_BIND_MW;
	will_return(poll_cq, &wc);

	/* run test */
	struct rpma_completion cmpl = {0};
	int ret = rpma_conn_next_completion(cstate->conn, &cmpl);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NOSUPP);
}

/*
 * test_next_completion__success - handling ibv_poll_cq() successfully
 */
static void
test_next_completion__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;
	struct ibv_wc wc = {0};

	/* configure mock */
	expect_value(poll_cq, cq, MOCK_CQ);
	will_return(poll_cq, 1);
	wc.opcode = IBV_WC_RDMA_READ;
	wc.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	wc.status = MOCK_WC_STATUS;
	will_return(poll_cq, &wc);

	/* run test */
	struct rpma_completion cmpl = {0};
	int ret = rpma_conn_next_completion(cstate->conn, &cmpl);

	/* verify the result */
	assert_int_equal(ret, 0);
	assert_int_equal(cmpl.op, RPMA_OP_READ);
	assert_int_equal(cmpl.op_context, MOCK_OP_CONTEXT);
	assert_int_equal(cmpl.op_status, MOCK_WC_STATUS);
}

/*
 * group_setup_next_completion -- prepare resources for all tests in the group
 */
int
group_setup_next_completion(void **unused)
{
	/* set poll_cq call back in mock of IBV CQ */
	Ibv_context.ops.poll_cq = poll_cq;
	Ibv_cq.context = &Ibv_context;

	return 0;
}

const struct CMUnitTest tests_next_completion[] = {
	/* rpma_conn_next_completion() unit tests */
	cmocka_unit_test(test_next_completion__conn_NULL),
	cmocka_unit_test_setup_teardown(
		test_next_completion__cmpl_NULL,
		conn_setup, conn_teardown),
	cmocka_unit_test_setup_teardown(
		test_next_completion__poll_cq_fail_EAGAIN,
		conn_setup, conn_teardown),
	cmocka_unit_test_setup_teardown(test_next_completion__poll_cq_0,
		conn_setup, conn_teardown),
	cmocka_unit_test_setup_teardown(test_next_completion__poll_cq_2,
		conn_setup, conn_teardown),
	cmocka_unit_test_setup_teardown(
		test_next_completion__poll_cq_opcode_IBV_WC_BIND_MW,
		conn_setup, conn_teardown),
	cmocka_unit_test_setup_teardown(
		test_next_completion__success,
		conn_setup, conn_teardown),
	cmocka_unit_test(NULL)
};
