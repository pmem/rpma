// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * ep-next_conn_req.c -- the endpoint unit tests
 *
 * API covered:
 * - rpma_ep_next_conn_req()
 */

#include "librpma.h"
#include "ep-common.h"
#include "cmocka_headers.h"
#include "mocks-rpma-conn_cfg.h"
#include "rpma_err.h"
#include "test-common.h"

/*
 * next_conn_req__ep_NULL - NULL ep is invalid
 */
static void
next_conn_req__ep_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_ep_next_conn_req(NULL, NULL, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * next_conn_req__req_NULL - NULL req is invalid
 */
static void
next_conn_req__req_NULL(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	/* run test */
	int ret = rpma_ep_next_conn_req(estate->ep, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * next_conn_req__ep_NULL_req_NULL - NULL ep and NULL req are invalid
 */
static void
next_conn_req__ep_NULL_req_NULL(void **unused)
{
	/* run test */
	int ret = rpma_ep_next_conn_req(NULL, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * next_conn_req__get_cm_event_EAGAIN -
 * rdma_get_cm_event() fails with EAGAIN
 */
static void
next_conn_req__get_cm_event_EAGAIN(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	expect_value(rdma_get_cm_event, channel, &estate->evch);
	will_return(rdma_get_cm_event, NULL);
	will_return(rdma_get_cm_event, EAGAIN);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_ep_next_conn_req(estate->ep, NULL, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * next_conn_req__get_cm_event_ENODATA -
 * rdma_get_cm_event() fails with ENODATA
 */
static void
next_conn_req__get_cm_event_ENODATA(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	expect_value(rdma_get_cm_event, channel, &estate->evch);
	will_return(rdma_get_cm_event, NULL);
	will_return(rdma_get_cm_event, ENODATA);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_ep_next_conn_req(estate->ep, NULL, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NO_NEXT);
	assert_null(req);
}

/*
 * next_conn_req__event_REJECTED -
 * RDMA_CM_EVENT_REJECTED is unexpected
 */
static void
next_conn_req__event_REJECTED(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	expect_value(rdma_get_cm_event, channel, &estate->evch);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_REJECTED;
	will_return(rdma_get_cm_event, &event);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_ep_next_conn_req(estate->ep, NULL, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * next_conn_req__event_REJECTED_ack_EINVAL -
 * rdma_ack_cm_event() fails with EINVAL after obtaining
 * an RDMA_CM_EVENT_REJECTED event (!= RDMA_CM_EVENT_CONNECT_REQUEST)
 */
static void
next_conn_req__event_REJECTED_ack_EINVAL(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	expect_value(rdma_get_cm_event, channel, &estate->evch);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_REJECTED;
	will_return(rdma_get_cm_event, &event);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, EINVAL);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_ep_next_conn_req(estate->ep, NULL, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * next_conn_req__conn_req_from_cm_event_ENOMEM -
 * rpma_conn_req_from_cm_event() fails with ENOMEM
 */
static void
next_conn_req__conn_req_from_cm_event_ENOMEM(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	expect_value(rdma_get_cm_event, channel, &estate->evch);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_CONNECT_REQUEST;
	will_return(rdma_get_cm_event, &event);

	expect_value(rpma_conn_req_from_cm_event, peer, MOCK_PEER);
	expect_value(rpma_conn_req_from_cm_event, edata, &event);
	expect_value(rpma_conn_req_from_cm_event, cfg, MOCK_CONN_CFG_DEFAULT);
	will_return(rpma_conn_req_from_cm_event, NULL);
	will_return(rpma_conn_req_from_cm_event, RPMA_E_NOMEM);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, MOCK_OK);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_ep_next_conn_req(estate->ep, NULL, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * next_conn_req__from_cm_event_ENOMEM_ack_EINVAL -
 * rpma_conn_req_from_cm_event() fails and
 * rdma_ack_cm_event() fails with EINVAL
 */
static void
next_conn_req__from_cm_event_ENOMEM_ack_EINVAL(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	expect_value(rdma_get_cm_event, channel, &estate->evch);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_CONNECT_REQUEST;
	will_return(rdma_get_cm_event, &event);

	expect_value(rpma_conn_req_from_cm_event, peer, MOCK_PEER);
	expect_value(rpma_conn_req_from_cm_event, edata, &event);
	expect_value(rpma_conn_req_from_cm_event, cfg, MOCK_CONN_CFG_DEFAULT);
	will_return(rpma_conn_req_from_cm_event, NULL);
	will_return(rpma_conn_req_from_cm_event, RPMA_E_NOMEM);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, EINVAL);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_ep_next_conn_req(estate->ep, NULL, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * next_conn_req__success - happy day scenario
 */
static void
next_conn_req__success(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	expect_value(rdma_get_cm_event, channel, &estate->evch);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_CONNECT_REQUEST;
	will_return(rdma_get_cm_event, &event);

	expect_value(rpma_conn_req_from_cm_event, peer, MOCK_PEER);
	expect_value(rpma_conn_req_from_cm_event, edata, &event);
	expect_value(rpma_conn_req_from_cm_event, cfg,
			(estate->cfg == NULL ?
					MOCK_CONN_CFG_DEFAULT : estate->cfg));
	will_return(rpma_conn_req_from_cm_event, MOCK_CONN_REQ);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_ep_next_conn_req(estate->ep, estate->cfg, &req);

	/* verify the results */
	assert_ptr_equal(req, MOCK_CONN_REQ);
	assert_int_equal(ret, 0);
}

int
main(int argc, char *argv[])
{
	/* prepare prestates */
	struct ep_test_state prestate_conn_cfg_default;
	prestate_init(&prestate_conn_cfg_default, NULL);
	struct ep_test_state prestate_conn_cfg_custom;
	prestate_init(&prestate_conn_cfg_default, MOCK_CONN_CFG_CUSTOM);

	const struct CMUnitTest tests[] = {
		/* rpma_ep_next_conn_req() unit tests */
		cmocka_unit_test(next_conn_req__ep_NULL),
		cmocka_unit_test_prestate_setup_teardown(
			next_conn_req__req_NULL,
			setup__ep_listen, teardown__ep_shutdown,
			&prestate_conn_cfg_default),
		cmocka_unit_test(next_conn_req__ep_NULL_req_NULL),
		cmocka_unit_test_prestate_setup_teardown(
			next_conn_req__get_cm_event_ENODATA,
			setup__ep_listen, teardown__ep_shutdown,
			&prestate_conn_cfg_default),
		cmocka_unit_test_prestate_setup_teardown(
			next_conn_req__get_cm_event_EAGAIN,
			setup__ep_listen, teardown__ep_shutdown,
			&prestate_conn_cfg_default),
		cmocka_unit_test_prestate_setup_teardown(
			next_conn_req__event_REJECTED,
			setup__ep_listen, teardown__ep_shutdown,
			&prestate_conn_cfg_default),
		cmocka_unit_test_prestate_setup_teardown(
			next_conn_req__event_REJECTED_ack_EINVAL,
			setup__ep_listen, teardown__ep_shutdown,
			&prestate_conn_cfg_default),
		cmocka_unit_test_prestate_setup_teardown(
			next_conn_req__conn_req_from_cm_event_ENOMEM,
			setup__ep_listen, teardown__ep_shutdown,
			&prestate_conn_cfg_default),
		cmocka_unit_test_prestate_setup_teardown(
			next_conn_req__from_cm_event_ENOMEM_ack_EINVAL,
			setup__ep_listen, teardown__ep_shutdown,
			&prestate_conn_cfg_default),
		{"next_conn_req__success_conn_cfg_default",
			next_conn_req__success, setup__ep_listen,
			teardown__ep_shutdown, &prestate_conn_cfg_default},
		{"next_conn_req__success_conn_cfg_custom",
			next_conn_req__success, setup__ep_listen,
			teardown__ep_shutdown, &prestate_conn_cfg_custom},
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
