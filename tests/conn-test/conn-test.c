/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn-test.c -- the connection unit tests
 */

#include "cmocka_headers.h"
#include "conn.h"

#define MOCK_EVCH	(struct rdma_event_channel *)0xE4C4
#define MOCK_CQ		(struct ibv_cq *)0x00C0
#define MOCK_CM_ID	(struct rdma_cm_id *)0xC41D
#define MOCK_EVENT	(enum rpma_conn_event *)(-1)

#define NO_ERROR	0

/*
 * rdma_destroy_qp -- rdma_destroy_qp() mock
 */
void
rdma_destroy_qp(struct rdma_cm_id *id)
{
	check_expected_ptr(id);
}

/*
 * ibv_destroy_cq -- ibv_destroy_cq() mock
 */
int
ibv_destroy_cq(struct ibv_cq *cq)
{
	check_expected_ptr(cq);

	return mock_type(int);
}

/*
 * rdma_destroy_id -- rdma_destroy_id() mock
 */
int
rdma_destroy_id(struct rdma_cm_id *id)
{
	check_expected_ptr(id);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_create_event_channel -- rdma_create_event_channel() mock
 */
struct rdma_event_channel *
rdma_create_event_channel(void)
{
	struct rdma_event_channel *evch =
		mock_type(struct rdma_event_channel *);
	if (!evch) {
		errno = mock_type(int);
		return NULL;
	}

	return evch;
}

/*
 * rdma_destroy_event_channel -- rdma_destroy_event_channel() mock
 */
void
rdma_destroy_event_channel(struct rdma_event_channel *channel)
{
	assert_ptr_equal(channel, MOCK_EVCH);
}

/*
 * Rdma_migrate_id_counter -- counter of calls to rdma_migrate_id() which allows
 * controlling its mock behaviour from call-to-call.
 */
static int Rdma_migrate_id_counter = 0;

#define RDMA_MIGRATE_TO_EVCH 0
#define RDMA_MIGRATE_FROM_EVCH 1
#define RDMA_MIGRATE_COUNTER_INIT (RDMA_MIGRATE_TO_EVCH)

/*
 * rdma_migrate_id -- rdma_migrate_id() mock
 */
int
rdma_migrate_id(struct rdma_cm_id *id, struct rdma_event_channel *channel)
{
	assert_ptr_equal(id, MOCK_CM_ID);

	/*
	 * This mock assumes the first call to rdma_migrate_id() always migrate
	 * a CM ID to an event channel. Whereas the second call migrate the
	 * CM ID from the event channel (channel == NULL).
	 */
	if (Rdma_migrate_id_counter == RDMA_MIGRATE_TO_EVCH)
		assert_ptr_equal(channel, MOCK_EVCH);
	else if (Rdma_migrate_id_counter == RDMA_MIGRATE_FROM_EVCH)
		assert_ptr_equal(channel, NULL);
	else
		assert_true(0);

	++Rdma_migrate_id_counter;

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

void *__real__test_malloc(size_t size);

/*
 * __wrap__test_malloc -- malloc() mock
 */
void *
__wrap__test_malloc(size_t size)
{
	errno = mock_type(int);

	if (errno)
		return NULL;

	return __real__test_malloc(size);
}

/*
 * rdma_get_cm_event -- rdma_get_cm_event() mock
 */
int
rdma_get_cm_event(struct rdma_event_channel *channel,
		struct rdma_cm_event **event_ptr)
{
	check_expected_ptr(channel);
	assert_non_null(event_ptr);

	struct rdma_cm_event *event = mock_type(struct rdma_cm_event *);
	if (!event) {
		errno = mock_type(int);
		return -1;
	}

	*event_ptr = event;
	return 0;
}

/*
 * rdma_ack_cm_event -- rdma_ack_cm_event() mock
 */
int
rdma_ack_cm_event(struct rdma_cm_event *event)
{
	check_expected_ptr(event);
	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * new_test_id_NULL - NULL id is invalid
 */
static void
new_test_id_NULL(void **unused)
{
	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(NULL, MOCK_CQ, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(conn);
}

/*
 * new_test_cq_NULL - NULL cq is invalid
 */
static void
new_test_cq_NULL(void **unused)
{
	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_CM_ID, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(conn);
}

/*
 * new_test_conn_ptr_NULL - NULL conn_ptr is invalid
 */
static void
new_test_conn_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_new(MOCK_CM_ID, MOCK_CQ, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new_test_id_cq_conn_ptr_NULL - NULL id, cq and conn_ptr are
 * invalid
 */
static void
new_test_id_cq_conn_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_new(NULL, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new_test_create_evch_EAGAIN - rdma_create_event_channel() fails with EAGAIN
 */
static void
new_test_create_evch_EAGAIN(void **unused)
{
	/* configure mock */
	will_return(rdma_create_event_channel, NULL);
	will_return(rdma_create_event_channel, EAGAIN);
	will_return_maybe(__wrap__test_malloc, NO_ERROR);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_CM_ID, MOCK_CQ, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(conn);
}

/*
 * new_test_migrate_id_EAGAIN - rdma_migrate_id() fails with EAGAIN
 */
static void
new_test_migrate_id_EAGAIN(void **unused)
{
	/* configure mock */
	will_return(rdma_create_event_channel, MOCK_EVCH);
	Rdma_migrate_id_counter = RDMA_MIGRATE_COUNTER_INIT;
	will_return(rdma_migrate_id, EAGAIN);
	will_return_maybe(__wrap__test_malloc, NO_ERROR);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_CM_ID, MOCK_CQ, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(conn);
}

/*
 * new_test_malloc_ENOMEM - malloc() fails with ENOMEM
 */
static void
new_test_malloc_ENOMEM(void **unused)
{
	/* configure mock */
	will_return(__wrap__test_malloc, ENOMEM);
	will_return_maybe(rdma_create_event_channel, MOCK_EVCH);
	Rdma_migrate_id_counter = RDMA_MIGRATE_COUNTER_INIT;
	will_return_maybe(rdma_migrate_id, NO_ERROR);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_CM_ID, MOCK_CQ, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(conn);
}

/*
 * conn_setup - prepare a valid rpma_conn object
 */
static int
conn_setup(void **conn_ptr)
{
	/* configure mock: */
	will_return(rdma_create_event_channel, MOCK_EVCH);
	Rdma_migrate_id_counter = RDMA_MIGRATE_COUNTER_INIT;
	will_return(rdma_migrate_id, NO_ERROR);
	will_return(__wrap__test_malloc, NO_ERROR);

	/* prepare an object */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_CM_ID, MOCK_CQ, &conn);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
	assert_non_null(conn);

	*conn_ptr = conn;

	return 0;
}

/*
 * conn_teardown - delete the rpma_conn object
 */
static int
conn_teardown(void **conn_ptr)
{
	struct rpma_conn *conn = *conn_ptr;

	/* configure mocks: */
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(ibv_destroy_cq, cq, MOCK_CQ);
	will_return(ibv_destroy_cq, NO_ERROR);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, NO_ERROR);

	/* delete the object */
	int ret = rpma_conn_delete(&conn);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
	assert_null(conn);

	*conn_ptr = NULL;

	return 0;
}

/*
 * conn_test_lifecycle - happy day scenario
 */
static void
conn_test_lifecycle(void **unused)
{
	/* the thing is done by conn_setup() and conn_teardown() */
}

/*
 * delete_test_conn_ptr_NULL - conn_ptr NULL is invalid
 */
static void
delete_test_conn_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_delete(NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * delete_test_conn_NULL - *conn_ptr NULL should cause quick exit
 */
static void
delete_test_conn_NULL(void **unused)
{
	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_delete(&conn);

	/* verify the results */
	assert_int_equal(ret, 0);
}

/*
 * delete_test_destroy_cq_EAGAIN - ibv_destroy_cq() fails with EAGAIN
 */
static void
delete_test_destroy_cq_EAGAIN(void **conn_ptr)
{
	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	struct rpma_conn *conn = NULL;
	int ret = conn_setup((void **)&conn);
	assert_int_equal(ret, 0);
	assert_non_null(conn);

	/* configure mocks: */
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(ibv_destroy_cq, cq, MOCK_CQ);
	will_return(ibv_destroy_cq, EAGAIN);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, NO_ERROR);

	/* run test */
	ret = rpma_conn_delete(&conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(conn);
}

/*
 * delete_test_destroy_cq_EAGAIN_destroy_id_EAGAIN - rdma_destroy_id() fails
 * with EIO when exiting after ibv_destroy_cq() fail (EAGAIN)
 */
static void
delete_test_destroy_cq_EAGAIN_destroy_id_EAGAIN(void **conn_ptr)
{
	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	struct rpma_conn *conn = NULL;
	int ret = conn_setup((void **)&conn);
	assert_int_equal(ret, 0);
	assert_non_null(conn);

	/* configure mocks: */
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(ibv_destroy_cq, cq, MOCK_CQ);
	will_return(ibv_destroy_cq, EAGAIN); /* first error */
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, EIO); /* second error */

	/* run test */
	ret = rpma_conn_delete(&conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	/*
	 * We cannot assume which fails first either ibv_destroy_cq() or
	 * rdma_destroy_id().
	 */
	int provider_error = rpma_err_get_provider_error();
	assert_true(provider_error == EAGAIN || provider_error == EIO);
	assert_null(conn);
}

/*
 * delete_test_destroy_id_EAGAIN - ibv_destroy_cq() fails with EAGAIN
 */
static void
delete_test_destroy_id_EAGAIN(void **conn_ptr)
{
	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	struct rpma_conn *conn = NULL;
	int ret = conn_setup((void **)&conn);
	assert_int_equal(ret, 0);
	assert_non_null(conn);

	/* configure mocks: */
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(ibv_destroy_cq, cq, MOCK_CQ);
	will_return(ibv_destroy_cq, NO_ERROR);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, EAGAIN);

	/* run test */
	ret = rpma_conn_delete(&conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(conn);
}

/*
 * next_event_test_conn_NULL - NULL conn is invalid
 */
static void
next_event_test_conn_NULL(void **unused)
{
	/* run test */
	enum rpma_conn_event *ptr = MOCK_EVENT;
	int ret = rpma_conn_next_event(NULL, ptr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_int_equal(ptr, MOCK_EVENT);
}

/*
 * next_event_test_event_NULL - NULL event is invalid
 */
static void
next_event_test_event_NULL(void **conn_ptr)
{
	struct rpma_conn *conn = *conn_ptr;

	/* run test */
	int ret = rpma_conn_next_event(conn, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * next_event_test_conn_NULL_event_NULL - NULL conn and NULL event are invalid
 */
static void
next_event_test_conn_NULL_event_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_next_event(NULL, NULL);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * next_event_test_get_cm_event_EAGAIN -
 * rdma_get_cm_event() fails with EAGAIN
 */
static void
next_event_test_get_cm_event_EAGAIN(void **conn_ptr)
{
	struct rpma_conn *conn = *conn_ptr;

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	will_return(rdma_get_cm_event, NULL);
	will_return(rdma_get_cm_event, EAGAIN);

	/* run test */
	enum rpma_conn_event *ptr = MOCK_EVENT;
	int ret = rpma_conn_next_event(conn, ptr);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_int_equal(ptr, MOCK_EVENT);
}

/*
 * next_event_test_event_REJECTED -
 * RDMA_CM_EVENT_REJECTED is unexpected
 */
static void
next_event_test_event_REJECTED(void **conn_ptr)
{
	struct rpma_conn *conn = *conn_ptr;

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_REJECTED;
	will_return(rdma_get_cm_event, &event);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, NO_ERROR);

	/* run test */
	enum rpma_conn_event ptr;
	int ret = rpma_conn_next_event(conn, &ptr);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_UNKNOWN);
}

/*
 * next_event_test_event_REJECTED_ack_EINVAL -
 * rdma_ack_cm_event() fails with EINVAL after obtaining
 * an RDMA_CM_EVENT_REJECTED event
 */
static void
next_event_test_event_REJECTED_ack_EINVAL(void **conn_ptr)
{
	struct rpma_conn *conn = *conn_ptr;

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_REJECTED;
	will_return(rdma_get_cm_event, &event);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, EINVAL);

	/* run test */
	enum rpma_conn_event ptr;
	int ret = rpma_conn_next_event(conn, &ptr);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_UNKNOWN);
}

/*
 * next_event_test_success_ESTABLISHED - happy day scenario
 */
static void
next_event_test_success_ESTABLISHED(void **conn_ptr)
{
	struct rpma_conn *conn = *conn_ptr;

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_ESTABLISHED;
	will_return(rdma_get_cm_event, &event);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, NO_ERROR);

	/* run test */
	enum rpma_conn_event ptr;
	int ret = rpma_conn_next_event(conn, &ptr);

	/* verify the results */
	assert_int_equal(ptr, RPMA_CONN_ESTABLISHED);
	assert_int_equal(ret, 0);
}

/*
 * next_event_test_success_LOST - happy day scenario
 */
static void
next_event_test_success_LOST(void **conn_ptr)
{
	struct rpma_conn *conn = *conn_ptr;

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_CONNECT_ERROR;
	will_return(rdma_get_cm_event, &event);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, NO_ERROR);

	/* run test */
	enum rpma_conn_event ptr;
	int ret = rpma_conn_next_event(conn, &ptr);

	/* verify the results */
	assert_int_equal(ptr, RPMA_CONN_LOST);
	assert_int_equal(ret, 0);
}

/*
 * next_event_test_success_CLOSED - happy day scenario
 */
static void
next_event_test_success_CLOSED(void **conn_ptr)
{
	struct rpma_conn *conn = *conn_ptr;

	expect_value(rdma_get_cm_event, channel, MOCK_EVCH);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_DISCONNECTED;
	will_return(rdma_get_cm_event, &event);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, NO_ERROR);

	/* run test */
	enum rpma_conn_event ptr;
	int ret = rpma_conn_next_event(conn, &ptr);

	/* verify the results */
	assert_int_equal(ptr, RPMA_CONN_CLOSED);
	assert_int_equal(ret, 0);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_conn_new() unit tests */
		cmocka_unit_test(new_test_id_NULL),
		cmocka_unit_test(new_test_cq_NULL),
		cmocka_unit_test(new_test_conn_ptr_NULL),
		cmocka_unit_test(new_test_id_cq_conn_ptr_NULL),
		cmocka_unit_test(new_test_create_evch_EAGAIN),
		cmocka_unit_test(new_test_migrate_id_EAGAIN),
		cmocka_unit_test(new_test_malloc_ENOMEM),

		/* rpma_conn_new()/_delete() lifecycle */
		cmocka_unit_test_setup_teardown(conn_test_lifecycle,
			conn_setup, conn_teardown),

		/* rpma_conn_delete() unit tests */
		cmocka_unit_test(delete_test_conn_ptr_NULL),
		cmocka_unit_test(delete_test_conn_NULL),
		cmocka_unit_test(delete_test_destroy_cq_EAGAIN),
		cmocka_unit_test(
			delete_test_destroy_cq_EAGAIN_destroy_id_EAGAIN),
		cmocka_unit_test(delete_test_destroy_id_EAGAIN),

		/* rpma_conn_next_event() unit tests */
		cmocka_unit_test(next_event_test_conn_NULL),
		cmocka_unit_test_setup_teardown(
			next_event_test_event_NULL,
			conn_setup, conn_teardown),
		cmocka_unit_test(next_event_test_conn_NULL_event_NULL),
		cmocka_unit_test_setup_teardown(
			next_event_test_get_cm_event_EAGAIN,
			conn_setup, conn_teardown),
		cmocka_unit_test_setup_teardown(
			next_event_test_event_REJECTED,
			conn_setup, conn_teardown),
		cmocka_unit_test_setup_teardown(
			next_event_test_event_REJECTED_ack_EINVAL,
			conn_setup, conn_teardown),
		cmocka_unit_test_setup_teardown(
			next_event_test_success_ESTABLISHED,
			conn_setup, conn_teardown),
		cmocka_unit_test_setup_teardown(
			next_event_test_success_LOST,
			conn_setup, conn_teardown),
		cmocka_unit_test_setup_teardown(
			next_event_test_success_CLOSED,
			conn_setup, conn_teardown),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
