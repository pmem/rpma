/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn-test-common.c -- the connection unit tests common functions
 */

#include "conn-test-common.h"
#include "mocks-ibverbs.h"

struct rdma_cm_id Cm_id;	/* mock CM ID */
struct rdma_event_channel Evch;	/* mock event channel */

const char Private_data[] = "Random data";
const char Private_data_2[] = "Another random data";

/*
 * rdma_destroy_qp -- rdma_destroy_qp() mock
 */
void
rdma_destroy_qp(struct rdma_cm_id *id)
{
	check_expected_ptr(id);
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
int Rdma_migrate_id_counter = 0;

/*
 * rdma_migrate_id -- rdma_migrate_id() mock
 */
int
rdma_migrate_id(struct rdma_cm_id *id, struct rdma_event_channel *channel)
{
	assert_ptr_equal(id, MOCK_CM_ID);

	/*
	 * This mock assumes the first call to rdma_migrate_id() always migrate
	 * a CM ID to an event channel. Whereas the second call migrate
	 * the CM ID from the event channel (channel == NULL).
	 */
	if (Rdma_migrate_id_counter == RDMA_MIGRATE_TO_EVCH)
		assert_ptr_equal(channel, MOCK_EVCH);
	else if (Rdma_migrate_id_counter == RDMA_MIGRATE_FROM_EVCH)
		assert_ptr_equal(channel, NULL);
	else
		assert_true(0);

	++Rdma_migrate_id_counter;
	id->qp = MOCK_QP;

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
	const LargestIntegralType allowed_events[] = {
			RDMA_CM_EVENT_CONNECT_REQUEST,
			RDMA_CM_EVENT_ESTABLISHED};
	assert_non_null(edata);
	assert_in_set(edata->event, allowed_events,
		sizeof(allowed_events) / sizeof(allowed_events[0]));
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
 * rpma_mr_read -- rpma_mr_read() mock
 */
int
rpma_mr_read(struct ibv_qp *qp,
	struct rpma_mr_local *dst, size_t dst_offset,
	struct rpma_mr_remote *src,  size_t src_offset,
	size_t len, int flags, void *op_context)
{
	assert_non_null(qp);
	assert_non_null(dst);
	assert_non_null(src);
	assert_int_not_equal(flags, 0);

	check_expected_ptr(qp);
	check_expected_ptr(dst);
	check_expected(dst_offset);
	check_expected_ptr(src);
	check_expected(src_offset);
	check_expected(len);
	check_expected(flags);
	check_expected_ptr(op_context);

	return mock_type(int);
}

/*
 * rpma_mr_write -- rpma_mr_write() mock
 */
int
rpma_mr_write(struct ibv_qp *qp,
	struct rpma_mr_remote *dst, size_t dst_offset,
	struct rpma_mr_local *src,  size_t src_offset,
	size_t len, int flags, void *op_context)
{
	assert_non_null(qp);
	assert_non_null(dst);
	assert_non_null(src);
	assert_int_not_equal(flags, 0);

	check_expected_ptr(qp);
	check_expected_ptr(dst);
	check_expected(dst_offset);
	check_expected_ptr(src);
	check_expected(src_offset);
	check_expected(len);
	check_expected(flags);
	check_expected_ptr(op_context);

	return mock_type(int);
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
 * conn_setup - prepare a valid rpma_conn object
 */
int
conn_setup(void **cstate_ptr)
{
	static struct conn_test_state cstate;
	cstate.conn = NULL;
	cstate.data.ptr = NULL;
	cstate.data.len = 0;

	Ibv_cq.channel = MOCK_COMP_CHANNEL;

	/* configure mock: */
	will_return(rdma_create_event_channel, MOCK_EVCH);
	Rdma_migrate_id_counter = RDMA_MIGRATE_COUNTER_INIT;
	will_return(rdma_migrate_id, MOCK_OK);
	will_return(rpma_flush_new, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_OK);

	/* prepare an object */
	int ret = rpma_conn_new(MOCK_PEER, MOCK_CM_ID,
			MOCK_IBV_CQ, &cstate.conn);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(cstate.conn);

	*cstate_ptr = &cstate;

	return 0;
}

/*
 * conn_teardown - delete the rpma_conn object
 */
int
conn_teardown(void **cstate_ptr)
{
	struct conn_test_state *cstate = *cstate_ptr;

	/* configure mocks: */
	will_return(rpma_flush_delete, MOCK_OK);
	expect_value(rdma_destroy_qp, id, MOCK_CM_ID);
	will_return(ibv_destroy_cq, MOCK_OK);
	will_return(ibv_destroy_comp_channel, MOCK_OK);
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
