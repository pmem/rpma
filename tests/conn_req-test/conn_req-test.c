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
#include "info.h"
#include "rpma_err.h"

#define MOCK_IP_ADDRESS	"127.0.0.1"
#define MOCK_SERVICE	"1234" /* a random port number */

/* random values */
#define MOCK_VERBS	(struct ibv_context *)0x4E4B
#define MOCK_CQ		(struct ibv_cq *)0x00C0
#define MOCK_PEER	(struct rpma_peer *)0xFEEF
#define MOCK_INFO	(struct rpma_info *)0xE6B2
#define MOCK_CONN	(struct rpma_peer *)0xC004

#define CM_EVENT_CONNECTION_REQUEST_INIT \
	{NULL, NULL, RDMA_CM_EVENT_CONNECT_REQUEST, 0, {{0}}}

#define CM_EVENT_CONNECT_ERROR_INIT \
	{NULL, NULL, RDMA_CM_EVENT_CONNECT_ERROR, 0, {{0}}}

#define NO_ERROR 0

/* mock control entities */

#define MOCK_CTRL_DEFER		1
#define MOCK_CTRL_NO_DEFER	0

/*
 * Cmocka does not allow call expect_* from setup whereas check_* will be called
 * on teardown. So, function creating an object which is called during setup
 * cannot queue any expect_* regarding the function destroying the object
 * which will be called in the teardown.
 */
static int Mock_ctrl_defer_destruction = MOCK_CTRL_NO_DEFER;

/* mocks */

/*
 * rpma_conn_new -- rpma_conn_new()  mock
 */
int
rpma_conn_new(struct rdma_cm_id *id, struct ibv_cq *cq,
		struct rpma_conn **conn_ptr)
{
	check_expected_ptr(id);
	assert_ptr_equal(cq, MOCK_CQ);

	assert_non_null(conn_ptr);

	struct rpma_conn *conn = mock_type(struct rpma_conn *);
	if (!conn) {
		int result = mock_type(int);
		if (result == RPMA_E_PROVIDER)
			Rpma_provider_error = mock_type(int);

		return result;
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
 * rdma_accept -- rdma_accept() mock
 */
int
rdma_accept(struct rdma_cm_id *id, struct rdma_conn_param *conn_param)
{
	check_expected(id);
	assert_non_null(conn_param);
	assert_null(conn_param->private_data);
	assert_int_equal(conn_param->private_data_len, 0);
	assert_int_equal(conn_param->responder_resources, RDMA_MAX_RESP_RES);
	assert_int_equal(conn_param->initiator_depth, RDMA_MAX_INIT_DEPTH);
	assert_int_equal(conn_param->flow_control, 1);
	assert_int_equal(conn_param->retry_count, 7); /* max 3-bit value */
	assert_int_equal(conn_param->rnr_retry_count, 7); /* max 3-bit value */

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
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

/*
 * rpma_info_new -- mock of rpma_info_new
 */
int
rpma_info_new(const char *addr, const char *service, enum rpma_info_side side,
		struct rpma_info **info_ptr)
{
	assert_string_equal(addr, MOCK_IP_ADDRESS);
	assert_string_equal(service, MOCK_SERVICE);
	assert_int_equal(side, RPMA_INFO_ACTIVE);

	*info_ptr = mock_type(struct rpma_info *);
	if (*info_ptr == NULL) {
		int result = mock_type(int);
		assert_int_not_equal(result, 0);

		if (result == RPMA_E_PROVIDER)
			Rpma_provider_error = mock_type(int);

		return result;
	}

	return 0;
}

/*
 * rpma_info_delete -- mock of rpma_info_delete
 */
int
rpma_info_delete(struct rpma_info **info_ptr)
{
	assert_non_null(info_ptr);
	assert_int_equal(*info_ptr, MOCK_INFO);

	/* if argument is correct it connot fail */
	return 0;
}

/*
 * rpma_info_resolve_addr -- mock of rpma_info_resolve_addr
 */
int
rpma_info_resolve_addr(const struct rpma_info *info, struct rdma_cm_id *id)
{
	assert_int_equal(info, MOCK_INFO);
	check_expected(id);

	int ret = mock_type(int);
	if (ret == RPMA_E_PROVIDER)
		Rpma_provider_error = mock_type(int);

	if (ret == NO_ERROR)
		expect_value(rdma_resolve_route, id, id);

	return ret;
}

/*
 * rdma_create_id -- mock of rdma_create_id
 */
int
rdma_create_id(struct rdma_event_channel *channel,
		struct rdma_cm_id **id, void *context,
		enum rdma_port_space ps)
{
	assert_non_null(id);
	assert_null(context);
	assert_int_equal(ps, RDMA_PS_TCP);

	/* allocate (struct rdma_cm_id *) */
	*id = mock_type(struct rdma_cm_id *);
	if (*id == NULL) {
		errno = mock_type(int);
		return -1;
	}

	if (!Mock_ctrl_defer_destruction)
		expect_value(rdma_destroy_id, id, *id);

	return 0;
}

/*
 * rdma_destroy_id -- mock of rdma_destroy_id
 */
int
rdma_destroy_id(struct rdma_cm_id *id)
{
	check_expected(id);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_resolve_route -- mock of rdma_resolve_route
 */
int
rdma_resolve_route(struct rdma_cm_id *id, int timeout_ms)
{
	check_expected(id);
	assert_int_equal(timeout_ms, RPMA_DEFAULT_TIMEOUT);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_connect -- rdma_connect() mock
 */
int
rdma_connect(struct rdma_cm_id *id, struct rdma_conn_param *conn_param)
{
	check_expected(id);
	assert_non_null(conn_param);
	assert_null(conn_param->private_data);
	assert_int_equal(conn_param->private_data_len, 0);
	assert_int_equal(conn_param->responder_resources, RDMA_MAX_RESP_RES);
	assert_int_equal(conn_param->initiator_depth, RDMA_MAX_INIT_DEPTH);
	assert_int_equal(conn_param->flow_control, 1);
	assert_int_equal(conn_param->retry_count, 7); /* max 3-bit value */
	assert_int_equal(conn_param->rnr_retry_count, 7); /* max 3-bit value */

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
 * all the resources used between conn_req_from_cm_event_setup and
 * conn_req_from_cm_event_teardown
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
	static struct conn_req_test_state cstate = {{0}};
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
	 * The thing is done by conn_req_from_cm_event_setup() and
	 * conn_req_from_cm_event_teardown().
	 */
}

/*
 * new_test_peer_NULL -- NULL peer is invalid
 */
static void
new_test_peer_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(NULL, MOCK_IP_ADDRESS, MOCK_SERVICE,
					&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * new_test_addr_NULL -- NULL addr is invalid
 */
static void
new_test_addr_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, NULL, MOCK_SERVICE,
					&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * new_test_service_NULL -- NULL service is invalid
 */
static void
new_test_service_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, NULL,
					&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * new_test_req_ptr_NULL -- NULL req_ptr is invalid
 */
static void
new_test_req_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
					NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new_test_all_NULL -- all NULL arguments are invalid
 */
static void
new_test_all_NULL(void **unused)
{
	/* run test */
	int ret = rpma_conn_req_new(NULL, NULL, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * new_test_info_new_E_PROVIDER_EAGAIN -- rpma_info_new() fails with
 * RPMA_E_PROVIDER+EAGAIN
 */
static void
new_test_info_new_E_PROVIDER_EAGAIN(void **unused)
{
	struct rdma_cm_id id = {0};

	/* configure mocks */
	will_return(rpma_info_new, NULL);
	will_return(rpma_info_new, RPMA_E_PROVIDER);
	will_return(rpma_info_new, EAGAIN);
	will_return_maybe(rdma_create_id, &id);
	will_return_maybe(rdma_destroy_id, NO_ERROR);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * new_test_create_id_EAGAIN -- rdma_create_id() fails with EAGAIN
 */
static void
new_test_create_id_EAGAIN(void **unused)
{
	/* configure mocks */
	will_return(rdma_create_id, NULL);
	will_return(rdma_create_id, EAGAIN);
	will_return_maybe(rpma_info_new, MOCK_INFO);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * new_test_resolve_addr_E_PROVIDER_EAGAIN -- rpma_info_resolve_addr() fails
 * with RPMA_E_PROVIDER+EAGAIN
 */
static void
new_test_resolve_addr_E_PROVIDER_EAGAIN(void **unused)
{
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &id);
	expect_value(rpma_info_resolve_addr, id, &id);
	will_return(rpma_info_resolve_addr, RPMA_E_PROVIDER);
	will_return(rpma_info_resolve_addr, EAGAIN);
	will_return(rdma_destroy_id, NO_ERROR);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * new_test_resolve_route_EAGAIN -- rdma_resolve_route() fails with EAGAIN
 */
static void
new_test_resolve_route_EAGAIN(void **unused)
{
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &id);
	expect_value(rpma_info_resolve_addr, id, &id);
	will_return(rpma_info_resolve_addr, NO_ERROR);
	will_return(rdma_resolve_route, EAGAIN);
	will_return(rdma_destroy_id, NO_ERROR);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * new_test_create_cq_EAGAIN -- ibv_create_cq() fails with EAGAIN
 */
static void
new_test_create_cq_EAGAIN(void **unused)
{
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &id);
	expect_value(rpma_info_resolve_addr, id, &id);
	will_return(rpma_info_resolve_addr, NO_ERROR);
	will_return(rdma_resolve_route, NO_ERROR);
	will_return(ibv_create_cq, NULL);
	will_return(ibv_create_cq, EAGAIN);
	will_return(rdma_destroy_id, NO_ERROR);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * new_test_peer_create_qp_E_PROVIDER_EAGAIN -- rpma_peer_create_qp() fails
 * with RPMA_E_PROVIDER+EAGAIN
 */
static void
new_test_peer_create_qp_E_PROVIDER_EAGAIN(void **unused)
{
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &id);
	expect_value(rpma_info_resolve_addr, id, &id);
	will_return(rpma_info_resolve_addr, NO_ERROR);
	will_return(rdma_resolve_route, NO_ERROR);
	will_return(ibv_create_cq, MOCK_CQ);
	expect_value(rpma_peer_create_qp, id, &id);
	will_return(rpma_peer_create_qp, RPMA_E_PROVIDER);
	will_return(rpma_peer_create_qp, EAGAIN);
	will_return(ibv_destroy_cq, NO_ERROR);
	will_return(rdma_destroy_id, NO_ERROR);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * new_test_malloc_ENOMEM -- malloc() fails with ENOMEM
 */
static void
new_test_malloc_ENOMEM(void **unused)
{
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &id);
	expect_value(rpma_info_resolve_addr, id, &id);
	will_return(rpma_info_resolve_addr, NO_ERROR);
	will_return(rdma_resolve_route, NO_ERROR);
	will_return(ibv_create_cq, MOCK_CQ);
	expect_value(rpma_peer_create_qp, id, &id);
	will_return(rpma_peer_create_qp, NO_ERROR);
	will_return(__wrap__test_malloc, ENOMEM);
	expect_value(rdma_destroy_qp, id, &id);
	will_return(ibv_destroy_cq, NO_ERROR);
	will_return(rdma_destroy_id, NO_ERROR);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * new_test_malloc_ENOMEM_subsequent_EAGAIN -- malloc() fails with ENOMEM
 * whereas subsequent calls fail with EAGAIN
 */
static void
new_test_malloc_ENOMEM_subsequent_EAGAIN(void **unused)
{
	struct rdma_cm_id id = {0};
	id.verbs = MOCK_VERBS;

	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &id);
	expect_value(rpma_info_resolve_addr, id, &id);
	will_return(rpma_info_resolve_addr, NO_ERROR);
	will_return(rdma_resolve_route, NO_ERROR);
	will_return(ibv_create_cq, MOCK_CQ);
	expect_value(rpma_peer_create_qp, id, &id);
	will_return(rpma_peer_create_qp, NO_ERROR);
	will_return(__wrap__test_malloc, ENOMEM); /* first error */
	expect_value(rdma_destroy_qp, id, &id);
	will_return(ibv_destroy_cq, EAGAIN); /* second error */
	will_return(rdma_destroy_id, EAGAIN); /* third error */

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
			&req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * all the resources used between conn_req_new_setup and conn_req_new_teardown
 */
struct conn_req_new_test_state {
	struct rdma_cm_id id;
	struct rpma_conn_req *req;
};

/*
 * conn_req_new_setup -- prepare a new outgoing rpma_conn_req
 */
static int
conn_req_new_setup(void **cstate_ptr)
{
	static struct conn_req_new_test_state cstate = {{0}};
	memset(&cstate, 0, sizeof(cstate));
	cstate.id.verbs = MOCK_VERBS;

	/* configure mocks for rpma_conn_req_new() */
	Mock_ctrl_defer_destruction = MOCK_CTRL_DEFER;
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, &cstate.id);
	expect_value(rpma_info_resolve_addr, id, &cstate.id);
	will_return(rpma_info_resolve_addr, NO_ERROR);
	will_return(rdma_resolve_route, NO_ERROR);
	will_return(ibv_create_cq, MOCK_CQ);
	expect_value(rpma_peer_create_qp, id, &cstate.id);
	will_return(rpma_peer_create_qp, NO_ERROR);
	will_return(__wrap__test_malloc, NO_ERROR);

	/* run test */
	int ret = rpma_conn_req_new(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_SERVICE,
			&cstate.req);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
	assert_non_null(cstate.req);

	*cstate_ptr = &cstate;

	/* restore default mock configuration */
	Mock_ctrl_defer_destruction = MOCK_CTRL_NO_DEFER;

	return 0;
}

/*
 * conn_req_new_teardown -- delete the outgoing rpma_conn_req object
 */
static int
conn_req_new_teardown(void **cstate_ptr)
{
	struct conn_req_new_test_state *cstate = *cstate_ptr;

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, NO_ERROR);
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, 0);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
	assert_null(cstate->req);

	*cstate_ptr = NULL;

	return 0;
}

/*
 * new_test_success -- all is OK
 */
static void
new_test_success(void **unused)
{
	/*
	 * The thing is done by conn_req_new_setup() and
	 * conn_req_new_teardown().
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

/*
 * delete_test_destroy_id_EAGAIN - rdma_destroy_id() fails with EAGAIN
 */
static void
delete_test_destroy_id_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(conn_req_new_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, NO_ERROR);
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, EAGAIN);

	/* run test */
	int ret = rpma_conn_req_delete(&cstate->req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
}

/*
 * connect_test_req_ptr_NULL -- NULL req_ptr is invalid
 */
static void
connect_test_req_ptr_NULL(void **unused)
{
	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(NULL, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(conn);
}

/*
 * connect_test_conn_ptr_NULL -- NULL conn_ptr is invalid
 */
static void
connect_test_conn_ptr_NULL(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;

	/* run test */
	int ret = rpma_conn_req_connect(&cstate->req, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_non_null(cstate->req);
}

/*
 * connect_test_req_NULL -- NULL *req_ptr is invalid
 */
static void
connect_test_req_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(conn);
	assert_null(req);
}

/*
 * connect_test_pdata_NULL_pdata_ptr_NULL -- pdata->ptr == NULL is invalid
 */
static void
connect_test_pdata_NULL_pdata_ptr_NULL(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;

	/* run test */
	struct rpma_conn *conn = NULL;
	struct rpma_conn_private_data pdata = {NULL, 1};
	int ret = rpma_conn_req_connect(&cstate->req, &pdata, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_non_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_test_pdata_NULL_pdata_len_0 -- pdata->len == 0 is invalid
 */
static void
connect_test_pdata_NULL_pdata_len_0(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;
	char buff = 0;

	/* run test */
	struct rpma_conn *conn = NULL;
	struct rpma_conn_private_data pdata = {&buff, 0};
	int ret = rpma_conn_req_connect(&cstate->req, &pdata, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_non_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_test_pdata_NULL_pdata_ptr_NULL_len_0 -- pdata->ptr == NULL and
 * pdata->len == 0 are invalid
 */
static void
connect_test_pdata_NULL_pdata_ptr_NULL_len_0(void **cstate_ptr)
{
	struct conn_req_test_state *cstate = *cstate_ptr;

	/* run test */
	struct rpma_conn *conn = NULL;
	struct rpma_conn_private_data pdata = {NULL, 0};
	int ret = rpma_conn_req_connect(&cstate->req, &pdata, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_non_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_test_accept_EAGAIN -- rdma_accept() fails with EAGAIN
 */
static void
connect_test_accept_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, EAGAIN);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, NO_ERROR);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, NO_ERROR);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_test_accept_EAGAIN_subsequent_EIO -- rdma_accept() fails with EAGAIN
 * whereas subsequent (rdma_ack_cm_event(), ibv_destroy_cq()) fail with EIO
 */
static void
connect_test_accept_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, EAGAIN);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, EIO);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, EIO);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_test_ack_EAGAIN -- rdma_ack_cm_event() fails with EAGAIN
 */
static void
connect_test_ack_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, NO_ERROR);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, EAGAIN);
	expect_value(rdma_disconnect, id, &cstate->id);
	will_return(rdma_disconnect, NO_ERROR);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, NO_ERROR);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_test_ack_EAGAIN_subsequent_EIO -- rdma_ack_cm_event() fails with
 * EAGAIN whereas subsequent (rdma_disconnect(), ibv_destroy_cq()) fail with
 * EIO
 */
static void
connect_test_ack_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, NO_ERROR);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, EAGAIN);
	expect_value(rdma_disconnect, id, &cstate->id);
	will_return(rdma_disconnect, EIO);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, EIO);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_test_connect_EAGAIN -- rdma_connect() fails with EAGAIN
 */
static void
connect_test_connect_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(conn_req_new_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_connect, id, &cstate->id);
	will_return(rdma_connect, EAGAIN);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, NO_ERROR);
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, NO_ERROR);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_test_connect_EAGAIN_subsequent_EIO -- rdma_connect() fails with
 * EAGAIN whereas subsequent (ibv_destroy_cq(), rdma_destroy_id()) fail with EIO
 */
static void
connect_test_connect_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(conn_req_new_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_connect, id, &cstate->id);
	will_return(rdma_connect, EAGAIN);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, EIO);
	expect_value(rdma_destroy_id, id, &cstate->id);
	will_return(rdma_destroy_id, EIO);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_test_conn_new_E_PROVIDER_EAGAIN -- rpma_conn_new() fails with
 * RPMA_E_PROVIDER + EAGAIN
 *
 * Note: It should not affect the test result if the failing API call is fed
 * with a connection request created from via rpma_conn_req_from_cm_event()
 * (an incoming connection request) or via rpma_conn_req_new() (an outgoing
 * connection request). This implementation makes use of an incoming one.
 */
static void
connect_test_conn_new_E_PROVIDER_EAGAIN(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, NO_ERROR);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, NO_ERROR);
	expect_value(rpma_conn_new, id, &cstate->id);
	will_return(rpma_conn_new, NULL);
	will_return(rpma_conn_new, RPMA_E_PROVIDER);
	will_return(rpma_conn_new, EAGAIN);
	expect_value(rdma_disconnect, id, &cstate->id);
	will_return(rdma_disconnect, NO_ERROR);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, NO_ERROR);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_test_conn_new_E_PROVIDER_EAGAIN_subsequent_EIO -- rpma_conn_new()
 * fails with RPMA_E_PROVIDER + EAGAIN whereas subsequent (rdma_disconnect(),
 * ibv_destroy_cq()) fail with EIO
 *
 * Note: It should not affect the test result if the failing API call is fed
 * with a connection request created from via rpma_conn_req_from_cm_event()
 * (an incoming connection request) or via rpma_conn_req_new() (an outgoing
 * connection request). This implementation makes use of an incoming one.
 */
static void
connect_test_conn_new_E_PROVIDER_EAGAIN_subsequent_EIO(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, NO_ERROR);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, NO_ERROR);
	expect_value(rpma_conn_new, id, &cstate->id);
	will_return(rpma_conn_new, NULL);
	will_return(rpma_conn_new, RPMA_E_PROVIDER);
	will_return(rpma_conn_new, EAGAIN);
	expect_value(rdma_disconnect, id, &cstate->id);
	will_return(rdma_disconnect, EIO);
	expect_value(rdma_destroy_qp, id, &cstate->id);
	will_return(ibv_destroy_cq, EIO);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(cstate->req);
	assert_null(conn);
}

/*
 * connect_test_success_incoming -- rpma_conn_req_connect() success (using an
 * incoming connection request)
 */
static void
connect_test_success_incoming(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_test_state *cstate = NULL;
	assert_int_equal(conn_req_from_cm_event_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_accept, id, &cstate->id);
	will_return(rdma_accept, NO_ERROR);
	expect_value(rdma_ack_cm_event, event, &cstate->event);
	will_return(rdma_ack_cm_event, NO_ERROR);
	expect_value(rpma_conn_new, id, &cstate->id);
	will_return(rpma_conn_new, MOCK_CONN);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
	assert_null(cstate->req);
	assert_int_equal(conn, MOCK_CONN);
}

/*
 * connect_test_success_outgoing -- rpma_conn_req_connect() success (using an
 * outgoing connection request)
 */
static void
connect_test_success_outgoing(void **unused)
{
	/* WA for cmocka/issues#47 */
	struct conn_req_new_test_state *cstate = NULL;
	assert_int_equal(conn_req_new_setup((void **)&cstate), 0);
	assert_non_null(cstate);

	/* configure mocks */
	expect_value(rdma_connect, id, &cstate->id);
	will_return(rdma_connect, NO_ERROR);
	expect_value(rpma_conn_new, id, &cstate->id);
	will_return(rpma_conn_new, MOCK_CONN);

	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&cstate->req, NULL, &conn);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
	assert_null(cstate->req);
	assert_int_equal(conn, MOCK_CONN);
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

		/* rpma_conn_req_new() unit tests */
		cmocka_unit_test(new_test_peer_NULL),
		cmocka_unit_test(new_test_addr_NULL),
		cmocka_unit_test(new_test_service_NULL),
		cmocka_unit_test(new_test_req_ptr_NULL),
		cmocka_unit_test(new_test_all_NULL),
		cmocka_unit_test(new_test_info_new_E_PROVIDER_EAGAIN),
		cmocka_unit_test(new_test_create_id_EAGAIN),
		cmocka_unit_test(new_test_resolve_addr_E_PROVIDER_EAGAIN),
		cmocka_unit_test(new_test_resolve_route_EAGAIN),
		cmocka_unit_test(new_test_create_cq_EAGAIN),
		cmocka_unit_test(new_test_peer_create_qp_E_PROVIDER_EAGAIN),
		cmocka_unit_test(new_test_malloc_ENOMEM),
		cmocka_unit_test(new_test_malloc_ENOMEM_subsequent_EAGAIN),
		cmocka_unit_test_setup_teardown(new_test_success,
				conn_req_new_setup, conn_req_new_teardown),

		/* rpma_conn_req_delete() unit tests */
		cmocka_unit_test(delete_test_req_ptr_NULL),
		cmocka_unit_test(delete_test_req_NULL),
		cmocka_unit_test(delete_test_destroy_cq_EAGAIN),
		cmocka_unit_test(
			delete_test_destroy_cq_EAGAIN_reject_EIO_ack_EINTR),
		cmocka_unit_test(delete_test_reject_EAGAIN),
		cmocka_unit_test(delete_test_reject_EAGAIN_ack_EIO),
		cmocka_unit_test(delete_test_ack_EAGAIN),
		cmocka_unit_test(delete_test_destroy_id_EAGAIN),

		/* rpma_conn_req_connect() unit tests */
		cmocka_unit_test(connect_test_req_ptr_NULL),
		cmocka_unit_test_setup_teardown(connect_test_conn_ptr_NULL,
			conn_req_from_cm_event_setup,
			conn_req_from_cm_event_teardown),
		cmocka_unit_test(connect_test_req_NULL),
		cmocka_unit_test_setup_teardown(
			connect_test_pdata_NULL_pdata_ptr_NULL,
			conn_req_from_cm_event_setup,
			conn_req_from_cm_event_teardown),
		cmocka_unit_test_setup_teardown(
			connect_test_pdata_NULL_pdata_len_0,
			conn_req_from_cm_event_setup,
			conn_req_from_cm_event_teardown),
		cmocka_unit_test_setup_teardown(
			connect_test_pdata_NULL_pdata_ptr_NULL_len_0,
			conn_req_from_cm_event_setup,
			conn_req_from_cm_event_teardown),
		cmocka_unit_test(connect_test_accept_EAGAIN),
		cmocka_unit_test(connect_test_accept_EAGAIN_subsequent_EIO),
		cmocka_unit_test(connect_test_ack_EAGAIN),
		cmocka_unit_test(connect_test_ack_EAGAIN_subsequent_EIO),
		cmocka_unit_test(connect_test_connect_EAGAIN),
		cmocka_unit_test(connect_test_connect_EAGAIN_subsequent_EIO),
		cmocka_unit_test(connect_test_conn_new_E_PROVIDER_EAGAIN),
		cmocka_unit_test(
			connect_test_conn_new_E_PROVIDER_EAGAIN_subsequent_EIO),
		cmocka_unit_test(connect_test_success_incoming),
		cmocka_unit_test(connect_test_success_outgoing),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
