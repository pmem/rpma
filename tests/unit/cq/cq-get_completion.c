// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Fujitsu */

/*
 * cq-get.c -- the rpma_cq_get_completion() unit tests
 *
 * API covered:
 * - rpma_cq_get_completion()
 */

#include <string.h>
#include <arpa/inet.h>

#include "cmocka_headers.h"
#include "mocks-ibverbs.h"
#include "cq-common.h"

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
 * get_completion__cq_NULL - cq NULL is invalid
 */
static void
get_completion__cq_NULL(void **unused)
{
	/* run test */
	struct rpma_completion cmpl = {0};
	int ret = rpma_cq_get_completion(NULL, &cmpl);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_completion__cmpl_NULL - cmpl NULL is invalid
 */
static void
get_completion__cmpl_NULL(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;

	/* run test */
	int ret = rpma_cq_get_completion(cq, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_completion__poll_cq_fail - ibv_poll_cq() returns -1
 */
static void
get_completion__poll_cq_fail(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;

	/* configure mock */
	expect_value(poll_cq, cq, MOCK_IBV_CQ);
	will_return(poll_cq, -1);

	/* run test */
	struct rpma_completion cmpl = {0};
	int ret = rpma_cq_get_completion(cq, &cmpl);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * get_completion__poll_cq_0 - ibv_poll_cq() returns 0 (no data)
 */
static void
get_completion__poll_cq_0(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;

	/* configure mock */
	expect_value(poll_cq, cq, MOCK_IBV_CQ);
	will_return(poll_cq, 0);

	/* run test */
	struct rpma_completion cmpl = {0};
	int ret = rpma_cq_get_completion(cq, &cmpl);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NO_COMPLETION);
}

/*
 * get_completion__poll_cq_2 - ibv_poll_cq() returns 2 which is
 * an abnormal situation
 */
static void
get_completion__poll_cq_2(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;

	/* configure mock */
	expect_value(poll_cq, cq, MOCK_IBV_CQ);
	will_return(poll_cq, 2);

	/* run test */
	struct rpma_completion cmpl = {0};
	int ret = rpma_cq_get_completion(cq, &cmpl);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_UNKNOWN);
}

/*
 * get_completion__poll_cq_wc_status_error - ibv_poll_cq() returns
 * an error wc.status (wc.status != IBV_WC_SUCCESS)
 */
static void
get_completion__poll_cq_wc_status_error(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;
	struct ibv_wc wc = {0};

	/* configure mock */
	expect_value(poll_cq, cq, MOCK_IBV_CQ);
	will_return(poll_cq, 1);
	wc.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	assert_int_not_equal(MOCK_WC_STATUS_ERROR, IBV_WC_SUCCESS);
	wc.status = MOCK_WC_STATUS_ERROR;
	will_return(poll_cq, &wc);

	/* run test */
	struct rpma_completion cmpl = {NULL, RPMA_OP_WRITE, 0xba,
			IBV_WC_SUCCESS, 0xba, 0xba};
	int ret = rpma_cq_get_completion(cq, &cmpl);

	/* verify the result */
	assert_int_equal(ret, 0);
	assert_int_equal(cmpl.op_context, MOCK_OP_CONTEXT);
	assert_int_equal(cmpl.op_status, MOCK_WC_STATUS_ERROR);
	/* the rest of the fields should not be touched at all */
	assert_int_equal(cmpl.op, RPMA_OP_WRITE);
	assert_int_equal(cmpl.byte_len, 0xba);
	assert_int_equal(cmpl.flags, 0xba);
	assert_int_equal(cmpl.imm, 0xba);
}

/*
 * get_completion__poll_cq_opcode_IBV_WC_BIND_MW - ibv_poll_cq() returns
 * IBV_WC_BIND_MW (an unexpected opcode)
 */
static void
get_completion__poll_cq_opcode_IBV_WC_BIND_MW(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;
	struct ibv_wc wc = {0};

	/* configure mock */
	expect_value(poll_cq, cq, MOCK_IBV_CQ);
	will_return(poll_cq, 1);
	wc.wr_id = (uint64_t)MOCK_OP_CONTEXT;
	wc.status = IBV_WC_SUCCESS;
	wc.opcode = IBV_WC_BIND_MW;
	will_return(poll_cq, &wc);

	/* run test */
	struct rpma_completion cmpl = {0};
	int ret = rpma_cq_get_completion(cq, &cmpl);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NOSUPP);
	assert_int_equal(cmpl.op_context, MOCK_OP_CONTEXT);
	assert_int_equal(cmpl.op_status, IBV_WC_SUCCESS);
}

/*
 * get_completion__success - handling ibv_poll_cq() successfully
 * with all possible values of opcode
 */
static void
get_completion__success(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;

	enum ibv_wc_opcode opcodes[] = {
			IBV_WC_RDMA_READ,
			IBV_WC_RDMA_WRITE,
			IBV_WC_SEND,
			IBV_WC_RECV,
			IBV_WC_RECV,
			IBV_WC_RECV_RDMA_WITH_IMM
	};
	enum rpma_op ops[] = {
			RPMA_OP_READ,
			RPMA_OP_WRITE,
			RPMA_OP_SEND,
			RPMA_OP_RECV,
			RPMA_OP_RECV,
			RPMA_OP_RECV_RDMA_WITH_IMM
	};
	unsigned flags[] = {
		0,
		0,
		0,
		0,
		IBV_WC_WITH_IMM,
		IBV_WC_WITH_IMM
	};

	int n_values = sizeof(opcodes) / sizeof(opcodes[0]);

	for (int i = 0; i < n_values; i++) {
		struct ibv_wc wc = {0};

		/* configure mock */
		expect_value(poll_cq, cq, MOCK_IBV_CQ);
		will_return(poll_cq, 1);
		wc.wr_id = (uint64_t)MOCK_OP_CONTEXT;
		wc.status = IBV_WC_SUCCESS;
		wc.opcode = opcodes[i];
		wc.byte_len = MOCK_LEN;
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
		int ret = rpma_cq_get_completion(cq, &cmpl);

		/* verify the result */
		assert_int_equal(ret, 0);
		assert_int_equal(cmpl.op_context, MOCK_OP_CONTEXT);
		assert_int_equal(cmpl.op_status, IBV_WC_SUCCESS);
		assert_int_equal(cmpl.op, ops[i]);
		assert_int_equal(cmpl.byte_len, MOCK_LEN);
		if (flags[i] == IBV_WC_WITH_IMM) {
			assert_int_equal(cmpl.flags, IBV_WC_WITH_IMM);
			assert_int_equal(cmpl.imm, MOCK_IMM_DATA);
		}
	}
}

/*
 * group_setup_get -- prepare resources for all tests in the group
 */
static int
group_setup_get_completion(void **unused)
{
	/* set the poll_cq callback in mock of IBV CQ */
	MOCK_VERBS->ops.poll_cq = poll_cq;

	return group_setup_common_cq(NULL);
}

static const struct CMUnitTest tests_get_completion[] = {
	/* rpma_cq_get_completion() unit tests */
	cmocka_unit_test(get_completion__cq_NULL),
	cmocka_unit_test_setup_teardown(
		get_completion__cmpl_NULL,
		setup__cq_new, teardown__cq_delete),
	cmocka_unit_test_setup_teardown(
		get_completion__poll_cq_fail,
		setup__cq_new, teardown__cq_delete),
	cmocka_unit_test_setup_teardown(get_completion__poll_cq_0,
		setup__cq_new, teardown__cq_delete),
	cmocka_unit_test_setup_teardown(get_completion__poll_cq_2,
		setup__cq_new, teardown__cq_delete),
	cmocka_unit_test_setup_teardown(
		get_completion__poll_cq_wc_status_error,
		setup__cq_new, teardown__cq_delete),
	cmocka_unit_test_setup_teardown(
		get_completion__poll_cq_opcode_IBV_WC_BIND_MW,
		setup__cq_new, teardown__cq_delete),
	cmocka_unit_test_setup_teardown(
		get_completion__success,
		setup__cq_new, teardown__cq_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_get_completion,
			group_setup_get_completion, NULL);
}
