// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * conn-wait.c -- the rpma_conn_wait() unit tests
 *
 * APIs covered:
 * - rpma_conn_wait()
 */

#include "conn-common.h"
#include "mocks-ibverbs.h"
#include "mocks-rdma_cm.h"
#include "test-common.h"

/*
 * wait__conn_NULL - NULL conn is invalid
 */
static void
wait__conn_NULL(void **unused)
{
	/* run test */
	struct rpma_cq *cq = NULL;
	bool is_rcq;
	int ret = rpma_conn_wait(NULL, 0, &cq, &is_rcq);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(cq);
}

/*
 * wait__cq_NULL - NULL cq is invalid
 */
static void
wait__cq_NULL(void **unused)
{
	/* run test */
	bool is_rcq;
	int ret = rpma_conn_wait(MOCK_CONN, 0, NULL, &is_rcq);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * wait__channel_not_shared - NULL conn->channel is invalid
 */
static void
wait__channel_not_shared(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* run test */
	struct rpma_cq *cq = NULL;
	bool is_rcq;
	int ret = rpma_conn_wait(cstate->conn, 0, &cq, &is_rcq);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOT_SHARED_CHNL);
	assert_null(cq);
}

/*
 * wait__get_cq_event_ERRNO - ibv_get_cq_event() fails with MOCK_ERRNO
 */
static void
wait__get_cq_event_ERRNO(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mock */
	expect_value(ibv_get_cq_event, channel, MOCK_COMP_CHANNEL);
	will_return(ibv_get_cq_event, MOCK_ERRNO);

	/* run test */
	struct rpma_cq *cq = NULL;
	bool is_rcq;
	int ret = rpma_conn_wait(cstate->conn, 0, &cq, &is_rcq);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NO_COMPLETION);
	assert_null(cq);
}

/*
 * wait__get_cq_event_UNKNOWN - ibv_get_cq_event() returned unknown CQ
 */
static void
wait__get_cq_event_UNKNOWN(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mock */
	expect_value(ibv_get_cq_event, channel, MOCK_COMP_CHANNEL);
	will_return(ibv_get_cq_event, MOCK_OK);
	will_return(ibv_get_cq_event, MOCK_IBV_CQ_UNKNOWN);
	expect_value(rpma_cq_get_ibv_cq, cq, MOCK_RPMA_CQ);
	will_return(rpma_cq_get_ibv_cq, MOCK_IBV_CQ);
	expect_value(rpma_cq_get_ibv_cq, cq, MOCK_RPMA_RCQ);
	will_return(rpma_cq_get_ibv_cq, MOCK_IBV_RCQ);

	/* run test */
	struct rpma_cq *cq = NULL;
	bool is_rcq;
	int ret = rpma_conn_wait(cstate->conn, 0, &cq, &is_rcq);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_UNKNOWN);
	assert_null(cq);
}

/*
 * wait__req_notify_cq_ERRNO - ibv_req_notify_cq() fails with MOCK_ERRNO
 */
static void
wait__req_notify_cq_ERRNO(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mock */
	expect_value(ibv_get_cq_event, channel, MOCK_COMP_CHANNEL);
	will_return(ibv_get_cq_event, MOCK_OK);
	will_return(ibv_get_cq_event, MOCK_IBV_CQ);
	expect_value(rpma_cq_get_ibv_cq, cq, MOCK_RPMA_CQ);
	will_return(rpma_cq_get_ibv_cq, MOCK_IBV_CQ);
	expect_value(ibv_ack_cq_events, cq, MOCK_IBV_CQ);
	expect_value(ibv_req_notify_cq_mock, cq, MOCK_IBV_CQ);
	will_return(ibv_req_notify_cq_mock, MOCK_ERRNO);

	/* run test */
	struct rpma_cq *cq = NULL;
	bool is_rcq;
	int ret = rpma_conn_wait(cstate->conn, 0, &cq, &is_rcq);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(cq);
	assert_int_equal(is_rcq, false);
}

/*
 * wait__success_is_rcq_NULL - happy day scenario without is_rcq
 */
static void
wait__success_is_rcq_NULL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mock */
	expect_value(ibv_get_cq_event, channel, MOCK_COMP_CHANNEL);
	will_return(ibv_get_cq_event, MOCK_OK);
	will_return(ibv_get_cq_event, MOCK_IBV_RCQ);
	expect_value(rpma_cq_get_ibv_cq, cq, MOCK_RPMA_CQ);
	will_return(rpma_cq_get_ibv_cq, MOCK_IBV_CQ);
	expect_value(rpma_cq_get_ibv_cq, cq, MOCK_RPMA_RCQ);
	will_return(rpma_cq_get_ibv_cq, MOCK_IBV_RCQ);
	expect_value(ibv_ack_cq_events, cq, MOCK_IBV_RCQ);
	expect_value(ibv_req_notify_cq_mock, cq, MOCK_IBV_RCQ);
	will_return(ibv_req_notify_cq_mock, MOCK_OK);

	/* run test */
	struct rpma_cq *cq = NULL;
	int ret = rpma_conn_wait(cstate->conn, 0, &cq, NULL);
	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(cq, MOCK_RPMA_RCQ);
}

/*
 * wait__success - happy day scenario
 */
static void
wait__success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mock */
	expect_value(ibv_get_cq_event, channel, MOCK_COMP_CHANNEL);
	will_return(ibv_get_cq_event, MOCK_OK);
	will_return(ibv_get_cq_event, MOCK_IBV_RCQ);
	expect_value(rpma_cq_get_ibv_cq, cq, MOCK_RPMA_CQ);
	will_return(rpma_cq_get_ibv_cq, MOCK_IBV_CQ);
	expect_value(rpma_cq_get_ibv_cq, cq, MOCK_RPMA_RCQ);
	will_return(rpma_cq_get_ibv_cq, MOCK_IBV_RCQ);
	expect_value(ibv_ack_cq_events, cq, MOCK_IBV_RCQ);
	expect_value(ibv_req_notify_cq_mock, cq, MOCK_IBV_RCQ);
	will_return(ibv_req_notify_cq_mock, MOCK_OK);

	/* run test */
	struct rpma_cq *cq = NULL;
	bool is_rcq;
	int ret = rpma_conn_wait(cstate->conn, 0, &cq, &is_rcq);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(cq, MOCK_RPMA_RCQ);
	assert_int_equal(is_rcq, true);
}


static const struct CMUnitTest tests_new[] = {
	/* rpma_conn_wait() unit tests */
	cmocka_unit_test(wait__conn_NULL),
	cmocka_unit_test(wait__cq_NULL),
	cmocka_unit_test_setup_teardown(wait__channel_not_shared,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_prestate_setup_teardown(wait__get_cq_event_ERRNO,
		setup__conn_new, teardown__conn_delete,
		&Conn_with_rcq_with_channel),
	cmocka_unit_test_prestate_setup_teardown(wait__get_cq_event_UNKNOWN,
		setup__conn_new, teardown__conn_delete,
		&Conn_with_rcq_with_channel),
	cmocka_unit_test_prestate_setup_teardown(wait__req_notify_cq_ERRNO,
		setup__conn_new, teardown__conn_delete,
		&Conn_with_rcq_with_channel),
	cmocka_unit_test_prestate_setup_teardown(wait__success_is_rcq_NULL,
		setup__conn_new, teardown__conn_delete,
		&Conn_with_rcq_with_channel),
	cmocka_unit_test_prestate_setup_teardown(wait__success,
		setup__conn_new, teardown__conn_delete,
		&Conn_with_rcq_with_channel)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_new,
		group_setup_common_conn, NULL);
}
