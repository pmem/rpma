// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Fujitsu */

/*
 * srq-new_delete.c -- the rpma_srq_new/delete() unit tests
 *
 * APIs covered:
 * - rpma_srq_new()
 * - rpma_srq_delete()
 */

#include "srq-common.h"

/*
 * new__peer_NULL -- NULL peer is invalid
 */
static void
new__peer_NULL(void **unused)
{
	/* run test */
	struct rpma_srq *srq = NULL;
	int ret = rpma_srq_new(NULL, NULL, &srq);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(srq);
}

/*
 * new__srq_ptr_NULL -- NULL srq_ptr is invalid
 */
static void
new__srq_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_srq_new(MOCK_PEER, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new__rpma_peer_create_srq_ERRNO -- rpma_peer_create_srq() fails with MOCK_ERRNO
 */
static void
new__rpma_peer_create_srq_ERRNO(void **cstate_ptr)
{
	struct srq_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	will_return(rpma_peer_create_srq, &cstate->get_args);
	will_return(rpma_peer_create_srq, RPMA_E_PROVIDER);
	will_return(rpma_peer_create_srq, MOCK_ERRNO);

	/* run test */
	int ret = rpma_srq_new(MOCK_PEER, MOCK_GET_SRQ_CFG(cstate),
			&cstate->srq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * new__malloc_ERRNO -- malloc() fails with MOCK_ERRNO
 */
static void
new__malloc_ERRNO(void **cstate_ptr)
{
	struct srq_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	will_return(rpma_peer_create_srq, &cstate->get_args);
	will_return(rpma_peer_create_srq, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_ERRNO);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_SRQ_RCQ(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(ibv_destroy_srq, srq, MOCK_IBV_SRQ);
	will_return(ibv_destroy_srq, MOCK_OK);

	/* run test */
	int ret = rpma_srq_new(MOCK_PEER, MOCK_GET_SRQ_CFG(cstate),
			&cstate->srq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NOMEM);
}

/*
 * new__malloc_ERRNO_subsequent_ERRNO2 -- malloc() fails with MOCK_ERRNO
 * whereas subsequent (rpma_cq_delete(), ibv_destroy_srq()) fail with
 * MOCK_ERRNO2
 */
static void
new__malloc_ERRNO_subsequent_ERRNO2(void **cstate_ptr)
{
	struct srq_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	will_return(rpma_peer_create_srq, &cstate->get_args);
	will_return(rpma_peer_create_srq, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_ERRNO);
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_SRQ_RCQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO2);
	expect_value(ibv_destroy_srq, srq, MOCK_IBV_SRQ);
	will_return(ibv_destroy_srq, MOCK_ERRNO2);

	/* run test */
	int ret = rpma_srq_new(MOCK_PEER, MOCK_GET_SRQ_CFG(cstate),
			&cstate->srq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NOMEM);
}

/*
 * test_lifecycle - happy day scenario
 */
static void
test_lifecycle(void **unused)
{
	/*
	 * the thing is done by setup__srq_new() and teardown__srq_delete()
	 */
}

/*
 * delete__srq_ptr_NULL - NULL srq_ptr is invalid
 */
static void
delete__srq_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_srq_delete(NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * delete__srq_NULL - NULL *srq_ptr should exit quickly
 */
static void
delete__srq_NULL(void **unused)
{
	struct rpma_srq *srq = NULL;

	/* run test */
	int ret = rpma_srq_delete(&srq);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * delete__rpma_cq_delete_ERRNO -- rpma_cq_delete() fails with MOCK_ERRNO
 */
static void
delete__rpma_cq_delete_ERRNO(void **cstate_ptr)
{
	struct srq_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__srq_new((void **)&cstate), MOCK_OK);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_SRQ_RCQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO);
	expect_value(ibv_destroy_srq, srq, MOCK_IBV_SRQ);
	will_return(ibv_destroy_srq, MOCK_OK);

	/* run test */
	int ret = rpma_srq_delete(&cstate->srq);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->srq);
}

/*
 * delete__rpma_cq_delete_ERRNO_subsequent_ERRNO2 -- ibv_destroy_srq()
 * fails with MOCK_ERRNO2 after rpma_cq_delete() failed with MOCK_ERRNO
 */
static void
delete__rpma_cq_delete_ERRNO_subsequent_ERRNO2(void **cstate_ptr)
{
	struct srq_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__srq_new((void **)&cstate), MOCK_OK);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_RPMA_SRQ_RCQ);
	will_return(rpma_cq_delete, RPMA_E_PROVIDER);
	will_return(rpma_cq_delete, MOCK_ERRNO);
	expect_value(ibv_destroy_srq, srq, MOCK_IBV_SRQ);
	will_return(ibv_destroy_srq, MOCK_ERRNO2);

	/* run test */
	int ret = rpma_srq_delete(&cstate->srq);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->srq);
}

/*
 * delete__ibv_destroy_srq_ERRNO -- ibv_destroy_srq() fails with MOCK_ERRNO
 */
static void
delete__ibv_destroy_srq_ERRNO(void **cstate_ptr)
{
	struct srq_test_state *cstate = *cstate_ptr;
	assert_int_equal(setup__srq_new((void **)&cstate), MOCK_OK);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rpma_cq_delete, *cq_ptr, MOCK_GET_SRQ_RCQ(cstate));
	will_return(rpma_cq_delete, MOCK_OK);
	expect_value(ibv_destroy_srq, srq, MOCK_IBV_SRQ);
	will_return(ibv_destroy_srq, MOCK_ERRNO);

	/* run test */
	int ret = rpma_srq_delete(&cstate->srq);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cstate->srq);
}

static const struct CMUnitTest tests_new_delete[] = {
	/* rpma_srq_new() unit tests */
	cmocka_unit_test(new__peer_NULL),
	cmocka_unit_test(new__srq_ptr_NULL),
	SRQ_NEW_TEST_WITHOUT_CFG(new__rpma_peer_create_srq_ERRNO),
	SRQ_NEW_TEST_WITH_AND_WITHOUT_CFG(new__malloc_ERRNO),
	SRQ_NEW_TEST_WITHOUT_CFG(new__malloc_ERRNO_subsequent_ERRNO2),

	/* rpma_srq_new()/delete() lifecycle */
	SRQ_NEW_TEST_SETUP_TEARDOWN_WITH_AND_WITHOUT_CFG(test_lifecycle,
		setup__srq_new, teardown__srq_delete),

	/* rpma_srq_delete() unit tests */
	cmocka_unit_test(delete__srq_ptr_NULL),
	cmocka_unit_test(delete__srq_NULL),
	SRQ_NEW_TEST_WITHOUT_CFG(delete__rpma_cq_delete_ERRNO),
	SRQ_NEW_TEST_WITHOUT_CFG(
		delete__rpma_cq_delete_ERRNO_subsequent_ERRNO2),
	SRQ_NEW_TEST_WITH_AND_WITHOUT_CFG(delete__ibv_destroy_srq_ERRNO),

	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_new_delete, NULL, NULL);
}
