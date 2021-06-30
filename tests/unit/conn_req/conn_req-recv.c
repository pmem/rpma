// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn_req-recv.c -- the rpma_conn_req_recv() unit tests
 *
 * API covered:
 * - rpma_conn_req_recv()
 */

#include "conn_req-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"

/*
 * recv__req_NULL - NULL req is invalid
 */
static void
recv__req_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_req_recv(NULL, MOCK_RPMA_MR_LOCAL,
			MOCK_LOCAL_OFFSET, MOCK_LEN,
			MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * recv__dst_NULL - NULL dst is invalid
 */
static void
recv__dst_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_req_recv(MOCK_CONN_REQ, NULL, MOCK_LOCAL_OFFSET,
			MOCK_LEN, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * recv__req_dst_NULL - NULL req and dst are invalid
 */
static void
recv__req_dst_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_req_recv(NULL, NULL, MOCK_LOCAL_OFFSET,
			MOCK_LEN, MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * recv__success - happy day scenario
 */
static void
recv__success(void **cstate_ptr)
{
	struct conn_req_new_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	expect_value(rpma_mr_recv, qp, MOCK_QP);
	expect_value(rpma_mr_recv, dst, MOCK_RPMA_MR_LOCAL);
	expect_value(rpma_mr_recv, offset, MOCK_LOCAL_OFFSET);
	expect_value(rpma_mr_recv, len, MOCK_LEN);
	expect_value(rpma_mr_recv, op_context, MOCK_OP_CONTEXT);
	will_return(rpma_mr_recv, MOCK_OK);

	/* run test */
	int ret = rpma_conn_req_recv(cstate->req, MOCK_RPMA_MR_LOCAL,
			MOCK_LOCAL_OFFSET, MOCK_LEN,
			MOCK_OP_CONTEXT);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

static const struct CMUnitTest tests_recv[] = {
	/* rpma_conn_req_recv() unit tests */
	cmocka_unit_test(recv__req_NULL),
	cmocka_unit_test(recv__dst_NULL),
	cmocka_unit_test(recv__req_dst_NULL),
	cmocka_unit_test_setup_teardown(recv__success,
		setup__conn_req_new, teardown__conn_req_new),
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_recv, NULL, NULL);
}
