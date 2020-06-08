/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * peer-test.c -- a peer unit test
 */

#include <infiniband/verbs.h>

#include "cmocka_headers.h"
#include "conn_req.h"

#define MOCK_PEER		(struct rpma_peer *)0xFEEF
#define MOCK_CONN_REQ	(struct rpma_conn_req *)0xCFEF

#define CM_EVENT_CONNECTION_REQUEST_INIT \
	{NULL, NULL, RDMA_CM_EVENT_CONNECT_REQUEST, 0, {{0}}}

#define CM_EVENT_CONNECT_ERROR_INIT \
	{NULL, NULL, RDMA_CM_EVENT_CONNECT_ERROR, 0, {{0}}}

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
 * rdma_connect -- rdma_connect() mock
 */
int
rdma_connect(struct rdma_cm_id *id, struct rdma_conn_param *conn_param)
{
	check_expected_ptr(id);

	assert_non_null(conn_param);
	check_expected_ptr(conn_param->private_data);
	check_expected_ptr(conn_param->private_data_len);
	assert_int_equal(conn_param->responder_resources, RDMA_MAX_RESP_RES);
	assert_int_equal(conn_param->initiator_depth, RDMA_MAX_INIT_DEPTH);
	assert_int_equal(conn_param->flow_control, 1);
	assert_int_equal(conn_param->retry_count, 7); /* max 3-bit value */
	assert_int_equal(conn_param->rnr_retry_count, 7); /* max 3-bit value */
	assert_int_equal(conn_param->srq, 0); /* unused */
	assert_int_equal(conn_param->qp_num, 0); /* unused */

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rdma_accept -- rdma_accept() mock
 */
int
rdma_accept(struct rdma_cm_id *id, struct rdma_conn_param *conn_param)
{
	check_expected_ptr(id);

	assert_non_null(conn_param);
	check_expected_ptr(conn_param->private_data);
	check_expected_ptr(conn_param->private_data_len);
	assert_int_equal(conn_param->responder_resources, RDMA_MAX_RESP_RES);
	assert_int_equal(conn_param->initiator_depth, RDMA_MAX_INIT_DEPTH);
	assert_int_equal(conn_param->flow_control, 1);
	assert_int_equal(conn_param->retry_count, 7); /* max 3-bit value */
	assert_int_equal(conn_param->rnr_retry_count, 7); /* max 3-bit value */
	assert_int_equal(conn_param->srq, 0); /* unused */
	assert_int_equal(conn_param->qp_num, 0); /* unused */

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
	int ret = rpma_conn_req_from_cm_event(MOCK_PEER, &event, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * XXX need additional tests when rpma_conn_req_new() and
 * rpma_conn_req_from_id() will be ready
 */

/*
 * connect_test_req_ptr_NULL -- NULL req_ptr is invalid
 */
static void
connect_test_req_ptr_NULL(void **unused)
{
	/* run test */
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(NULL, NULL, 0, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(conn);
}

/*
 * connect_test_conn_ptr_NULL -- NULL conn_ptr is invalid
 */
static void
connect_test_conn_ptr_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = MOCK_CONN_REQ;
	int ret = rpma_conn_req_connect(&req, NULL, 0, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
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
	int ret = rpma_conn_req_connect(&req, NULL, 0, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(conn);
}

/*
 * connect_test_pdata_NULL_pdata_len_not_0 -- private_data_len != when
 * private_data == NULL is invalid
 */
static void
connect_test_pdata_NULL_pdata_len_not_0(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = MOCK_CONN_REQ;
	struct rpma_conn *conn = NULL;
	int ret = rpma_conn_req_connect(&req, NULL, 1, &conn);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(conn);
}

/*
 * XXX need additional tests when rpma_conn_req_new() and
 * rpma_conn_req_from_id() will be ready
 */

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

		/* rpma_conn_req_connect() unit tests */
		cmocka_unit_test(connect_test_req_ptr_NULL),
		cmocka_unit_test(connect_test_conn_ptr_NULL),
		cmocka_unit_test(connect_test_req_NULL),
		cmocka_unit_test(connect_test_pdata_NULL_pdata_len_not_0),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
