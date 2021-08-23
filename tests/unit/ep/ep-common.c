// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * ep-common.c -- common part of the endpoint unit tests
 */

#include "librpma.h"
#include "ep-common.h"
#include "info.h"
#include "cmocka_headers.h"
#include "test-common.h"

const struct rdma_cm_id Cmid_zero = {0};
const struct rdma_event_channel Evch_zero = {.fd = MOCK_FD};

/*
 * The following graph depicts the relationships between mocked function
 * calls:
 *
 * _create_evench_channel--->_create_id-->\
 *                                         \
 * _info_new--------------------------------->_info_bind---->_listen
 *
 * malloc (may happen at any time)
 *
 * Additionally, this test assumes each successful creation of the resource will
 * be paired with destroying it before the end of the setup->test->teardown
 * sequence e.g.:
 * _create_event_channel -> _destroy_event_channel
 */

/*
 * Cmocka does not allow call expect_* from setup whereas check_* will be called
 * on teardown. So, function creating an object which is called during setup
 * cannot queue any expect_* regarding the function destroying the object
 * which will be called in the teardown.
 */
int Mock_ctrl_defer_destruction = MOCK_CTRL_NO_DEFER;

/*
 * rpma_info_bind() function requires successful creation of two types of
 * objects so both of them have to be created before queuing any expect_*
 * against rpma_info_bind().
 */
static struct rdma_cm_id *Mock_ctrl_info_bind_id = NULL;
static struct rpma_info *Mock_ctrl_info_bind_info = NULL;

static void
expect_info_bind()
{
	if (!Mock_ctrl_info_bind_id || !Mock_ctrl_info_bind_info)
		return;

	expect_value(rpma_info_bind_addr, id, Mock_ctrl_info_bind_id);
	expect_value(rpma_info_bind_addr, info, Mock_ctrl_info_bind_info);

	Mock_ctrl_info_bind_id = NULL;
	Mock_ctrl_info_bind_info = NULL;
}

static void
expect_info_bind_info(struct rpma_info *info)
{
	Mock_ctrl_info_bind_info = info;
	expect_info_bind();
}

static void
expect_info_bind_id(struct rdma_cm_id *id)
{
	Mock_ctrl_info_bind_id = id;
	expect_info_bind();
}

/* mocks */

/*
 * rdma_create_event_channel -- rdma_create_event_channel() mock
 */
struct rdma_event_channel *
rdma_create_event_channel(void)
{
	struct rdma_event_channel *evch =
		mock_type(struct rdma_event_channel *);

	if (evch == NULL) {
		errno = mock_type(int);
		return NULL;
	}

	/* queue expects */
	expect_value(rdma_create_id, channel, evch);
	if (!Mock_ctrl_defer_destruction)
		expect_value(rdma_destroy_event_channel, channel, evch);

	return evch;
}

/*
 * rdma_destroy_event_channel -- rdma_destroy_event_channel() mock
 */
void
rdma_destroy_event_channel(struct rdma_event_channel *channel)
{
	check_expected_ptr(channel);
}

/*
 * rdma_create_id -- rdma_create_id() mock
 */
int
rdma_create_id(struct rdma_event_channel *channel, struct rdma_cm_id **id_ptr,
		void *context, enum rdma_port_space ps)
{
	check_expected_ptr(channel);
	assert_non_null(id_ptr);
	assert_null(context);
	assert_int_equal(ps, RDMA_PS_TCP);

	struct rdma_cm_id *id = mock_type(struct rdma_cm_id *);

	if (id == NULL) {
		errno = mock_type(int);
		return -1;
	}

	*id_ptr = id;

	/* queue expects */
	expect_info_bind_id(id);
	if (!Mock_ctrl_defer_destruction)
		expect_value(rdma_destroy_id, id, id);

	return 0;
}

/*
 * rdma_destroy_id -- rdma_destroy_id() mock
 */
int
rdma_destroy_id(struct rdma_cm_id *id)
{
	check_expected_ptr(id);

	Mock_ctrl_info_bind_id = NULL;

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
}

/*
 * rpma_info_new -- rpma_info_new() mock
 */
int
rpma_info_new(const char *addr, const char *port, enum rpma_info_side side,
		struct rpma_info **info_ptr)
{
	assert_string_equal(addr, MOCK_IP_ADDRESS);
	assert_string_equal(port, MOCK_PORT);
	assert_int_equal(side, RPMA_INFO_PASSIVE);
	assert_non_null(info_ptr);

	struct rpma_info *info = mock_type(struct rpma_info *);
	if (info == NULL)
		return mock_type(int);

	*info_ptr = info;

	/* queue expects */
	expect_info_bind_info(info);
	if (!Mock_ctrl_defer_destruction)
		expect_value(rpma_info_delete, *info_ptr, info);

	return 0;
}

/*
 * rpma_info_delete -- rpma_info_delete() mock
 */
int
rpma_info_delete(struct rpma_info **info_ptr)
{
	assert_non_null(info_ptr);
	check_expected_ptr(*info_ptr);
	*info_ptr = NULL;

	Mock_ctrl_info_bind_info = NULL;

	/* if arg is valid this function cannot fail otherwise */
	return 0;
}

/*
 * rpma_info_bind_addr -- rpma_info_bind_addr() mock
 * Note: CM ID is not modified.
 */
int
rpma_info_bind_addr(const struct rpma_info *info, struct rdma_cm_id *id)
{
	check_expected_ptr(info);
	check_expected_ptr(id);
	assert_ptr_equal(info, MOCK_INFO);

	/* XXX validate the errno handling */
	errno = mock_type(int);
	if (errno)
		return RPMA_E_PROVIDER;

	expect_value(rdma_listen, id, id);

	return 0;
}

/*
 * rdma_listen -- rdma_listen() mock
 */
int
rdma_listen(struct rdma_cm_id *id, int backlog)
{
	check_expected_ptr(id);
	assert_int_equal(backlog, 0);

	errno = mock_type(int);
	if (errno)
		return -1;

	return 0;
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
 * rpma_conn_req_from_cm_event -- rpma_conn_req_from_cm_event() mock
 */
int
rpma_conn_req_from_cm_event(struct rpma_peer *peer,
		struct rdma_cm_event *edata, const struct rpma_conn_cfg *cfg,
		struct rpma_conn_req **req_ptr)
{
	check_expected_ptr(peer);
	check_expected_ptr(edata);
	check_expected_ptr(cfg);
	assert_non_null(req_ptr);

	struct rpma_conn_req *req = mock_type(struct rpma_conn_req *);
	if (!req)
		return mock_type(int);

	*req_ptr = req;
	return 0;
}

/*
 * rdma_ack_cm_event -- rdma_ack_cm_event() mock
 */
int
rdma_ack_cm_event(struct rdma_cm_event *event)
{
	check_expected_ptr(event);
	return mock_type(int);
}

/*
 * rdma_event_str -- rdma_event_str() mock
 */
const char *
rdma_event_str(enum rdma_cm_event_type event)
{
	return "";
}

/*
 * prestate_init -- initialize ep_listen prestate
 */
void
prestate_init(struct ep_test_state *prestate, struct rpma_conn_cfg *cfg)
{
	memset(prestate, 0, sizeof(struct ep_test_state));
	prestate->cfg = cfg;
}

/* setups and teardowns */

/*
 * setup__ep_listen - prepare a valid rpma_ep object
 */
int
setup__ep_listen(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;
	memset(&estate->cmid, 0, sizeof(struct rdma_cm_id));
	estate->evch.fd = MOCK_FD;

	/* configure mocks: */
	Mock_ctrl_defer_destruction = MOCK_CTRL_DEFER;
	will_return(rdma_create_event_channel, &estate->evch);
	will_return(rdma_create_id, &estate->cmid);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rpma_info_bind_addr, MOCK_OK);
	will_return(rdma_listen, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_OK);
	expect_value(rpma_info_delete, *info_ptr, MOCK_INFO);

	/* prepare an object */
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT,
		&estate->ep);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(estate->ep);
	assert_int_equal(memcmp(&estate->cmid, &Cmid_zero,
			sizeof(struct rdma_cm_id)), 0);
	assert_int_equal(memcmp(&estate->evch, &Evch_zero,
			sizeof(struct rdma_event_channel)), 0);

	/* restore default mock configuration */
	Mock_ctrl_defer_destruction = MOCK_CTRL_NO_DEFER;

	return 0;
}

/*
 * teardown__ep_shutdown - delete the rpma_ep object
 */
int
teardown__ep_shutdown(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	/* configure mocks: */
	expect_value(rdma_destroy_id, id, &estate->cmid);
	will_return(rdma_destroy_id, MOCK_OK);
	expect_value(rdma_destroy_event_channel, channel, &estate->evch);

	/* delete the object */
	int ret = rpma_ep_shutdown(&estate->ep);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(estate->ep);
	assert_int_equal(memcmp(&estate->cmid, &Cmid_zero,
		sizeof(estate->cmid)), 0);
	assert_int_equal(memcmp(&estate->evch, &Evch_zero,
		sizeof(estate->evch)), 0);

	return 0;
}
