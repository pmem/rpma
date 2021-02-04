// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * conn-completion_get.c -- the rpma_conn_completion_get() unit tests
 *
 * APIs covered:
 * - rpma_conn_completion_get()
 */

#include <string.h>
#include <arpa/inet.h>

#include "conn-common.h"
#include "mocks-ibverbs.h"
#include "test-common.h"

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
 * completion_get__conn_NULL - NULL conn is invalid
 */
static void
completion_get__conn_NULL(void **unused)
{
	/* run test */
	struct rpma_completion cmpl = {0};
	int ret = rpma_conn_completion_get(NULL, &cmpl);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * completion_get__cmpl_NULL - NULL cmpl is invalid
 */
static void
completion_get__cmpl_NULL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_completion_get(cstate->conn, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * completion_get__poll_cq_fail_EAGAIN - ibv_poll_cq() fails with EAGAIN
 */
static void
completion_get__poll_cq_fail_EAGAIN(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mock */
	expect_value(poll_cq, cq, MOCK_IBV_CQ);
	will_return(poll_cq, -EAGAIN);

	/* run test */
	struct rpma_completion cmpl = {0};
	int ret = rpma_conn_completion_get(cstate->conn, &cmpl);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * completion_get__poll_cq_0 - ibv_poll_cq() returns 0 (no data)
 */
static void
completion_get__poll_cq_0(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mock */
	expect_value(poll_cq, cq, MOCK_IBV_CQ);
	will_return(poll_cq, 0);

	/* run test */
	struct rpma_completion cmpl = {0};
	int ret = rpma_conn_completion_get(cstate->conn, &cmpl);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NO_COMPLETION);
}

/*
 * completion_get__poll_cq_2 - ibv_poll_cq() returns 2 which is
 * an abnormal situation
 */
static void
completion_get__poll_cq_2(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mock */
	expect_value(poll_cq, cq, MOCK_IBV_CQ);
	will_return(poll_cq, 2);

	/* run test */
	struct rpma_completion cmpl = {0};
	int ret = rpma_conn_completion_get(cstate->conn, &cmpl);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_UNKNOWN);
}

/*
 * completion_get__poll_cq_opcode_IBV_WC_BIND_MW - ibv_poll_cq() returns
 * IBV_WC_BIND_MW (an unexptected opcode)
 */
static void
completion_get__poll_cq_opcode_IBV_WC_BIND_MW(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;
	struct ibv_wc wc = {0};

	/* configure mock */
	expect_value(poll_cq, cq, MOCK_IBV_CQ);
	will_return(poll_cq, 1);
	wc.opcode = IBV_WC_BIND_MW;
	will_return(poll_cq, &wc);

	/* run test */
	struct rpma_completion cmpl = {0};
	int ret = rpma_conn_completion_get(cstate->conn, &cmpl);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NOSUPP);
}

/*
 * completion_get__success - handling ibv_poll_cq() successfully
 * with all possible values of opcode
 */
static void
completion_get__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	enum ibv_wc_opcode opcodes[] = {
			IBV_WC_RDMA_READ,
			IBV_WC_RDMA_WRITE,
			IBV_WC_SEND,
			IBV_WC_RECV,
			IBV_WC_RECV
	};
	enum rpma_op ops[] = {
			RPMA_OP_READ,
			RPMA_OP_WRITE,
			RPMA_OP_SEND,
			RPMA_OP_RECV,
			RPMA_OP_RECV
	};
	unsigned flags[] = {
		0,
		0,
		0,
		0,
		IBV_WC_WITH_IMM
	};

	int n_values = sizeof(opcodes) / sizeof(opcodes[0]);

	for (int i = 0; i < n_values; i++) {
		struct ibv_wc wc = {0};

		/* configure mock */
		expect_value(poll_cq, cq, MOCK_IBV_CQ);
		will_return(poll_cq, 1);
		wc.opcode = opcodes[i];
		wc.wr_id = (uint64_t)MOCK_OP_CONTEXT;
		wc.byte_len = MOCK_LEN;
		wc.status = MOCK_WC_STATUS;
		if (flags[i] == IBV_WC_WITH_IMM) {
			/*
			 * 'wc_flags' is of 'int' type
			 * in older versions of libibverbs.
			 */
			wc.wc_flags = (typeof(wc.wc_flags))flags[i];
			wc.imm_data = htonl(MOCK_IMM_DATA);
		}
		will_return(poll_cq, &wc);

		/* run test */
		struct rpma_completion cmpl = {0};
		int ret = rpma_conn_completion_get(cstate->conn, &cmpl);

		/* verify the result */
		assert_int_equal(ret, 0);
		assert_int_equal(cmpl.op, ops[i]);
		assert_int_equal(cmpl.op_context, MOCK_OP_CONTEXT);
		assert_int_equal(cmpl.byte_len, MOCK_LEN);
		assert_int_equal(cmpl.op_status, MOCK_WC_STATUS);
		if (flags[i] == IBV_WC_WITH_IMM) {
			assert_int_equal(cmpl.flags, IBV_WC_WITH_IMM);
			assert_int_equal(cmpl.imm, MOCK_IMM_DATA);
		}
	}
}

/*
 * group_setup_completion_get -- prepare resources for all tests in the group
 */
static int
group_setup_completion_get(void **unused)
{
	/* set the poll_cq callback in mock of IBV CQ */
	MOCK_VERBS->ops.poll_cq = poll_cq;
	Ibv_cq.context = MOCK_VERBS;

	return 0;
}

static const struct CMUnitTest tests_completion_get[] = {
	/* rpma_conn_completion_get() unit tests */
	cmocka_unit_test(completion_get__conn_NULL),
	cmocka_unit_test_setup_teardown(
		completion_get__cmpl_NULL,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(
		completion_get__poll_cq_fail_EAGAIN,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(completion_get__poll_cq_0,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(completion_get__poll_cq_2,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(
		completion_get__poll_cq_opcode_IBV_WC_BIND_MW,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(
		completion_get__success,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_completion_get,
			group_setup_completion_get, NULL);
}
