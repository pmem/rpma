// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * conn-next-event.c -- the connection next_event unit tests
 *
 * API covered:
 * - rpma_conn_next_event()
 */

#include "conn-common.h"
#include "mocks-rdma_cm.h"

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
 * next_event__get_cm_event_ERRNO -
 * rdma_get_cm_event() fails with MOCK_ERRNO
 */
static void
next_event__get_cm_event_ERRNO(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	will_return(rdma_get_cm_event, NULL);
	will_return(rdma_get_cm_event, MOCK_ERRNO);

	/* run test */
	enum rpma_conn_event c_event = RPMA_CONN_UNDEFINED;
	int ret = rpma_conn_next_event(cstate->conn, &c_event);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
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
	assert_int_equal(ret, RPMA_E_NO_EVENT);
	assert_int_equal(c_event, RPMA_CONN_UNDEFINED);
}

/*
 * next_event__event_UNREACHABLE -
 * RDMA_CM_EVENT_UNREACHABLE is unexpected
 */
static void
next_event__event_UNREACHABLE(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_UNREACHABLE;
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
 * next_event__event_UNREACHABLE_ack_ERRNO -
 * rdma_ack_cm_event() fails with MOCK_ERRNO after obtaining
 * an RDMA_CM_EVENT_UNREACHABLE event
 */
static void
next_event__event_UNREACHABLE_ack_ERRNO(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_UNREACHABLE;
	will_return(rdma_get_cm_event, &event);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, MOCK_ERRNO);

	expect_value(rpma_private_data_discard, pdata->ptr, NULL);
	expect_value(rpma_private_data_discard, pdata->len, 0);

	/* run test */
	enum rpma_conn_event c_event = RPMA_CONN_UNDEFINED;
	int ret = rpma_conn_next_event(cstate->conn, &c_event);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(c_event, RPMA_CONN_UNDEFINED);
}

/*
 * next_event__data_store_E_NOMEM - rpma_private_data_store() fails
 * with RPMA_E_NOMEM
 */
static void
next_event__data_store_E_NOMEM(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_ESTABLISHED;
	will_return(rdma_get_cm_event, &event);
	will_return(rpma_private_data_store, RPMA_E_NOMEM);
	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, MOCK_OK);

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
	will_return(rpma_private_data_store, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, MOCK_OK);

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
	will_return(rpma_private_data_store, MOCK_OK);
	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, MOCK_OK);

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

	/* configure mocks for rpma_conn_transfer_private_data() */
	struct rpma_conn_private_data data;
	data.ptr = MOCK_PRIVATE_DATA;
	data.len = MOCK_PDATA_LEN;

	/* transfer the private data to the connection (a take over) */
	rpma_conn_transfer_private_data(cstate->conn, &data);

	/* verify the source of the private data is zeroed */
	assert_ptr_equal(data.ptr, NULL);
	assert_int_equal(data.len, 0);

	/* get the private data */
	struct rpma_conn_private_data check_data;
	int ret = rpma_conn_get_private_data(cstate->conn, &check_data);

	/* verify the results of rpma_conn_get_private_data() */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(check_data.ptr, MOCK_PRIVATE_DATA);
	assert_int_equal(check_data.len, MOCK_PDATA_LEN);

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

	/* configure mocks for rpma_conn_transfer_private_data() */
	struct rpma_conn_private_data data;
	data.ptr = MOCK_PRIVATE_DATA;
	data.len = MOCK_PDATA_LEN;

	/* transfer the private data to the connection (a take over) */
	rpma_conn_transfer_private_data(cstate->conn, &data);

	/* verify the source of the private data is zeroed */
	assert_ptr_equal(data.ptr, NULL);
	assert_int_equal(data.len, 0);

	/* get the private data */
	struct rpma_conn_private_data check_data;
	int ret = rpma_conn_get_private_data(cstate->conn, &check_data);

	/* verify the results of rpma_conn_get_private_data() */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(check_data.ptr, MOCK_PRIVATE_DATA);
	assert_int_equal(check_data.len, MOCK_PDATA_LEN);

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

/*
 * next_event__success_REJECTED - happy day scenario
 */
static void
next_event__success_REJECTED(void **cstate_ptr)
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
	assert_int_equal(ret, MOCK_OK);
	assert_int_equal(c_event, RPMA_CONN_REJECTED);
}

static const struct CMUnitTest tests_next_event[] = {
	/* rpma_conn_next_event() unit tests */
	cmocka_unit_test(next_event__conn_NULL),
	cmocka_unit_test_setup_teardown(
		next_event__event_NULL,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(next_event__conn_NULL_event_NULL),
	cmocka_unit_test_setup_teardown(
		next_event__get_cm_event_ERRNO,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(
		next_event__get_cm_event_ENODATA,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(
		next_event__event_UNREACHABLE,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(
		next_event__event_UNREACHABLE_ack_ERRNO,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test_setup_teardown(
		next_event__data_store_E_NOMEM,
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
	cmocka_unit_test_setup_teardown(
		next_event__success_REJECTED,
		setup__conn_new, teardown__conn_delete),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_next_event, NULL, NULL);
}
