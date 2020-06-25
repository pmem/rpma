/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn-test.c -- the connection unit tests
 */

#include "cmocka_headers.h"
#include "conn.h"

#define MOCK_EVCH		(struct rdma_event_channel *)0xE4C4
#define MOCK_CQ			(struct ibv_cq *)0x00C0
#define MOCK_CM_ID		(struct rdma_cm_id *)0xC41D
#define MOCK_CONN		(struct rpma_conn *)0xA41F
#define MOCK_PRIVATE_DATA	((void *)"Random data")
#define MOCK_PDATA_LEN		(strlen(MOCK_PRIVATE_DATA) + 1)
#define MOCK_PRIVATE_DATA_2	((void *)"Another random data")
#define MOCK_PDATA_LEN_2	(strlen(MOCK_PRIVATE_DATA_2) + 1)

#define MOCK_OK			0

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

/*
 * rpma_private_data_store -- rpma_private_data_store() mock
 */
int
rpma_private_data_store(struct rdma_cm_event *edata,
		struct rpma_conn_private_data *pdata)
{
	assert_non_null(edata);
	assert_true(edata->event == RDMA_CM_EVENT_CONNECT_REQUEST ||
			edata->event == RDMA_CM_EVENT_ESTABLISHED);
	assert_non_null(pdata);
	assert_null(pdata->ptr);
	assert_int_equal(pdata->len, 0);

	int ret = mock_type(int);
	if (ret)
		return ret;

	pdata->ptr = (void *)edata->param.conn.private_data;
	pdata->len = edata->param.conn.private_data_len;

	return 0;
}

/*
 * rpma_private_data_copy -- rpma_private_data_copy() mock
 */
int
rpma_private_data_copy(struct rpma_conn_private_data *dst,
		struct rpma_conn_private_data *src)
{
	assert_non_null(src);
	assert_non_null(dst);
	assert_null(dst->ptr);
	assert_int_equal(dst->len, 0);
	assert_true((src->ptr == NULL && src->len == 0) ||
			(src->ptr != NULL && src->len != 0));

	dst->len = 0;

	int ret = mock_type(int);
	if (ret) {
		dst->ptr = NULL;
		return ret;
	}

	dst->ptr = mock_type(void *);
	if (dst->ptr)
		dst->len = strlen(dst->ptr) + 1;

	return 0;
}

/*
 * rpma_private_data_discard -- rpma_private_data_discard() mock
 */
void
rpma_private_data_discard(struct rpma_conn_private_data *pdata)
{
	assert_non_null(pdata);
	check_expected(pdata->ptr);
	check_expected(pdata->len);
	pdata->ptr = NULL;
	pdata->len = 0;
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
 * rdma_disconnect -- rdma_disconnect() mock
 */
int
rdma_disconnect(struct rdma_cm_id *id)
{
	check_expected_ptr(id);
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
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

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
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

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
	will_return_maybe(rdma_migrate_id, MOCK_OK);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_new(MOCK_CM_ID, MOCK_CQ, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(conn);
}

/*
 * get_private_data_test_conn_NULL - NULL conn is invalid
 */
static void
get_private_data_test_conn_NULL(void **unused)
{
	/* get private data */
	struct rpma_conn_private_data data = {0};
	int ret = rpma_conn_get_private_data(NULL, &data);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_ptr_equal(data.ptr, NULL);
	assert_int_equal(data.len, 0);
}

/*
 * get_private_data_test_pdata_NULL - NULL pdata is invalid
 */
static void
get_private_data_test_pdata_NULL(void **unused)
{
	/* get private data */
	struct rpma_conn *conn = MOCK_CONN;
	int ret = rpma_conn_get_private_data(conn, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_private_data_test_conn_NULL_pdata_NULL -
 * (conn == NULL && pdata == NULL) is invalid
 */
static void
get_private_data_test_conn_NULL_pdata_NULL(void **unused)
{
	/* get private data */
	int ret = rpma_conn_get_private_data(NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/* all the resources used between conn_setup and conn_teardown */
struct conn_test_state {
	struct rpma_conn *conn;
	struct rpma_conn_private_data data;
};

/*
 * conn_setup - prepare a valid rpma_conn object
 */
static int
conn_setup(void **cstate_ptr)
{
	static struct conn_test_state cstate;
	cstate.conn = NULL;
	cstate.data.ptr = NULL;
	cstate.data.len = 0;

	/* configure mock: */
	will_return(rdma_create_event_channel, MOCK_EVCH);
	Rdma_migrate_id_counter = RDMA_MIGRATE_COUNTER_INIT;
	will_return(rdma_migrate_id, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_OK);

	/* prepare an object */
	int ret = rpma_conn_new(MOCK_CM_ID, MOCK_CQ, &cstate.conn);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(cstate.conn);

	*cstate_ptr = &cstate;

	return 0;
}

/*
 * conn_teardown - delete the rpma_conn object
 */
static int
conn_teardown(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks: */
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(ibv_destroy_cq, cq, MOCK_CQ);
	will_return(ibv_destroy_cq, MOCK_OK);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, MOCK_OK);
	expect_value(rpma_private_data_discard, pdata->ptr, cstate->data.ptr);
	expect_value(rpma_private_data_discard, pdata->len, cstate->data.len);

	/* delete the object */
	int ret = rpma_conn_delete(&cstate->conn);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(cstate->conn);

	*cstate_ptr = NULL;

	return 0;
}

/*
 * conn_test_lifecycle - happy day scenario
 */
static void
conn_test_lifecycle(void **cstate_ptr)
{
	/* main things are done by conn_setup() and conn_teardown() */

	struct conn_test_state *cstate = *cstate_ptr;

	/* get private data */
	struct rpma_conn_private_data data;
	int ret = rpma_conn_get_private_data(cstate->conn, &data);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(data.ptr, cstate->data.ptr);
	assert_int_equal(data.len, cstate->data.len);
}

/*
 * set_private_data_test_failed_ENOMEM - rpma_conn_set_private_data() failed
 *                                       with RPMA_E_NOMEM
 */
static void
set_private_data_test_failed_ENOMEM(void **cstate_ptr)
{
	/*
	 * Common things are done by conn_setup()
	 * and conn_teardown().
	 */
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks for rpma_conn_set_private_data() */
	struct rpma_conn_private_data data;
	data.ptr = MOCK_PRIVATE_DATA;
	data.len = MOCK_PDATA_LEN;
	will_return(rpma_private_data_copy, RPMA_E_NOMEM);

	/* set private data */
	int ret = rpma_conn_set_private_data(cstate->conn, &data);

	/* verify the results of rpma_conn_set_private_data() */
	assert_int_equal(ret, RPMA_E_NOMEM);

	/* get private data */
	struct rpma_conn_private_data check_data;
	ret = rpma_conn_get_private_data(cstate->conn, &check_data);

	/* verify the results of rpma_conn_get_private_data() */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(check_data.ptr, NULL);
	assert_int_equal(check_data.len, 0);
}

/*
 * set_private_data_test_success - rpma_conn_set_private_data() succeeds
 */
static void
set_private_data_test_success(void **cstate_ptr)
{
	/*
	 * Common things are done by conn_setup()
	 * and conn_teardown().
	 */
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks for rpma_conn_set_private_data() */
	cstate->data.ptr = MOCK_PRIVATE_DATA;
	cstate->data.len = MOCK_PDATA_LEN;
	will_return(rpma_private_data_copy, 0);
	will_return(rpma_private_data_copy, MOCK_PRIVATE_DATA);

	/* set private data */
	int ret = rpma_conn_set_private_data(cstate->conn, &cstate->data);

	/* verify the results of rpma_conn_set_private_data() */
	assert_int_equal(ret, MOCK_OK);

	/* get private data */
	struct rpma_conn_private_data check_data;
	ret = rpma_conn_get_private_data(cstate->conn, &check_data);

	/* verify the results of rpma_conn_get_private_data() */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(check_data.ptr, cstate->data.ptr);
	assert_int_equal(check_data.len, cstate->data.len);
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
delete_test_destroy_cq_EAGAIN(void **unused)
{
	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	struct conn_test_state *cstate;
	int ret = conn_setup((void **)&cstate);
	assert_int_equal(ret, 0);
	assert_non_null(cstate->conn);

	/* configure mocks: */
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(ibv_destroy_cq, cq, MOCK_CQ);
	will_return(ibv_destroy_cq, EAGAIN);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	ret = rpma_conn_delete(&cstate->conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->conn);
}

/*
 * delete_test_destroy_cq_EAGAIN_destroy_id_EAGAIN - rdma_destroy_id() fails
 * with EIO when exiting after ibv_destroy_cq() fail (EAGAIN)
 */
static void
delete_test_destroy_cq_EAGAIN_destroy_id_EAGAIN(void **unused)
{
	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	struct conn_test_state *cstate;
	int ret = conn_setup((void **)&cstate);
	assert_int_equal(ret, 0);
	assert_non_null(cstate->conn);

	/* configure mocks: */
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(ibv_destroy_cq, cq, MOCK_CQ);
	will_return(ibv_destroy_cq, EAGAIN); /* first error */
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, EIO); /* second error */

	/* run test */
	ret = rpma_conn_delete(&cstate->conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	/*
	 * We cannot assume which fails first either ibv_destroy_cq() or
	 * rdma_destroy_id().
	 */
	int provider_error = rpma_err_get_provider_error();
	assert_true(provider_error == EAGAIN || provider_error == EIO);
	assert_null(cstate->conn);
}

/*
 * delete_test_destroy_id_EAGAIN - ibv_destroy_cq() fails with EAGAIN
 */
static void
delete_test_destroy_id_EAGAIN(void **unused)
{
	/*
	 * Cmocka does not allow freeing an object in a test if the object was
	 * created in the setup step whereas even failing rpma_conn_delete()
	 * will deallocate the rpma_conn object.
	 */
	struct conn_test_state *cstate;
	int ret = conn_setup((void **)&cstate);
	assert_int_equal(ret, 0);
	assert_non_null(cstate->conn);

	/* configure mocks: */
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	expect_value(ibv_destroy_cq, cq, MOCK_CQ);
	will_return(ibv_destroy_cq, MOCK_OK);
	expect_value(rdma_destroy_id, id, MOCK_CM_ID);
	will_return(rdma_destroy_id, EAGAIN);

	/* run test */
	ret = rpma_conn_delete(&cstate->conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->conn);
}

/*
 * next_event_test_conn_NULL - NULL conn is invalid
 */
static void
next_event_test_conn_NULL(void **unused)
{
	/* run test */
	enum rpma_conn_event c_event = RPMA_CONN_UNDEFINED;
	int ret = rpma_conn_next_event(NULL, &c_event);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_int_equal(c_event, RPMA_CONN_UNDEFINED);
}

/*
 * next_event_test_event_NULL - NULL event is invalid
 */
static void
next_event_test_event_NULL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_next_event(cstate->conn, NULL);

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
next_event_test_get_cm_event_EAGAIN(void **cstate_ptr)
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
 * next_event_test_event_REJECTED -
 * RDMA_CM_EVENT_REJECTED is unexpected
 */
static void
next_event_test_event_REJECTED(void **cstate_ptr)
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
 * next_event_test_event_REJECTED_ack_EINVAL -
 * rdma_ack_cm_event() fails with EINVAL after obtaining
 * an RDMA_CM_EVENT_REJECTED event
 */
static void
next_event_test_event_REJECTED_ack_EINVAL(void **cstate_ptr)
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
 * next_event_test_data_store_ENOMEM - rpma_private_data_store fails
 * with RPMA_E_NOMEM
 */
static void
next_event_test_data_store_ENOMEM(void **cstate_ptr)
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
 * next_event_test_success_no_data_ESTABLISHED_no_data - happy day scenario,
 * no private data in the connection and no private data in the event
 */
static void
next_event_test_success_no_data_ESTABLISHED_no_data(void **cstate_ptr)
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
 * next_event_test_success_no_data_ESTABLISHED_with_data - happy day scenario
 * no private data in the connection and with private data in the event
 */
static void
next_event_test_success_no_data_ESTABLISHED_with_data(void **cstate_ptr)
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
 * next_event_test_success_with_data_ESTABLISHED_no_data - happy day scenario,
 * with private data in the connection and with no private data in the event
 */
static void
next_event_test_success_with_data_ESTABLISHED_no_data(void **cstate_ptr)
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
 * next_event_test_success_with_data_ESTABLISHED_with_data - happy day scenario,
 * with private data in the connection and with another private data
 * in the event
 */
static void
next_event_test_success_with_data_ESTABLISHED_with_data(void **cstate_ptr)
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
 * next_event_test_success_CONNECT_ERROR - happy day scenario
 */
static void
next_event_test_success_CONNECT_ERROR(void **cstate_ptr)
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
 * next_event_test_success_DEVICE_REMOVAL - happy day scenario
 */
static void
next_event_test_success_DEVICE_REMOVAL(void **cstate_ptr)
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
 * next_event_test_success_DISCONNECTED - happy day scenario
 */
static void
next_event_test_success_DISCONNECTED(void **cstate_ptr)
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
 * next_event_test_success_TIMEWAIT_EXIT - happy day scenario
 */
static void
next_event_test_success_TIMEWAIT_EXIT(void **cstate_ptr)
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
 * disconnect_test_conn_NULL - NULL conn is invalid
 */
static void
disconnect_test_conn_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_disconnect(NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * disconnect_test_rdma_disconnect_EINVAL -
 * rdma_disconnect() fails with EINVAL
 */
static void
disconnect_test_rdma_disconnect_EINVAL(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_disconnect, id, MOCK_CM_ID);
	will_return(rdma_disconnect, EINVAL);

	/* run test */
	int ret = rpma_conn_disconnect(cstate->conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(errno, EINVAL);
}

/*
 * disconnect_test_success - happy day scenario
 */
static void
disconnect_test_success(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	expect_value(rdma_disconnect, id, MOCK_CM_ID);
	will_return(rdma_disconnect, MOCK_OK);

	/* run test */
	int ret = rpma_conn_disconnect(cstate->conn);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
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
			next_event_test_data_store_ENOMEM,
			conn_setup, conn_teardown),
		cmocka_unit_test_setup_teardown(
			next_event_test_success_no_data_ESTABLISHED_no_data,
			conn_setup, conn_teardown),
		cmocka_unit_test_setup_teardown(
			next_event_test_success_no_data_ESTABLISHED_with_data,
			conn_setup, conn_teardown),
		cmocka_unit_test_setup_teardown(
			next_event_test_success_with_data_ESTABLISHED_no_data,
			conn_setup, conn_teardown),
		cmocka_unit_test_setup_teardown(
			next_event_test_success_with_data_ESTABLISHED_with_data,
			conn_setup, conn_teardown),
		cmocka_unit_test_setup_teardown(
			next_event_test_success_CONNECT_ERROR,
			conn_setup, conn_teardown),
		cmocka_unit_test_setup_teardown(
			next_event_test_success_DEVICE_REMOVAL,
			conn_setup, conn_teardown),
		cmocka_unit_test_setup_teardown(
			next_event_test_success_DISCONNECTED,
			conn_setup, conn_teardown),
		cmocka_unit_test_setup_teardown(
			next_event_test_success_TIMEWAIT_EXIT,
			conn_setup, conn_teardown),

		/* rpma_conn_disconnect() unit tests */
		cmocka_unit_test(disconnect_test_conn_NULL),
		cmocka_unit_test_setup_teardown(
			disconnect_test_rdma_disconnect_EINVAL,
			conn_setup, conn_teardown),
		cmocka_unit_test_setup_teardown(
			disconnect_test_success,
			conn_setup, conn_teardown),

		/* rpma_conn_set_private_data() unit tests */
		cmocka_unit_test_setup_teardown(
			set_private_data_test_failed_ENOMEM,
			conn_setup, conn_teardown),
		cmocka_unit_test_setup_teardown(
			set_private_data_test_success,
			conn_setup, conn_teardown),

		/* rpma_conn_get_private_data() unit tests */
		cmocka_unit_test(get_private_data_test_conn_NULL),
		cmocka_unit_test(get_private_data_test_pdata_NULL),
		cmocka_unit_test(get_private_data_test_conn_NULL_pdata_NULL),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
