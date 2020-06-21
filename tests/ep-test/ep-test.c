/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * ep-test.c -- the endpoint unit tests
 */

#include <stdlib.h>

#include "cmocka_headers.h"
#include "conn_req.h"
#include "info.h"
#include "librpma.h"
#include "rpma_err.h"

#include <infiniband/verbs.h>

#define MOCK_ADDR	"127.0.0.1"
#define MOCK_SERVICE	"1234"
#define MOCK_PEER	(struct rpma_peer *)0xFEEF
#define MOCK_INFO	(struct rpma_info *)0x14F0
#define MOCK_CONN_REQ	(struct rpma_conn_req *)0xCFEF

#define NO_ERROR	0
#define MOCK_ERRNO	0xE440

static const struct rdma_cm_id Cmid_zero = {0};
static const struct rdma_event_channel Evch_zero = {0};

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
 * be paired with destroying it before the end of the setup-test-teardown
 * sequence e.g.:
 * _create_event_channel -> _destroy_event_channel
 */

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

/*
 * rpma_info_bind() function requires succesfull creation of two types of
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
rpma_info_new(const char *addr, const char *service, enum rpma_info_side side,
		struct rpma_info **info_ptr)
{
	assert_ptr_equal(addr, MOCK_ADDR);
	assert_ptr_equal(service, MOCK_SERVICE);
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

	Rpma_provider_error = mock_type(int);

	if (Rpma_provider_error)
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
 * rpma_conn_req_from_cm_event -- rpma_conn_req_from_cm_event() mock
 */
int
rpma_conn_req_from_cm_event(struct rpma_peer *peer, struct rdma_cm_event *edata,
		struct rpma_conn_req **req_ptr)
{
	check_expected_ptr(peer);
	check_expected_ptr(edata);
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

/* tests */

/*
 * ep_listen_test_peer_NULL - NULL peer is invalid
 */
static void
ep_listen_test_peer_NULL(void **unused)
{
	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(NULL, MOCK_ADDR, MOCK_SERVICE, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(ep);
}

/*
 * ep_listen_test_addr_NULL - NULL addr is invalid
 */
static void
ep_listen_test_addr_NULL(void **unused)
{
	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, NULL, MOCK_SERVICE, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(ep);
}

/*
 * ep_listen_test_service_NULL - NULL service is invalid
 */
static void
ep_listen_test_service_NULL(void **unused)
{
	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_ADDR, NULL, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(ep);
}

/*
 * ep_listen_test_ep_ptr_NULL - NULL ep_ptr is invalid
 */
static void
ep_listen_test_ep_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_ADDR, MOCK_SERVICE, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * ep_listen_test_peer_addr_service_ep_ptr_NULL - peer, addr, service
 * and ep_ptr == NULL are invalid
 */
static void
ep_listen_test_peer_addr_service_ep_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_ep_listen(NULL, NULL, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * ep_listen_test_create_evch_EAGAIN - rdma_create_event_channel() fails
 * with EAGAIN
 */
static void
ep_listen_test_create_evch_EAGAIN(void **unused)
{
	/*
	 * configure mocks for:
	 * - constructing:
	 */
	will_return(rdma_create_event_channel, NULL);
	will_return(rdma_create_event_channel, EAGAIN);
	/* - things which may happen: */
	will_return_maybe(rpma_info_new, MOCK_INFO);
	will_return_maybe(__wrap__test_malloc, NO_ERROR);

	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_ADDR, MOCK_SERVICE, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(ep);
}

/*
 * ep_listen_test_create_id_EAGAIN - rdma_create_id() fails with EAGAIN
 */
static void
ep_listen_test_create_id_EAGAIN(void **unused)
{
	/*
	 * configure mocks:
	 * - constructing
	 */
	struct rdma_event_channel evch;
	will_return(rdma_create_event_channel, &evch);
	will_return(rdma_create_id, NULL);
	will_return(rdma_create_id, EAGAIN);
	/* - things which may happen: */
	will_return_maybe(rpma_info_new, MOCK_INFO);
	will_return_maybe(__wrap__test_malloc, NO_ERROR);

	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_ADDR, MOCK_SERVICE, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(ep);
}

/*
 * ep_listen_test_info_new_E_NOMEM - rpma_info_new() fails with
 * RPMA_E_NOMEM
 */
static void
ep_listen_test_info_new_E_NOMEM(void **unused)
{
	/*
	 * configure mocks for:
	 * - constructing
	 */
	will_return(rpma_info_new, NULL);
	will_return(rpma_info_new, RPMA_E_NOMEM);
	/* - things which may happen: */
	struct rdma_event_channel evch;
	will_return_maybe(rdma_create_event_channel, &evch);
	struct rdma_cm_id id;
	will_return_maybe(rdma_create_id, &id);
	will_return_maybe(rdma_destroy_id, NO_ERROR);

	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_ADDR, MOCK_SERVICE, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(ep);
}

/*
 * ep_listen_test_info_bind_addr_E_PROVIDER - rpma_info_bind_addr() fails
 * with RPMA_E_PROVIDER
 */
static void
ep_listen_test_info_bind_addr_E_PROVIDER(void **unused)
{
	/*
	 * configure mocks for:
	 * - constructing
	 */
	struct rdma_event_channel evch;
	will_return(rdma_create_event_channel, &evch);
	struct rdma_cm_id id;
	will_return(rdma_create_id, &id);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rpma_info_bind_addr, MOCK_ERRNO);
	/* - deconstructing */
	will_return(rdma_destroy_id, NO_ERROR);

	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_ADDR, MOCK_SERVICE, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), MOCK_ERRNO);
	assert_null(ep);
}

/*
 * ep_listen_test_listen_EAGAIN - rdma_listen() fails with EAGAIN
 */
static void
ep_listen_test_listen_EAGAIN(void **unused)
{
	/*
	 * configure mocks for:
	 * - constructing
	 */
	struct rdma_event_channel evch;
	will_return(rdma_create_event_channel, &evch);
	struct rdma_cm_id id;
	will_return(rdma_create_id, &id);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rpma_info_bind_addr, NO_ERROR);
	will_return(rdma_listen, EAGAIN);
	/* - deconstructing */
	will_return(rdma_destroy_id, NO_ERROR);

	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_ADDR, MOCK_SERVICE, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(ep);
}

/*
 * ep_listen_test_malloc_ENOMEM - malloc() fails with ENOMEM
 */
static void
ep_listen_test_malloc_ENOMEM(void **unused)
{
	/*
	 * configure mocks for:
	 * - constructing
	 */
	will_return(__wrap__test_malloc, ENOMEM);
	/* - things which may happen: */
	struct rdma_event_channel evch;
	will_return_maybe(rdma_create_event_channel, &evch);
	struct rdma_cm_id id;
	will_return_maybe(rdma_create_id, &id);
	will_return_maybe(rpma_info_new, MOCK_INFO);
	will_return_maybe(rpma_info_bind_addr, NO_ERROR);
	will_return_maybe(rdma_listen, NO_ERROR);
	will_return_maybe(rdma_destroy_id, NO_ERROR);

	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_ADDR, MOCK_SERVICE, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(ep);
}

/*
 * ep_listen_test_malloc_ENOMEM_destroy_id_EAGAIN - malloc() fails with ENOMEM
 * rdma_destroy_id() fails consequently during the handling of the first error
 *
 * Note: test assumes rdma_create_id() is called before the first failing
 * malloc()
 */
static void
ep_listen_test_malloc_ENOMEM_destroy_id_EAGAIN(void **unused)
{
	/*
	 * configure mocks for:
	 * - constructing
	 */
	struct rdma_event_channel evch;
	will_return(rdma_create_event_channel, &evch);
	struct rdma_cm_id id;
	will_return(rdma_create_id, &id);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rpma_info_bind_addr, NO_ERROR);
	will_return(rdma_listen, NO_ERROR);
	will_return(__wrap__test_malloc, ENOMEM); /* first error */
	/* - deconstructing */
	will_return(rdma_destroy_id, MOCK_ERRNO); /* second error */

	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_ADDR, MOCK_SERVICE, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(ep);
}

/*
 * ep_shutdown_test_ep_ptr_NULL - NULL ep_ptr is invalid
 */
static void
ep_shutdown_test_ep_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_ep_shutdown(NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * ep_shutdown_test_ep_NULL - NULL ep is valid
 */
static void
ep_shutdown_test_ep_NULL(void **unused)
{
	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_shutdown(&ep);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
	assert_null(ep);
}

/*
 * all the resources used between ep_setup and ep_teardown
 */
struct ep_test_state {
	struct rdma_event_channel evch;
	struct rdma_cm_id cmid;
	struct rpma_ep *ep;
};

/*
 * ep_setup - prepare a valid rpma_ep object
 */
static int
ep_setup(void **estate_ptr)
{
	/* configure mocks: */
	Mock_ctrl_defer_destruction = MOCK_CTRL_DEFER;
	static struct ep_test_state estate = {{0}};
	will_return(rdma_create_event_channel, &estate.evch);
	will_return(rdma_create_id, &estate.cmid);
	will_return(rpma_info_new, MOCK_INFO);
	will_return(rpma_info_bind_addr, NO_ERROR);
	will_return(rdma_listen, NO_ERROR);
	will_return(__wrap__test_malloc, NO_ERROR);
	expect_value(rpma_info_delete, *info_ptr, MOCK_INFO);

	/* prepare an object */
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_ADDR, MOCK_SERVICE,
		&estate.ep);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
	assert_non_null(estate.ep);
	assert_int_equal(memcmp(&estate.cmid, &Cmid_zero, sizeof(estate.cmid)),
		0);
	assert_int_equal(memcmp(&estate.evch, &Evch_zero, sizeof(estate.evch)),
		0);

	*estate_ptr = &estate;

	/* restore default mock configuration */
	Mock_ctrl_defer_destruction = MOCK_CTRL_NO_DEFER;

	return 0;
}

/*
 * ep_teardown - delete the rpma_ep object
 */
static int
ep_teardown(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	/* configure mocks: */
	expect_value(rdma_destroy_id, id, &estate->cmid);
	will_return(rdma_destroy_id, NO_ERROR);
	expect_value(rdma_destroy_event_channel, channel, &estate->evch);

	/* delete the object */
	int ret = rpma_ep_shutdown(&estate->ep);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
	assert_null(estate->ep);
	assert_int_equal(memcmp(&estate->cmid, &Cmid_zero,
		sizeof(estate->cmid)), 0);
	assert_int_equal(memcmp(&estate->evch, &Evch_zero,
		sizeof(estate->evch)), 0);

	return 0;
}

/*
 * ep_test_lifecycle - happy day scenario
 */
static void
ep_test_lifecycle(void **unused)
{
	/* the thing is done by ep_setup() and ep_teardown() */
}

/*
 * ep_shutdown_test_destroy_id_EAGAIN -- rdma_destroy_id() fails with EAGAIN
 */
static void
ep_shutdown_test_destroy_id_EAGAIN(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	/* configure mocks */
	expect_value(rdma_destroy_id, id, &estate->cmid);
	will_return(rdma_destroy_id, EAGAIN);

	/* run test */
	int ret = rpma_ep_shutdown(&estate->ep);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_non_null(estate->ep);
	assert_int_equal(memcmp(&estate->cmid, &Cmid_zero,
		sizeof(estate->cmid)), 0);
	assert_int_equal(memcmp(&estate->evch, &Evch_zero,
		sizeof(estate->evch)), 0);
}

/*
 * ep_next_conn_req_test_ep_NULL - NULL ep is invalid
 */
static void
ep_next_conn_req_test_ep_NULL(void **unused)
{
	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_ep_next_conn_req(NULL, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * ep_next_conn_req_test_req_NULL - NULL req is invalid
 */
static void
ep_next_conn_req_test_req_NULL(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	/* run test */
	int ret = rpma_ep_next_conn_req(estate->ep, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * ep_next_conn_req_test_ep_NULL_req_NULL - NULL ep and NULL req are invalid
 */
static void
ep_next_conn_req_test_ep_NULL_req_NULL(void **unused)
{
	/* run test */
	int ret = rpma_ep_next_conn_req(NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * ep_next_conn_req_test_get_cm_event_EAGAIN -
 * rdma_get_cm_event() fails with EAGAIN
 */
static void
ep_next_conn_req_test_get_cm_event_EAGAIN(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	expect_value(rdma_get_cm_event, channel, &estate->evch);
	will_return(rdma_get_cm_event, NULL);
	will_return(rdma_get_cm_event, EAGAIN);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_ep_next_conn_req(estate->ep, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_null(req);
}

/*
 * ep_next_conn_req_test_event_REJECTED -
 * RDMA_CM_EVENT_REJECTED is unexpected
 */
static void
ep_next_conn_req_test_event_REJECTED(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	expect_value(rdma_get_cm_event, channel, &estate->evch);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_REJECTED;
	will_return(rdma_get_cm_event, &event);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, NO_ERROR);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_ep_next_conn_req(estate->ep, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * ep_next_conn_req_test_event_REJECTED_ack_EINVAL -
 * rdma_ack_cm_event() fails with EINVAL after obtaining
 * an RDMA_CM_EVENT_REJECTED event (!= RDMA_CM_EVENT_CONNECT_REQUEST)
 */
static void
ep_next_conn_req_test_event_REJECTED_ack_EINVAL(void **estate_ptr)
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
	int ret = rpma_ep_next_conn_req(estate->ep, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(req);
}

/*
 * ep_next_conn_req_test_conn_req_from_cm_event_ENOMEM -
 * rpma_conn_req_from_cm_event() fails with ENOMEM
 */
static void
ep_next_conn_req_test_conn_req_from_cm_event_ENOMEM(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	expect_value(rdma_get_cm_event, channel, &estate->evch);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_CONNECT_REQUEST;
	will_return(rdma_get_cm_event, &event);

	expect_value(rpma_conn_req_from_cm_event, peer, MOCK_PEER);
	expect_value(rpma_conn_req_from_cm_event, edata, &event);
	will_return(rpma_conn_req_from_cm_event, NULL);
	will_return(rpma_conn_req_from_cm_event, RPMA_E_NOMEM);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, NO_ERROR);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_ep_next_conn_req(estate->ep, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * ep_next_conn_req_test_from_cm_event_ENOMEM_ack_EINVAL -
 * rpma_conn_req_from_cm_event() fails and
 * rdma_ack_cm_event() fails with EINVAL
 */
static void
ep_next_conn_req_test_from_cm_event_ENOMEM_ack_EINVAL(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	expect_value(rdma_get_cm_event, channel, &estate->evch);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_CONNECT_REQUEST;
	will_return(rdma_get_cm_event, &event);

	expect_value(rpma_conn_req_from_cm_event, peer, MOCK_PEER);
	expect_value(rpma_conn_req_from_cm_event, edata, &event);
	will_return(rpma_conn_req_from_cm_event, NULL);
	will_return(rpma_conn_req_from_cm_event, RPMA_E_NOMEM);

	expect_value(rdma_ack_cm_event, event, &event);
	will_return(rdma_ack_cm_event, EINVAL);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_ep_next_conn_req(estate->ep, &req);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(req);
}

/*
 * ep_next_conn_req_test_success - happy day scenario
 */
static void
ep_next_conn_req_test_success(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	expect_value(rdma_get_cm_event, channel, &estate->evch);
	struct rdma_cm_event event;
	event.event = RDMA_CM_EVENT_CONNECT_REQUEST;
	will_return(rdma_get_cm_event, &event);

	expect_value(rpma_conn_req_from_cm_event, peer, MOCK_PEER);
	expect_value(rpma_conn_req_from_cm_event, edata, &event);
	will_return(rpma_conn_req_from_cm_event, MOCK_CONN_REQ);

	/* run test */
	struct rpma_conn_req *req = NULL;
	int ret = rpma_ep_next_conn_req(estate->ep, &req);

	/* verify the results */
	assert_ptr_equal(req, MOCK_CONN_REQ);
	assert_int_equal(ret, 0);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_ep_listen() unit tests */
		cmocka_unit_test(ep_listen_test_peer_NULL),
		cmocka_unit_test(ep_listen_test_addr_NULL),
		cmocka_unit_test(ep_listen_test_service_NULL),
		cmocka_unit_test(ep_listen_test_ep_ptr_NULL),
		cmocka_unit_test(ep_listen_test_peer_addr_service_ep_ptr_NULL),
		cmocka_unit_test(ep_listen_test_create_evch_EAGAIN),
		cmocka_unit_test(ep_listen_test_create_id_EAGAIN),
		cmocka_unit_test(ep_listen_test_info_new_E_NOMEM),
		cmocka_unit_test(ep_listen_test_info_bind_addr_E_PROVIDER),
		cmocka_unit_test(ep_listen_test_listen_EAGAIN),
		cmocka_unit_test(ep_listen_test_malloc_ENOMEM),
		cmocka_unit_test(
			ep_listen_test_malloc_ENOMEM_destroy_id_EAGAIN),

		/* rpma_ep_listen()/_shutdown() lifecycle */
		cmocka_unit_test_setup_teardown(ep_test_lifecycle,
			ep_setup, ep_teardown),

		/* rpma_ep_shutdown() unit tests */
		cmocka_unit_test(ep_shutdown_test_ep_ptr_NULL),
		cmocka_unit_test(ep_shutdown_test_ep_NULL),
		cmocka_unit_test_setup_teardown(
			ep_shutdown_test_destroy_id_EAGAIN,
			ep_setup, ep_teardown),

		/* rpma_ep_next_conn_req() unit tests */
		cmocka_unit_test(ep_next_conn_req_test_ep_NULL),
		cmocka_unit_test_setup_teardown(
			ep_next_conn_req_test_req_NULL,
			ep_setup, ep_teardown),
		cmocka_unit_test(ep_next_conn_req_test_ep_NULL_req_NULL),
		cmocka_unit_test_setup_teardown(
			ep_next_conn_req_test_get_cm_event_EAGAIN,
			ep_setup, ep_teardown),
		cmocka_unit_test_setup_teardown(
			ep_next_conn_req_test_event_REJECTED,
			ep_setup, ep_teardown),
		cmocka_unit_test_setup_teardown(
			ep_next_conn_req_test_event_REJECTED_ack_EINVAL,
			ep_setup, ep_teardown),
		cmocka_unit_test_setup_teardown(
			ep_next_conn_req_test_conn_req_from_cm_event_ENOMEM,
			ep_setup, ep_teardown),
		cmocka_unit_test_setup_teardown(
			ep_next_conn_req_test_from_cm_event_ENOMEM_ack_EINVAL,
			ep_setup, ep_teardown),
		cmocka_unit_test_setup_teardown(
			ep_next_conn_req_test_success,
			ep_setup, ep_teardown),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
