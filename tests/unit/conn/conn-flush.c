// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * conn-flush.c -- the rpma_flush() unit tests
 *
 * APIs covered:
 * - rpma_flush()
 */

#include "conn-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rpma-flush.h"

/*
 * flush__conn_NULL - NULL conn is invalid
 */
static void
flush__conn_NULL(void **unused)
{
	/* run test */
	int ret = rpma_flush(NULL, MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
			MOCK_LEN, RPMA_FLUSH_TYPE_VISIBILITY,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * flush__dst_NULL - NULL dst is invalid
 */
static void
flush__dst_NULL(void **unused)
{
	/* run test */
	int ret = rpma_flush(MOCK_CONN, NULL, MOCK_REMOTE_OFFSET,
			MOCK_LEN, RPMA_FLUSH_TYPE_VISIBILITY,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * flush__flags_0 - flags == 0 is invalid
 */
static void
flush__flags_0(void **unused)
{
	/* run test */
	int ret = rpma_flush(MOCK_CONN, MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
			MOCK_LEN, RPMA_FLUSH_TYPE_VISIBILITY,
			0, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * flush__conn_dst_NULL_flags_0 - NULL conn, dst
 * and flags == 0 are invalid
 */
static void
flush__conn_dst_NULL_flags_0(void **unused)
{
	/* run test */
	int ret = rpma_flush(NULL, NULL, MOCK_REMOTE_OFFSET,
			MOCK_LEN, RPMA_FLUSH_TYPE_VISIBILITY,
			0, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * flush__FLUSH_PERSISTENT_NO_DIRECT_WRITE - flush fails with RPMA_E_NOSUPP
 * for RPMA_FLUSH_TYPE_PERSISTENT and not supported direct_write_to_pmem
 */
static void
flush__FLUSH_PERSISTENT_NO_DIRECT_WRITE(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* set direct_write_to_pmem to false */
	will_return(rpma_peer_cfg_get_direct_write_to_pmem, false);
	int ret = rpma_conn_apply_remote_peer_cfg(cstate->conn, MOCK_PEER_PCFG);
	assert_int_equal(ret, MOCK_OK);

	/* run test */
	ret = rpma_flush(cstate->conn, MOCK_RPMA_MR_REMOTE,
			MOCK_REMOTE_OFFSET, MOCK_LEN,
			RPMA_FLUSH_TYPE_PERSISTENT,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOSUPP);
}

/*
 * flush__FLUSH_PERSISTENT_USAGE_EMPTY - flush fails with RPMA_E_NOSUPP
 * for RPMA_FLUSH_TYPE_PERSISTENT and no usage
 */
static void
flush__FLUSH_PERSISTENT_USAGE_EMPTY(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* set direct_write_to_pmem to true */
	will_return(rpma_peer_cfg_get_direct_write_to_pmem, true);
	int ret = rpma_conn_apply_remote_peer_cfg(cstate->conn, MOCK_PEER_PCFG);
	assert_int_equal(ret, MOCK_OK);

	/* configure mocks */
	expect_value(rpma_mr_remote_get_flush_type, mr, MOCK_RPMA_MR_REMOTE);
	will_return(rpma_mr_remote_get_flush_type, 0);

	/* run test */
	ret = rpma_flush(cstate->conn, MOCK_RPMA_MR_REMOTE,
			MOCK_REMOTE_OFFSET, MOCK_LEN,
			RPMA_FLUSH_TYPE_PERSISTENT,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOSUPP);
}

/*
 * flush__FLUSH_PERSISTENT_USAGE_VISIBILITY - flush fails with RPMA_E_NOSUPP
 * for RPMA_FLUSH_TYPE_PERSISTENT and RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY
 */
static void
flush__FLUSH_PERSISTENT_USAGE_VISIBILITY(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* set direct_write_to_pmem to true */
	will_return(rpma_peer_cfg_get_direct_write_to_pmem, true);
	int ret = rpma_conn_apply_remote_peer_cfg(cstate->conn, MOCK_PEER_PCFG);
	assert_int_equal(ret, MOCK_OK);

	/* configure mocks */
	expect_value(rpma_mr_remote_get_flush_type, mr, MOCK_RPMA_MR_REMOTE);
	will_return(rpma_mr_remote_get_flush_type,
			RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY);

	/* run test */
	ret = rpma_flush(cstate->conn, MOCK_RPMA_MR_REMOTE,
			MOCK_REMOTE_OFFSET, MOCK_LEN,
			RPMA_FLUSH_TYPE_PERSISTENT,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOSUPP);
}

/*
 * flush__FLUSH_VISIBILITY_USAGE_EMPTY - flush fails with RPMA_E_NOSUPP
 * for RPMA_FLUSH_TYPE_VISIBILITY and no usage
 */
static void
flush__FLUSH_VISIBILITY_USAGE_EMPTY(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* set direct_write_to_pmem to true */
	will_return(rpma_peer_cfg_get_direct_write_to_pmem, true);
	int ret = rpma_conn_apply_remote_peer_cfg(cstate->conn, MOCK_PEER_PCFG);
	assert_int_equal(ret, MOCK_OK);

	/* configure mocks */
	expect_value(rpma_mr_remote_get_flush_type, mr, MOCK_RPMA_MR_REMOTE);
	will_return(rpma_mr_remote_get_flush_type, 0);

	/* run test */
	ret = rpma_flush(cstate->conn, MOCK_RPMA_MR_REMOTE,
			MOCK_REMOTE_OFFSET, MOCK_LEN,
			RPMA_FLUSH_TYPE_VISIBILITY,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOSUPP);
}

/*
 * flush__FLUSH_VISIBILITY_USAGE_PERSISTENT - flush fails with RPMA_E_NOSUPP
 * for RPMA_FLUSH_TYPE_VISIBILITY and RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT
 */
static void
flush__FLUSH_VISIBILITY_USAGE_PERSISTENT(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* set direct_write_to_pmem to true */
	will_return(rpma_peer_cfg_get_direct_write_to_pmem, true);
	int ret = rpma_conn_apply_remote_peer_cfg(cstate->conn, MOCK_PEER_PCFG);
	assert_int_equal(ret, MOCK_OK);

	/* configure mocks */
	expect_value(rpma_mr_remote_get_flush_type, mr, MOCK_RPMA_MR_REMOTE);
	will_return(rpma_mr_remote_get_flush_type,
			RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT);

	/* run test */
	ret = rpma_flush(cstate->conn, MOCK_RPMA_MR_REMOTE,
			MOCK_REMOTE_OFFSET, MOCK_LEN,
			RPMA_FLUSH_TYPE_VISIBILITY,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOSUPP);
}

/*
 * flush__success_FLUSH_TYPE_VISIBILITY - happy day scenario
 * for RPMA_FLUSH_TYPE_VISIBILITY
 */
static void
flush__success_FLUSH_TYPE_VISIBILITY(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	expect_value(rpma_flush_mock_execute, qp, MOCK_QP);
	expect_value(rpma_flush_mock_execute, flush, MOCK_FLUSH);
	expect_value(rpma_flush_mock_execute, dst, MOCK_RPMA_MR_REMOTE);
	expect_value(rpma_flush_mock_execute, dst_offset, MOCK_REMOTE_OFFSET);
	expect_value(rpma_flush_mock_execute, len, MOCK_LEN);
	expect_value(rpma_flush_mock_execute, flags, MOCK_FLAGS);
	expect_value(rpma_flush_mock_execute, op_context, MOCK_OP_CONTEXT);

	expect_value(rpma_mr_remote_get_flush_type, mr, MOCK_RPMA_MR_REMOTE);
	will_return(rpma_mr_remote_get_flush_type,
			RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY);

	/* run test */
	int ret = rpma_flush(cstate->conn, MOCK_RPMA_MR_REMOTE,
			MOCK_REMOTE_OFFSET, MOCK_LEN,
			RPMA_FLUSH_TYPE_VISIBILITY,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * flush__success_FLUSH_TYPE_PERSISTENT - happy day scenario
 * for RPMA_FLUSH_TYPE_PERSISTENT
 */
static void
flush__success_FLUSH_TYPE_PERSISTENT(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* set direct_write_to_pmem to true */
	will_return(rpma_peer_cfg_get_direct_write_to_pmem, true);
	int ret = rpma_conn_apply_remote_peer_cfg(cstate->conn, MOCK_PEER_PCFG);
	assert_int_equal(ret, MOCK_OK);

	/* configure mocks for rpma_flush() */
	expect_value(rpma_flush_mock_execute, qp, MOCK_QP);
	expect_value(rpma_flush_mock_execute, flush, MOCK_FLUSH);
	expect_value(rpma_flush_mock_execute, dst, MOCK_RPMA_MR_REMOTE);
	expect_value(rpma_flush_mock_execute, dst_offset, MOCK_REMOTE_OFFSET);
	expect_value(rpma_flush_mock_execute, len, MOCK_LEN);
	expect_value(rpma_flush_mock_execute, flags, MOCK_FLAGS);
	expect_value(rpma_flush_mock_execute, op_context, MOCK_OP_CONTEXT);

	expect_value(rpma_mr_remote_get_flush_type, mr, MOCK_RPMA_MR_REMOTE);
	will_return(rpma_mr_remote_get_flush_type,
			RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT);

	/* run test */
	ret = rpma_flush(cstate->conn, MOCK_RPMA_MR_REMOTE,
			MOCK_REMOTE_OFFSET, MOCK_LEN,
			RPMA_FLUSH_TYPE_PERSISTENT,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

static const struct CMUnitTest tests_flush[] = {
	/* rpma_read() unit tests */
	cmocka_unit_test(flush__conn_NULL),
	cmocka_unit_test(flush__dst_NULL),
	cmocka_unit_test(flush__flags_0),
	cmocka_unit_test(flush__conn_dst_NULL_flags_0),
	cmocka_unit_test_setup_teardown(
		flush__FLUSH_PERSISTENT_NO_DIRECT_WRITE,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(
		flush__FLUSH_PERSISTENT_USAGE_EMPTY,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(
		flush__FLUSH_PERSISTENT_USAGE_VISIBILITY,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(
		flush__FLUSH_VISIBILITY_USAGE_EMPTY,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(
		flush__FLUSH_VISIBILITY_USAGE_PERSISTENT,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(flush__success_FLUSH_TYPE_VISIBILITY,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(flush__success_FLUSH_TYPE_PERSISTENT,
		setup__conn_new, teardown__conn_delete),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_flush, NULL, NULL);
}
