/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn-test-next-event.c -- the connection next_event unit tests
 *
 * API covered:
 * - rpma_conn_next_event()
 */

#include "conn-common.h"

/*
 * next_event__conn_NULL - NULL conn is invalid
 */
static void
next_event__conn_NULL(void **unused)
{
	/* run test */
	enum rpma_conn_event c_event = RPMA_CONN_UNDEFINED;
	int ret = rpma_conn_next_event(NULL, &c_event);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_int_equal(c_event, RPMA_CONN_UNDEFINED);
}

/*
 * next_event__event_NULL - NULL event is invalid
 */
static void
next_event__event_NULL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_next_event(cstate->conn, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * next_event__conn_NULL_event_NULL - NULL conn and NULL event are invalid
 */
static void
next_event__conn_NULL_event_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_next_event(NULL, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * next_event__get_cm_event_EAGAIN -
 * rdma_get_cm_event() fails with EAGAIN
 */
static void
next_event__get_cm_event_EAGAIN(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	will_return(rdma_get_cm_event, NULL);
	will_return(rdma_get_cm_event, EAGAIN);

	/* run test */
	enum rpma_conn_event c_event = RPMA_CONN_UNDEFINED;
	int ret = rpma_conn_next_event(cstate->conn, &c_event);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_int_equal(c_event, RPMA_CONN_UNDEFINED);
}

/*
 * next_event__get_cm_event_ENODATA -
 * rdma_get_cm_event() fails with ENODATA
 */
static void
next_event__get_cm_event_ENODATA(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	will_return(rdma_get_cm_event, NULL);
	will_return(rdma_get_cm_event, ENODATA);

	/* run test */
	enum rpma_conn_event c_event = RPMA_CONN_UNDEFINED;
	int ret = rpma_conn_next_event(cstate->conn, &c_event);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NO_NEXT);
	assert_int_equal(c_event, RPMA_CONN_UNDEFINED);
}

/*
 * next_event__event_REJECTED -
 * RDMA_CM_EVENT_REJECTED is unexpected
 */
static void
next_event__event_REJECTED(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_REJECTED;
	will_return(rdma_get_cm_event, &event);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	/* run test */
	enum rpma_conn_event c_event = RPMA_CONN_UNDEFINED;
	int ret = rpma_conn_next_event(cstate->conn, &c_event);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_UNKNOWN);
	assert_int_equal(c_event, RPMA_CONN_UNDEFINED);
}

/*
 * next_event__event_REJECTED_ack_EINVAL -
 * rdma_ack_cm_event() fails with EINVAL after obtaining
 * an RDMA_CM_EVENT_REJECTED event
 */
static void
next_event__event_REJECTED_ack_EINVAL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_REJECTED;
	will_return(rdma_get_cm_event, &event);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, EINVAL);

	expect_value(rpma_private_data_discard, pdata->ptr, NULL);
	expect_value(rpma_private_data_discard, pdata->len, 0);

	/* run test */
	enum rpma_conn_event c_event = RPMA_CONN_UNDEFINED;
	int ret = rpma_conn_next_event(cstate->conn, &c_event);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EINVAL);
	assert_int_equal(c_event, RPMA_CONN_UNDEFINED);
}

/*
 * next_event__data_store_ENOMEM - rpma_private_data_store fails
 * with RPMA_E_NOMEM
 */
static void
next_event__data_store_ENOMEM(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_ESTABLISHED;
	will_return(rdma_get_cm_event, &event);
	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, MOCK_OK);
	will_return(rpma_private_data_store, RPMA_E_NOMEM);

	/* run test */
	enum rpma_conn_event c_event = RPMA_CONN_UNDEFINED;
	int ret = rpma_conn_next_event(cstate->conn, &c_event);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_int_equal(c_event, RPMA_CONN_UNDEFINED);
}

/*
 * next_event__success_no_data_ESTABLISHED_no_data - happy day scenario,
 * no private data in the connection and no private data in the event
 */
static void
next_event__success_no_data_ESTABLISHED_no_data(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks for rpma_conn_next_event() */
	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_ESTABLISHED;
	/* no private data in the event */
	event.param.conn.private_data = NULL;
	event.param.conn.private_data_len = 0;
	will_return(rdma_get_cm_event, &event);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	will_return_maybe(rpma_private_data_store, MOCK_OK);

	/* run test */
	enum rpma_conn_event c_event = RPMA_CONN_UNDEFINED;
	int ret = rpma_conn_next_event(cstate->conn, &c_event);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(c_event, RPMA_CONN_ESTABLISHED);

	/* get private data for verification */
	struct rpma_conn_private_data check_data;
	ret = rpma_conn_get_private_data(cstate->conn, &check_data);

	/* verify the results of rpma_conn_get_private_data() */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(check_data.ptr, NULL);
	assert_int_equal(check_data.len, 0);

	/* set expected private data */
	cstate->data.ptr = NULL;
	cstate->data.len = 0;
}

/*
 * next_event__success_no_data_ESTABLISHED_with_data - happy day scenario
 * no private data in the connection and with private data in the event
 */
static void
next_event__success_no_data_ESTABLISHED_with_data(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks for rpma_conn_next_event() */
	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_ESTABLISHED;
	/* with private data in the event */
	event.param.conn.private_data = MOCK_PRIVATE_DATA;
	event.param.conn.private_data_len = MOCK_PDATA_LEN;
	will_return(rdma_get_cm_event, &event);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	will_return_maybe(rpma_private_data_store, MOCK_OK);

	/* run test */
	enum rpma_conn_event c_event = RPMA_CONN_UNDEFINED;
	int ret = rpma_conn_next_event(cstate->conn, &c_event);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(c_event, RPMA_CONN_ESTABLISHED);

	/* get private data for verification */
	struct rpma_conn_private_data check_data;
	ret = rpma_conn_get_private_data(cstate->conn, &check_data);

	/* verify the results of rpma_conn_get_private_data() */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(check_data.ptr, MOCK_PRIVATE_DATA);
	assert_int_equal(check_data.len, MOCK_PDATA_LEN);

	/* set expected private data */
	cstate->data.ptr = MOCK_PRIVATE_DATA;
	cstate->data.len = MOCK_PDATA_LEN;
}

/*
 * next_event__success_with_data_ESTABLISHED_no_data - happy day scenario,
 * with private data in the connection and with no private data in the event
 */
static void
next_event__success_with_data_ESTABLISHED_no_data(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks for rpma_conn_set_private_data() */
	struct rpma_conn_private_data data;
	data.ptr = MOCK_PRIVATE_DATA;
	data.len = MOCK_PDATA_LEN;
	will_return(rpma_private_data_copy, 0);
	will_return(rpma_private_data_copy, MOCK_PRIVATE_DATA);

	/* set private data in the connection */
	int ret = rpma_conn_set_private_data(cstate->conn, &data);

	/* verify the results of rpma_conn_set_private_data() */
	assert_int_equal(ret, MOCK_OK);

	/* get the private data */
	struct rpma_conn_private_data check_data;
	ret = rpma_conn_get_private_data(cstate->conn, &check_data);

	/* verify the results of rpma_conn_get_private_data() */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(check_data.ptr, data.ptr);
	assert_int_equal(check_data.len, data.len);

	/* configure mocks for rpma_conn_next_event() */
	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_ESTABLISHED;
	/* no private data in the event */
	event.param.conn.private_data = NULL;
	event.param.conn.private_data_len = 0;
	will_return(rdma_get_cm_event, &event);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	will_return_maybe(rpma_private_data_store, MOCK_OK);

	/* run test */
	enum rpma_conn_event c_event = RPMA_CONN_UNDEFINED;
	ret = rpma_conn_next_event(cstate->conn, &c_event);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(c_event, RPMA_CONN_ESTABLISHED);

	/* get private data for verification */
	ret = rpma_conn_get_private_data(cstate->conn, &check_data);

	/* verify the results of rpma_conn_get_private_data() */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(check_data.ptr, MOCK_PRIVATE_DATA);
	assert_int_equal(check_data.len, MOCK_PDATA_LEN);

	/* set expected private data */
	cstate->data.ptr = MOCK_PRIVATE_DATA;
	cstate->data.len = MOCK_PDATA_LEN;
}

/*
 * next_event__success_with_data_ESTABLISHED_with_data - happy day scenario,
 * with private data in the connection and with another private data
 * in the event
 */
static void
next_event__success_with_data_ESTABLISHED_with_data(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks for rpma_conn_set_private_data() */
	struct rpma_conn_private_data data;
	data.ptr = MOCK_PRIVATE_DATA;
	data.len = MOCK_PDATA_LEN;
	will_return(rpma_private_data_copy, 0);
	will_return(rpma_private_data_copy, MOCK_PRIVATE_DATA);

	/* set private data in the connection */
	int ret = rpma_conn_set_private_data(cstate->conn, &data);

	/* verify the results of rpma_conn_set_private_data() */
	assert_int_equal(ret, MOCK_OK);

	/* get the private data */
	struct rpma_conn_private_data check_data;
	ret = rpma_conn_get_private_data(cstate->conn, &check_data);

	/* verify the results of rpma_conn_get_private_data() */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(check_data.ptr, data.ptr);
	assert_int_equal(check_data.len, data.len);

	/* configure mocks for rpma_conn_next_event() */
	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_ESTABLISHED;
	/* with another private data in the event */
	event.param.conn.private_data = MOCK_PRIVATE_DATA_2;
	event.param.conn.private_data_len = MOCK_PDATA_LEN_2;
	will_return(rdma_get_cm_event, &event);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	/* another data in the event */
	will_return_maybe(rpma_private_data_store, MOCK_OK);

	/* run test */
	enum rpma_conn_event c_event = RPMA_CONN_UNDEFINED;
	ret = rpma_conn_next_event(cstate->conn, &c_event);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(c_event, RPMA_CONN_ESTABLISHED);

	/* get private data for verification */
	ret = rpma_conn_get_private_data(cstate->conn, &check_data);

	/* verify the results of rpma_conn_get_private_data() */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(check_data.ptr, MOCK_PRIVATE_DATA);
	assert_int_equal(check_data.len, MOCK_PDATA_LEN);

	/* set expected private data */
	cstate->data.ptr = MOCK_PRIVATE_DATA;
	cstate->data.len = MOCK_PDATA_LEN;
}

/*
 * next_event__success_CONNECT_ERROR - happy day scenario
 */
static void
next_event__success_CONNECT_ERROR(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_CONNECT_ERROR;
	will_return(rdma_get_cm_event, &event);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	/* run test */
	enum rpma_conn_event c_event = RPMA_CONN_UNDEFINED;
	int ret = rpma_conn_next_event(cstate->conn, &c_event);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(c_event, RPMA_CONN_LOST);
}

/*
 * next_event__success_DEVICE_REMOVAL - happy day scenario
 */
static void
next_event__success_DEVICE_REMOVAL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_DEVICE_REMOVAL;
	will_return(rdma_get_cm_event, &event);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	/* run test */
	enum rpma_conn_event c_event = RPMA_CONN_UNDEFINED;
	int ret = rpma_conn_next_event(cstate->conn, &c_event);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(c_event, RPMA_CONN_LOST);
}

/*
 * next_event__success_DISCONNECTED - happy day scenario
 */
static void
next_event__success_DISCONNECTED(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_DISCONNECTED;
	will_return(rdma_get_cm_event, &event);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	/* run test */
	enum rpma_conn_event c_event = RPMA_CONN_UNDEFINED;
	int ret = rpma_conn_next_event(cstate->conn, &c_event);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(c_event, RPMA_CONN_CLOSED);
}

/*
 * next_event__success_TIMEWAIT_EXIT - happy day scenario
 */
static void
next_event__success_TIMEWAIT_EXIT(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_TIMEWAIT_EXIT;
	will_return(rdma_get_cm_event, &event);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	/* run test */
	enum rpma_conn_event c_event = RPMA_CONN_UNDEFINED;
	int ret = rpma_conn_next_event(cstate->conn, &c_event);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(c_event, RPMA_CONN_CLOSED);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests_next_event[] = {
		/* rpma_conn_next_event() unit tests */
		cmocka_unit_test(next_event__conn_NULL),
		cmocka_unit_test_setup_teardown(
			next_event__event_NULL,
			setup__conn_new, teardown__conn_delete),
		cmocka_unit_test(next_event__conn_NULL_event_NULL),
		cmocka_unit_test_setup_teardown(
			next_event__get_cm_event_EAGAIN,
			setup__conn_new, teardown__conn_delete),
		cmocka_unit_test_setup_teardown(
			next_event__get_cm_event_ENODATA,
			setup__conn_new, teardown__conn_delete),
		cmocka_unit_test_setup_teardown(
			next_event__event_REJECTED,
			setup__conn_new, teardown__conn_delete),
		cmocka_unit_test_setup_teardown(
			next_event__event_REJECTED_ack_EINVAL,
			setup__conn_new, teardown__conn_delete),
		cmocka_unit_test_setup_teardown(
			next_event__data_store_ENOMEM,
			setup__conn_new, teardown__conn_delete),
		cmocka_unit_test_setup_teardown(
			next_event__success_no_data_ESTABLISHED_no_data,
			setup__conn_new, teardown__conn_delete),
		cmocka_unit_test_setup_teardown(
			next_event__success_no_data_ESTABLISHED_with_data,
			setup__conn_new, teardown__conn_delete),
		cmocka_unit_test_setup_teardown(
			next_event__success_with_data_ESTABLISHED_no_data,
			setup__conn_new, teardown__conn_delete),
		cmocka_unit_test_setup_teardown(
			next_event__success_with_data_ESTABLISHED_with_data,
			setup__conn_new, teardown__conn_delete),
		cmocka_unit_test_setup_teardown(
			next_event__success_CONNECT_ERROR,
			setup__conn_new, teardown__conn_delete),
		cmocka_unit_test_setup_teardown(
			next_event__success_DEVICE_REMOVAL,
			setup__conn_new, teardown__conn_delete),
		cmocka_unit_test_setup_teardown(
			next_event__success_DISCONNECTED,
			setup__conn_new, teardown__conn_delete),
		cmocka_unit_test_setup_teardown(
			next_event__success_TIMEWAIT_EXIT,
			setup__conn_new, teardown__conn_delete),
		cmocka_unit_test(NULL)
	};

	return cmocka_run_group_tests(tests_next_event, NULL, NULL);
}
