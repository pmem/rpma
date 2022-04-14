// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */
/* Copyright 2022, Fujitsu */

/*
 * cq-get_wc.c -- the rpma_cq_get_wc() unit tests
 *
 * API covered:
 * - rpma_cq_get_wc()
 */

#include <string.h>
#include <arpa/inet.h>

#include "cmocka_headers.h"
#include "mocks-ibverbs.h"
#include "cq-common.h"

static enum ibv_wc_opcode opcodes[] = {
	IBV_WC_RDMA_READ,
	IBV_WC_RDMA_WRITE,
	IBV_WC_SEND,
	IBV_WC_RECV,
	IBV_WC_RECV,
	IBV_WC_RECV_RDMA_WITH_IMM
};

static unsigned flags[] = {
	0,
	0,
	0,
	0,
	IBV_WC_WITH_IMM,
	IBV_WC_WITH_IMM
};

static int All_values = sizeof(opcodes) / sizeof(opcodes[0]);

/*
 * poll_cq -- mock of ibv_poll_cq()
 */
int
poll_cq(struct ibv_cq *cq, int num_entries, struct ibv_wc *wc)
{
	check_expected_ptr(cq);
	check_expected(num_entries);
	assert_non_null(wc);

	int result = mock_type(int);
	if (result < 1 || result > num_entries)
		return result;

	struct ibv_wc *wc_ret = mock_type(struct ibv_wc *);
	memcpy(wc, wc_ret, sizeof(struct ibv_wc) * (size_t)result);

	return result;
}

/*
 * get_wc__cq_NULL - cq NULL is invalid
 */
static void
get_wc__cq_NULL(void **unused)
{
	/* run test */
	struct ibv_wc wc = {0};
	int ret = rpma_cq_get_wc(NULL, 1, &wc, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_wc__num_entries_non_positive - num_entries < 1 is invalid
 */
static void
get_wc__num_entries_non_positive(void **cq_ptr)
{
	struct cq_test_state *cstate = *cq_ptr;
	struct rpma_cq *cq = cstate->cq;

	/* run test */
	struct ibv_wc wc = {0};
	int ret = rpma_cq_get_wc(cq, -1, &wc, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_wc__wc_NULL - wc NULL is invalid
 */
static void
get_wc__wc_NULL(void **cq_ptr)
{
	struct cq_test_state *cstate = *cq_ptr;
	struct rpma_cq *cq = cstate->cq;

	/* run test */
	int ret = rpma_cq_get_wc(cq, 1, NULL, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_wc__num_entries_2_num_entries_got_NULL - num_entries > 1
 * and num_entries_got NULL are invalid
 */
static void
get_wc__num_entries_2_num_entries_got_NULL(void **cq_ptr)
{
	struct cq_test_state *cstate = *cq_ptr;
	struct rpma_cq *cq = cstate->cq;

	/* run test */
	struct ibv_wc wc[2];
	memset(wc, 0, sizeof(wc));
	int ret = rpma_cq_get_wc(cq, 2, wc, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_wc__poll_cq_fail - ibv_poll_cq() returns -1
 */
static void
get_wc__poll_cq_fail(void **cq_ptr)
{
	struct cq_test_state *cstate = *cq_ptr;
	struct rpma_cq *cq = cstate->cq;

	/* configure mock */
	expect_value(poll_cq, cq, MOCK_IBV_CQ);
	expect_value(poll_cq, num_entries, 1);
	will_return(poll_cq, -1);

	/* run test */
	struct ibv_wc wc = {0};
	int ret = rpma_cq_get_wc(cq, 1, &wc, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * get_wc__poll_cq_no_data - ibv_poll_cq() returns 0 (no data)
 */
static void
get_wc__poll_cq_no_data(void **cq_ptr)
{
	struct cq_test_state *cstate = *cq_ptr;
	struct rpma_cq *cq = cstate->cq;

	/* configure mock */
	expect_value(poll_cq, cq, MOCK_IBV_CQ);
	expect_value(poll_cq, num_entries, 1);
	will_return(poll_cq, 0);

	/* run test */
	struct ibv_wc wc = {0};
	int ret = rpma_cq_get_wc(cq, 1, &wc, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NO_COMPLETION);
}

/*
 * get_wc__poll_cq_more_data - ibv_poll_cq() returns 3 (more than
 * num_entries) which is an abnormal situation
 */
static void
get_wc__poll_cq_more_data(void **cq_ptr)
{
	struct cq_test_state *cstate = *cq_ptr;
	struct rpma_cq *cq = cstate->cq;

	/* configure mock */
	expect_value(poll_cq, cq, MOCK_IBV_CQ);
	expect_value(poll_cq, num_entries, 2);
	will_return(poll_cq, 3);

	/* run test */
	struct ibv_wc wc[2];
	memset(wc, 0, sizeof(wc));
	int num_entries_got = 0;
	int ret = rpma_cq_get_wc(cq, 2, wc, &num_entries_got);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_UNKNOWN);
	assert_int_equal(num_entries_got, 0);
}

/*
 * get_wc__success_each_opcode - handle ibv_poll_cq() successfully
 * with each possible value of opcode
 */
static void
get_wc__success_each_opcode(void **cq_ptr)
{
	struct cq_test_state *cstate = *cq_ptr;
	struct rpma_cq *cq = cstate->cq;

	for (int i = 0; i < All_values; i++) {
		struct ibv_wc orig_wc = {0};

		/* configure mock */
		expect_value(poll_cq, cq, MOCK_IBV_CQ);
		expect_value(poll_cq, num_entries, 1);
		will_return(poll_cq, 1);
		orig_wc.wr_id = (uint64_t)MOCK_OP_CONTEXT;
		orig_wc.status = IBV_WC_SUCCESS;
		orig_wc.opcode = opcodes[i];
		orig_wc.byte_len = MOCK_LEN;
		if (flags[i] == IBV_WC_WITH_IMM) {
			/*
			 * 'wc_flags' is of 'int' type
			 * in older versions of libibverbs.
			 */
			orig_wc.wc_flags = (typeof(orig_wc.wc_flags))flags[i];
			orig_wc.imm_data = htonl(MOCK_IMM_DATA);
		}
		will_return(poll_cq, &orig_wc);

		/* run test */
		struct ibv_wc wc = {0};
		int ret = 0, num_entries_got = 0;
		/* alternate num_entries_got and NULL */
		int use_num_entries_got = i % 2;
		if (use_num_entries_got)
			ret = rpma_cq_get_wc(cq, 1, &wc, &num_entries_got);
		else
			ret = rpma_cq_get_wc(cq, 1, &wc, NULL);

		/* verify the result */
		assert_int_equal(ret, 0);
		assert_int_equal((memcmp(&orig_wc, &wc, sizeof(wc))), 0);
		if (use_num_entries_got)
			assert_int_equal(num_entries_got, 1);
	}
}

/*
 * get_wc__success_all_opcodes - handle ibv_poll_cq() successfully
 * with all possible values of opcode
 */
static void
get_wc__success_all_opcodes(void **cq_ptr)
{
	struct cq_test_state *cstate = *cq_ptr;
	struct rpma_cq *cq = cstate->cq;
	int n_values[3] = {2, 3, All_values};

	for (int i = 0; i < 3; i++) {
		struct ibv_wc orig_wc[n_values[i]];

		/* configure mock */
		expect_value(poll_cq, cq, MOCK_IBV_CQ);
		expect_value(poll_cq, num_entries, n_values[i]);
		will_return(poll_cq, n_values[i]);
		for (int j = 0; j < n_values[i]; j++) {
			orig_wc[j].wr_id = (uint64_t)MOCK_OP_CONTEXT;
			orig_wc[j].status = IBV_WC_SUCCESS;
			orig_wc[j].opcode = opcodes[j];
			orig_wc[j].byte_len = MOCK_LEN;
			if (flags[j] == IBV_WC_WITH_IMM) {
				/*
				 * 'wc_flags' is of 'int' type
				 * in older versions of libibverbs.
				 */
				orig_wc[j].wc_flags =
					(typeof(orig_wc[j].wc_flags))flags[j];
				orig_wc[j].imm_data = htonl(MOCK_IMM_DATA);
			}
		}
		will_return(poll_cq, orig_wc);

		/* run test */
		struct ibv_wc wc[n_values[i]];
		memset(wc, 0, sizeof(wc));
		int num_entries_got = 0;
		int ret = rpma_cq_get_wc(cq, n_values[i], wc, &num_entries_got);

		/* verify the result */
		assert_int_equal(ret, 0);
		assert_int_equal((memcmp(orig_wc, wc, sizeof(wc))), 0);
		assert_int_equal(num_entries_got, n_values[i]);
	}
}

/*
 * group_setup_get -- prepare resources for all tests in the group
 */
static int
group_setup_get_wc(void **unused)
{
	/* set the poll_cq callback in mock of IBV CQ */
	MOCK_VERBS->ops.poll_cq = poll_cq;

	return group_setup_common_cq(NULL);
}

static const struct CMUnitTest tests_get_wc[] = {
	/* rpma_cq_get_wc() unit tests */
	cmocka_unit_test(get_wc__cq_NULL),
	cmocka_unit_test_setup_teardown(get_wc__num_entries_non_positive,
		setup__cq_new, teardown__cq_delete),
	cmocka_unit_test_setup_teardown(get_wc__wc_NULL,
		setup__cq_new, teardown__cq_delete),
	cmocka_unit_test_setup_teardown(
		get_wc__num_entries_2_num_entries_got_NULL,
		setup__cq_new, teardown__cq_delete),
	cmocka_unit_test_setup_teardown(get_wc__poll_cq_fail,
		setup__cq_new, teardown__cq_delete),
	cmocka_unit_test_setup_teardown(get_wc__poll_cq_no_data,
		setup__cq_new, teardown__cq_delete),
	cmocka_unit_test_setup_teardown(get_wc__poll_cq_more_data,
		setup__cq_new, teardown__cq_delete),
	cmocka_unit_test_setup_teardown(get_wc__success_each_opcode,
		setup__cq_new, teardown__cq_delete),
	cmocka_unit_test_setup_teardown(get_wc__success_all_opcodes,
		setup__cq_new, teardown__cq_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_get_wc,
			group_setup_get_wc, NULL);
}
