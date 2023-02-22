// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */
/* Copyright (c) 2023 Fujitsu Limited */

/*
 * flush-apm_execute.c -- unit tests of the flush module
 *
 * API covered:
 * - rpma_flush_apm_execute
 * - rpma_flush_native_execute
 */

#include "cmocka_headers.h"
#include "flush.h"
#include "mocks-ibverbs.h"
#include "mocks-unistd.h"
#include "test-common.h"
#include "flush-common.h"

/*
 * apm_execute__success -- rpma_flush_apm_execute() success
 */
static void
apm_execute__success(void **fstate_ptr)
{
	/* configure mocks */
	expect_value(rpma_mr_read, qp, MOCK_QP);
	expect_value(rpma_mr_read, dst, MOCK_RPMA_MR_LOCAL);
	expect_value(rpma_mr_read, dst_offset, 0);
	expect_value(rpma_mr_read, src, MOCK_RPMA_MR_REMOTE);
	expect_value(rpma_mr_read, src_offset, MOCK_REMOTE_OFFSET);
	expect_value(rpma_mr_read, len, MOCK_RAW_LEN);
	expect_value(rpma_mr_read, flags, MOCK_FLAGS);
	expect_value(rpma_mr_read, op_context, MOCK_OP_CONTEXT);
	will_return(rpma_mr_read, MOCK_OK);

	/* run test */
	struct flush_test_state *fstate = *fstate_ptr;
	int ret = fstate->flush->func(MOCK_QP, fstate->flush,
			MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
			MOCK_LEN, RPMA_FLUSH_TYPE_VISIBILITY,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	assert_int_equal(ret, MOCK_OK);
}

#ifdef NATIVE_FLUSH_SUPPORTED
/*
 * native_execute__success -- rpma_flush_native_execute() success
 */
static void
native_execute__success(void **fstate_ptr)
{
	/* configure mocks */
	expect_value(rpma_mr_flush, qp, MOCK_QP);
	expect_value(rpma_mr_flush, dst, MOCK_RPMA_MR_REMOTE);
	expect_value(rpma_mr_flush, dst_offset, MOCK_REMOTE_OFFSET);
	expect_value(rpma_mr_flush, len, MOCK_LEN);
	expect_value(rpma_mr_flush, type, RPMA_FLUSH_TYPE_VISIBILITY);
	expect_value(rpma_mr_flush, flags, MOCK_FLAGS);
	expect_value(rpma_mr_flush, op_context, MOCK_OP_CONTEXT);
	will_return(rpma_mr_flush, MOCK_OK);

	/* run test */
	struct flush_test_state *fstate = *fstate_ptr;
	int ret = fstate->flush->func(MOCK_QP, fstate->flush,
			MOCK_RPMA_MR_REMOTE, MOCK_REMOTE_OFFSET,
			MOCK_LEN, RPMA_FLUSH_TYPE_VISIBILITY,
			MOCK_FLAGS, MOCK_OP_CONTEXT);

	assert_int_equal(ret, MOCK_OK);
}
#endif

int
main(int argc, char *argv[])
{
	enable_unistd_mocks();

	const struct CMUnitTest tests[] = {
		/* rpma_flush_apm_execute() unit tests */
		cmocka_unit_test_setup_teardown(apm_execute__success,
			setup__apm_flush_new, teardown__apm_flush_delete),
#ifdef NATIVE_FLUSH_SUPPORTED
		/* rpma_flush_native_execute() unit tests */
		cmocka_unit_test_setup_teardown(native_execute__success,
			setup__native_flush_new, teardown__native_flush_delete),
#endif
	};

	int ret = cmocka_run_group_tests(tests, group_setup_flush_common, NULL);

	disable_unistd_mocks();

	return ret;
}
