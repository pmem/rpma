// SPDX-License-Identifier: BSD-3-Clause
/* Copyright (c) 2020 Fujitsu */
/* Copyright 2021, Intel Corporation */

/*
 * conn-send_with_imm.c -- the rpma_send_with_imm() unit tests
 *
 * APIs covered:
 * - rpma_send_with_imm()
 */

#include "conn-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"

/*
 * send_with_imm__src_NULL_offset_not_NULL -- NULL src
 * and not NULL offset is invalid
 */
static void
send_with_imm__src_NULL_offset_not_NULL(void **unused)
{
	/* run test */
	int ret = rpma_send_with_imm(MOCK_CONN, NULL, MOCK_LOCAL_OFFSET, 0,
			MOCK_FLAGS, MOCK_IMM_DATA, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * send_with_imm__src_NULL_len_not_NULL -- NULL src
 * and not NULL len is invalid
 */
static void
send_with_imm__src_NULL_len_not_NULL(void **unused)
{
	/* run test */
	int ret = rpma_send_with_imm(MOCK_CONN, NULL, 0, MOCK_LEN,
			MOCK_FLAGS, MOCK_IMM_DATA, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * send_with_imm__src_NULL_offset_len_not_NULL -- NULL src
 * and not NULL offset or len are invalid
 */
static void
send_with_imm__src_NULL_offset_len_not_NULL(void **unused)
{
	/* run test */
	int ret = rpma_send_with_imm(MOCK_CONN, NULL,
			MOCK_LOCAL_OFFSET, MOCK_LEN, MOCK_FLAGS,
			MOCK_IMM_DATA, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * send_with_imm__conn_NULL -- NULL conn is invalid
 */
static void
send_with_imm__conn_NULL(void **unused)
{
	/* run test */
	int ret = rpma_send_with_imm(NULL, MOCK_RPMA_MR_LOCAL,
			MOCK_LOCAL_OFFSET, MOCK_LEN, MOCK_FLAGS,
			MOCK_IMM_DATA, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * send_with_imm__flags_0 -- flags == 0 is invalid
 */
static void
send_with_imm__flags_0(void **unused)
{
	/* run test */
	int ret = rpma_send_with_imm(MOCK_CONN, MOCK_RPMA_MR_LOCAL,
			MOCK_LOCAL_OFFSET, MOCK_LEN, 0, MOCK_IMM_DATA,
			MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * send_with_imm__conn_NULL_flags_0 -- NULL conn, src
 * and flags == 0 are invalid
 */
static void
send_with_imm__conn_NULL_flags_0(void **unused)
{
	/* run test */
	int ret = rpma_send_with_imm(NULL, NULL, MOCK_LOCAL_OFFSET,
			MOCK_LEN, 0, MOCK_IMM_DATA, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * send_with_imm__success -- happy day scenario
 */
static void
send_with_imm__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	expect_value(rpma_mr_send, qp, MOCK_QP);
	expect_value(rpma_mr_send, src, MOCK_RPMA_MR_LOCAL);
	expect_value(rpma_mr_send, offset, MOCK_LOCAL_OFFSET);
	expect_value(rpma_mr_send, len, MOCK_LEN);
	expect_value(rpma_mr_send, flags, MOCK_FLAGS);
	expect_value(rpma_mr_send, operation, IBV_WR_SEND_WITH_IMM);
	expect_value(rpma_mr_send, imm, MOCK_IMM_DATA);
	expect_value(rpma_mr_send, op_context, MOCK_OP_CONTEXT);
	will_return(rpma_mr_send, MOCK_OK);

	/* run test */
	int ret = rpma_send_with_imm(cstate->conn,
			MOCK_RPMA_MR_LOCAL, MOCK_LOCAL_OFFSET,
			MOCK_LEN, MOCK_FLAGS, MOCK_IMM_DATA,
			MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * group_setup_send -- prepare resources for all tests in the group
 */
static int
group_setup_send(void **unused)
{
	/* set value of QP in mock of CM ID */
	Cm_id.qp = MOCK_QP;

	return 0;
}

static const struct CMUnitTest tests_send_with_imm[] = {
	/* rpma_read_with_imm() unit tests */
	cmocka_unit_test(send_with_imm__src_NULL_offset_not_NULL),
	cmocka_unit_test(send_with_imm__src_NULL_len_not_NULL),
	cmocka_unit_test(send_with_imm__src_NULL_offset_len_not_NULL),
	cmocka_unit_test(send_with_imm__conn_NULL),
	cmocka_unit_test(send_with_imm__flags_0),
	cmocka_unit_test(send_with_imm__conn_NULL_flags_0),
	cmocka_unit_test_setup_teardown(send_with_imm__success,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_send_with_imm,
			group_setup_send, NULL);
}
