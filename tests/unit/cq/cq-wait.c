// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Fujitsu */

/*
 * cq-wait.c -- the rpma_cq_wait() unit tests
 *
 * API covered:
 * - rpma_cq_wait()
 */

#include "librpma.h"
#include "cmocka_headers.h"
#include "mocks-ibverbs.h"
#include "cq-common.h"

/*
 * wait__cq_NULL - cq NULL is invalid
 */
static void
wait__cq_NULL(void **unused)
{
	/* run test */
	int ret = rpma_cq_wait(NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * wait__get_cq_event_ERRNO - ibv_get_cq_event() fails with MOCK_ERRNO
 */
static void
wait__get_cq_event_ERRNO(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;

	/* configure mock */
	expect_value(ibv_get_cq_event, channel, MOCK_COMP_CHANNEL);
	will_return(ibv_get_cq_event, MOCK_ERRNO);

	/* run test */
	int ret = rpma_cq_wait(cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_NO_COMPLETION);
}

/*
 * wait__req_notify_cq_ERRNO - ibv_req_notify_cq() fails with MOCK_ERRNO
 */
static void
wait__req_notify_cq_ERRNO(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;

	/* configure mocks */
	expect_value(ibv_get_cq_event, channel, MOCK_COMP_CHANNEL);
	will_return(ibv_get_cq_event, MOCK_OK);
	will_return(ibv_get_cq_event, MOCK_IBV_CQ);
	expect_value(ibv_ack_cq_events, cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_ERRNO);

	/* run test */
	int ret = rpma_cq_wait(cq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * wait__success - happy day scenario
 */
static void
wait__success(void **cq_ptr)
{
	struct rpma_cq *cq = *cq_ptr;

	/* configure mocks */
	expect_value(ibv_get_cq_event, channel, MOCK_COMP_CHANNEL);
	will_return(ibv_get_cq_event, MOCK_OK);
	will_return(ibv_get_cq_event, MOCK_IBV_CQ);
	expect_value(ibv_ack_cq_events, cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_OK);

	/* run test */
	int ret = rpma_cq_wait(cq);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
}

static const struct CMUnitTest tests_wait[] = {
	/* rpma_cq_wait() unit tests */
	cmocka_unit_test(wait__cq_NULL),
	cmocka_unit_test_setup_teardown(
		wait__get_cq_event_ERRNO,
		setup__cq_new, teardown__cq_delete),
	cmocka_unit_test_setup_teardown(
		wait__req_notify_cq_ERRNO,
		setup__cq_new, teardown__cq_delete),
	cmocka_unit_test_setup_teardown(
		wait__success,
		setup__cq_new, teardown__cq_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_wait,
			group_setup_common_cq, NULL);
}
