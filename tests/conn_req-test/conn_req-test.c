/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * conn_req-test.c -- a connection request unit test
 */

#include <infiniband/verbs.h>

#include "cmocka_headers.h"
#include "conn_req.h"
#include "rpma_err.h"

/* random values */
#define MOCK_VERBS	(struct ibv_context *)0x4E4B
#define MOCK_CQ		(struct ibv_cq *)0x00C0
#define MOCK_PEER	(struct rpma_peer *)0xFEEF

#define CM_EVENT_CONNECTION_REQUEST_INIT \
	{NULL, NULL, RDMA_CM_EVENT_CONNECT_REQUEST, 0, {{0}}}

#define CM_EVENT_CONNECT_ERROR_INIT \
	{NULL, NULL, RDMA_CM_EVENT_CONNECT_ERROR, 0, {{0}}}

#define NO_ERROR 0

/* mocks */

/*
 * rpma_conn_new -- rpma_conn_new()  mock
 */
int
rpma_conn_new(struct rdma_cm_id *id, struct rdma_event_channel *evch,
		struct ibv_cq *cq, struct rpma_conn **conn_ptr)
{
	check_expected_ptr(id);
	check_expected_ptr(evch);
	check_expected_ptr(cq);

	assert_non_null(conn_ptr);

	struct rpma_conn *conn = mock_type(struct rpma_conn *);
	if (!conn) {
		return mock_type(int);
	}

	*conn_ptr = conn;
	return 0;
}

/*
 * rpma_conn_delete -- rpma_conn_delete()  mock
 */
int
rpma_conn_delete(struct rpma_conn **conn_ptr)
{
	assert_non_null(conn_ptr);

	struct rpma_conn *conn = *conn_ptr;
	check_expected_ptr(conn);

	int result = mock_type(int);
	if (result)
		return result;

	*conn_ptr = NULL;
	return 0;
}

/*
 * ibv_create_cq -- ibv_create_cq() mock
 */
struct ibv_cq *
ibv_create_cq(struct ibv_context *context, int cqe, void *cq_context,
		struct ibv_comp_channel *channel, int comp_vector)
{
	assert_ptr_equal(context, MOCK_VERBS);
	assert_int_equal(cqe, RPMA_DEFAULT_Q_SIZE);
	assert_null(channel);
	assert_int_equal(comp_vector, 0);

	struct ibv_cq *cq = mock_type(struct ibv_cq *);
	if (!cq) {
		errno = mock_type(int);
		return NULL;
	}

	return cq;
}

/*
 * ibv_destroy_cq -- ibv_destroy_cq() mock
 */
int
ibv_destroy_cq(struct ibv_cq *cq)
{
	assert_int_equal(cq, MOCK_CQ);

	return mock_type(int);
}

/*
 * rpma_peer_create_qp -- rpma_peer_create_qp() mock
 */
int
rpma_peer_create_qp(struct rpma_peer *peer, struct rdma_cm_id *id,
		struct ibv_cq *cq)
{
	assert_ptr_equal(peer, MOCK_PEER);
	check_expected_ptr(id);
	assert_ptr_equal(cq, MOCK_CQ);

	int result = mock_type(int);
	if (result == RPMA_E_PROVIDER)
		Rpma_provider_error = mock_type(int);

	return result;
}

/*
 * rdma_destroy_qp -- rdma_destroy_qp() mock
 */
void
rdma_destroy_qp(struct rdma_cm_id *id)
{
	check_expected_ptr(id);
}

/*
 * rdma_reject -- rdma_reject() mock
 */
int
rdma_reject(struct rdma_cm_id *id, const void *private_data,
		uint8_t private_data_len)
{
	check_expected_ptr(id);
	assert_null(private_data);
	assert_int_equal(private_data_len, 0);

	errno = mock_type(int);
	if (errno)
		return -1;

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

/* tests */

/*
 * from_cm_event_test_peer_NULL -- NULL peer is invalid
 */
static void
from_cm_event_test_peer_NULL(void **unused)
{
	/* run test */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(NULL, &event, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * from_cm_event_test_edata_NULL -- NULL edata is invalid
 */
static void
from_cm_event_test_edata_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, NULL, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * from_cm_event_test_req_ptr_NULL -- NULL req_ptr is invalid
 */
static void
from_cm_event_test_req_ptr_NULL(void **unused)
{
	/* run test */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * from_cm_event_test_peer_NULL_edata_NULL_req_ptr_NULL -- NULL peer,
 * NULL edata and NULL req_ptr are not valid
 */
static void
from_cm_event_test_peer_NULL_edata_NULL_req_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_req_from_cm_event(NULL, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * from_cm_event_test_RDMA_CM_EVENT_CONNECT_ERROR -- edata of type
 * RDMA_CM_EVENT_CONNECT_ERROR
 */
static void
from_cm_event_test_RDMA_CM_EVENT_CONNECT_ERROR(void **unused)
{
	/* run test */
	struct rdma_cm_event event = CM_EVENT_CONNECT_ERROR_INIT;
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * from_cm_event_test_create_cq_EAGAIN -- ibv_create_cq() fails with EAGAIN
 */
static void
from_cm_event_test_create_cq_EAGAIN(void **unused)
{
	/* configure mocks */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;
	event.id = &id;
	will_return(ibv_create_cq, NULL);
	will_return(ibv_create_cq, EAGAIN);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * from_cm_event_test_peer_create_qp_E_PROVIDER_EAGAIN -- rpma_peer_create_qp()
 * fails with RPMA_E_PROVIDER+EAGAIN
 */
static void
from_cm_event_test_peer_create_qp_E_PROVIDER_EAGAIN(void **unused)
{
	/* configure mocks */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;
	event.id = &id;
	will_return(ibv_create_cq, MOCK_CQ);
	expect_value(rpma_peer_create_qp, id, &id);
	will_return(rpma_peer_create_qp, RPMA_E_PROVIDER);
	will_return(rpma_peer_create_qp, EAGAIN);
	will_return(ibv_destroy_cq, NO_ERROR);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * from_cm_event_test_create_qp_EAGAIN_destroy_cq_EIO -- rpma_peer_create_qp()
 * fail with RPMA_E_PROVIDER+EAGAIN followed by ibv_destroy_cq() fail with EIO
 */
static void
from_cm_event_test_create_qp_EAGAIN_destroy_cq_EIO(
		void **unused)
{
	/* configure mocks */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;
	event.id = &id;
	will_return(ibv_create_cq, MOCK_CQ);
	expect_value(rpma_peer_create_qp, id, &id);
	will_return(rpma_peer_create_qp, RPMA_E_PROVIDER); /* first error */
	will_return(rpma_peer_create_qp, EAGAIN);
	will_return(ibv_destroy_cq, EIO); /* second error */

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * from_cm_event_test_malloc_ENOMEM -- malloc() fails with ENOMEM
 */
static void
from_cm_event_test_malloc_ENOMEM(void **unused)
{
	/* configure mocks */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;
	event.id = &id;
	will_return(ibv_create_cq, MOCK_CQ);
	expect_value(rpma_peer_create_qp, id, &id);
	will_return(rpma_peer_create_qp, NO_ERROR);
	will_return(__wrap__test_malloc, ENOMEM);
	expect_value(rdma_destroy_qp, id, &id);
	will_return(ibv_destroy_cq, NO_ERROR);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * from_cm_event_test_malloc_ENOMEM_destroy_cq_EAGAIN -- malloc() fail with
 * ENOMEM followed by ibv_destroy_cq() fail with EAGAIN
 */
static void
from_cm_event_test_malloc_ENOMEM_destroy_cq_EAGAIN(void **unused)
{
	/* configure mocks */
	struct rdma_cm_event event = CM_EVENT_CONNECTION_REQUEST_INIT;
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;
	event.id = &id;
	will_return(ibv_create_cq, MOCK_CQ);
	expect_value(rpma_peer_create_qp, id, &id);
	will_return(rpma_peer_create_qp, NO_ERROR);
	will_return(__wrap__test_malloc, ENOMEM); /* first error */
	expect_value(rdma_destroy_qp, id, &id);
	will_return(ibv_destroy_cq, EAGAIN); /* second error */

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * all the resources used between conn_req_setup and conn_req_teardown
 */
struct conn_req_test_state {
	struct rdma_cm_event event;
	struct rdma_cm_id id;
	struct rpma_conn_req *req;
};

/*
 * conn_req_from_cm_event_setup -- prepare a valid rpma_conn_req object from CM
 * event
 */
static int
conn_req_from_cm_event_setup(void **cstate_ptr)
{
	static struct conn_req_test_state cstate = {0};
	memset(&cstate, 0, sizeof(cstate));
	cstate.event.event = RDMA_CM_EVENT_CONNECT_REQUEST;
	cstate.event.id = &cstate.id;
	cstate.id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(ibv_create_cq, MOCK_CQ);
	expect_value(rpma_peer_create_qp, id, &cstate.id);
	will_return(rpma_peer_create_qp, NO_ERROR);
	will_return(__wrap__test_malloc, NO_ERROR);

	/* run test */
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &cstate.event,
		&cstate.req);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
	assert_non_null(cstate.req);

	*cstate_ptr = &cstate;

	return 0;
}

/*
 * conn_req_from_cm_event_teardown -- delete the rpma_conn_req object created
 * from a CM event
 */
static int
conn_req_from_cm_event_teardown(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, NO_ERROR);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, NO_ERROR);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, NO_ERROR);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
	assert_null(cstate->req);

	*cstate_ptr = NULL;

	return 0;
}

/*
 * conn_req_from_cm_test_lifecycle - happy day scenario
 */
static void
conn_req_from_cm_test_lifecycle(void **unused)
{
	/*
	 * the thing is done by conn_req_from_cm_event_setup() and
	 * conn_req_teardown()
	 */
}

/*
 * delete_test_req_ptr_NULL -- NULL req_ptr is invalid
 */
static void
delete_test_req_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_req_delete(NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * delete_test_req_NULL -- NULL req is valid - quick exit
 */
static void
delete_test_req_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_delete(&req);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
}

/*
 * delete_test_destroy_cq_EAGAIN - ibv_destroy_cq() fails with EAGAIN
 */
static void
delete_test_destroy_cq_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, EAGAIN);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, NO_ERROR);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, NO_ERROR);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
}

/*
 * delete_test_destroy_cq_EAGAIN_reject_EIO_ack_EINTR - rdma_ack_cm_event()
 * fails with EINTR after rdma_reject() failed with EIO after ibv_destroy_cq()
 * failed with EAGAIN
 */
static void
delete_test_destroy_cq_EAGAIN_reject_EIO_ack_EINTR(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, EAGAIN);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, EIO);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, EINTR);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
}

/*
 * delete_test_reject_EAGAIN - rdma_reject() fails with EAGAIN
 */
static void
delete_test_reject_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, NO_ERROR);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, EAGAIN);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, NO_ERROR);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
}

/*
 * delete_test_reject_EAGAIN_ack_EIO - rdma_ack_cm_event() fails with EIO after
 * rdma_reject() failed with EAGAIN
 */
static void
delete_test_reject_EAGAIN_ack_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, NO_ERROR);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, EAGAIN);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, EIO);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
}

/*
 * delete_test_ack_EAGAIN - rdma_ack_cm_event() fails with EAGAIN
 */
static void
delete_test_ack_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, NO_ERROR);
	expect_value(rdma_reject, id, &cstate->id);
	will_return(rdma_reject, NO_ERROR);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, EAGAIN);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_conn_req_from_cm_event() unit tests */
		cmocka_unit_test(from_cm_event_test_peer_NULL),
		cmocka_unit_test(from_cm_event_test_edata_NULL),
		cmocka_unit_test(from_cm_event_test_req_ptr_NULL),
		cmocka_unit_test(
			from_cm_event_test_peer_NULL_edata_NULL_req_ptr_NULL),
		cmocka_unit_test(
			from_cm_event_test_RDMA_CM_EVENT_CONNECT_ERROR),
		cmocka_unit_test(from_cm_event_test_create_cq_EAGAIN),
		cmocka_unit_test(
			from_cm_event_test_peer_create_qp_E_PROVIDER_EAGAIN),
		cmocka_unit_test(
			from_cm_event_test_create_qp_EAGAIN_destroy_cq_EIO),
		cmocka_unit_test(from_cm_event_test_malloc_ENOMEM),
		cmocka_unit_test(
			from_cm_event_test_malloc_ENOMEM_destroy_cq_EAGAIN),

		/* rpma_conn_req_from_cm_event()/_delete() lifecycle */
		cmocka_unit_test_setup_teardown(conn_req_from_cm_test_lifecycle,
			conn_req_from_cm_event_setup,
			conn_req_from_cm_event_teardown),

		/* rpma_conn_req_delete() unit tests */
		cmocka_unit_test(delete_test_req_ptr_NULL),
		cmocka_unit_test(delete_test_req_NULL),
		cmocka_unit_test(delete_test_destroy_cq_EAGAIN),
		cmocka_unit_test(
			delete_test_destroy_cq_EAGAIN_reject_EIO_ack_EINTR),
		cmocka_unit_test(delete_test_reject_EAGAIN),
		cmocka_unit_test(delete_test_reject_EAGAIN_ack_EIO),
		cmocka_unit_test(delete_test_ack_EAGAIN),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
