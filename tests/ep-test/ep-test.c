/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * ep-test.c -- an endpoint unit test
 */

#include <stdlib.h>

#include "cmocka_headers.h"
#include "conn_req.h"
#include "info.h"
#include "librpma.h"
#include "rpma_err.h"

#include <infiniband/verbs.h>

#define MOCK_ADDR		"mock_addr"
#define MOCK_SERVICE	"mock_service"
#define MOCK_PEER		(struct rpma_peer *)0xFEEF
#define MOCK_INFO		(struct rpma_info *)0x14F0

#define NO_ERROR		0
#define MOCK_ERRNO		0xE440

/*
 * rdma_create_event_channel -- rdma_create_event_channel() mock
 */
struct rdma_event_channel *
rdma_create_event_channel(void)
{
	struct rdma_event_channel *evch =
			mock_type(struct rdma_event_channel *);

	if (evch == NULL)
		errno = mock_type(int);

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

	return 0;
}

struct generic_mock_args {
	int validate_params;
	int result;
};

/*
 * rdma_destroy_id -- rdma_destroy_id() mock
 */
int
rdma_destroy_id(struct rdma_cm_id *id)
{
	check_expected_ptr(id);

	errno = mock_type(int);

	if (errno != 0)
		return -1;

	return 0;
}

struct rpma_info_new_args {
	int validate_params;
	struct rpma_info *info;
};

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

	return 0;
}

/*
 * rpma_info_delete -- rpma_info_delete() mock
 */
int
rpma_info_delete(struct rpma_info **info_ptr)
{
	assert_non_null(info_ptr);
	assert_ptr_equal(*info_ptr, MOCK_INFO);
	*info_ptr = NULL;

	/* if arg is valid this function cannot fail otherwise */
	return 0;
}

/*
 * rpma_info_bind_addr -- rpma_info_bind_addr() mock
 */
int
rpma_info_bind_addr(struct rpma_info *info, struct rdma_cm_id *id)
{
	check_expected_ptr(id);
	assert_ptr_equal(info, MOCK_INFO);

	Rpma_provider_error = mock_type(int);

	if (Rpma_provider_error == 0)
		return 0;

	return RPMA_E_PROVIDER;
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

	if (errno != NO_ERROR)
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

	if (errno != NO_ERROR)
		return NULL;

	return __real__test_malloc(size);
}

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
	 * configure mocks:
	 * NOTE: it is assumed ep won't create a CM ID before creating an event
	 * channel.
	 */
	will_return(rdma_create_event_channel, NULL);
	will_return(rdma_create_event_channel, EAGAIN);

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
	 * NOTE: it is assumed ep won't create an rpma_info before creating a
	 * CM ID.
	 * - constructing
	 */
	struct rdma_event_channel evch;
	will_return(rdma_create_event_channel, &evch);
	expect_value(rdma_create_id, channel, &evch);
	will_return(rdma_create_id, NULL);
	will_return(rdma_create_id, EAGAIN);
	/* - deconstructing */
	expect_value(rdma_destroy_event_channel, channel, &evch);

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
	struct rdma_event_channel evch;
	will_return(rdma_create_event_channel, &evch);
	struct rdma_cm_id id;
	expect_value(rdma_create_id, channel, &evch);
	will_return(rdma_create_id, &id);
	will_return(rpma_info_new, NULL);
	will_return(rpma_info_new, RPMA_E_NOMEM);
	/* - deconstructing */
	expect_value(rdma_destroy_id, id, &id);
	will_return(rdma_destroy_id, NO_ERROR);
	expect_value(rdma_destroy_event_channel, channel, &evch);

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
	expect_value(rdma_create_id, channel, &evch);
	will_return(rdma_create_id, &id);
	will_return(rpma_info_new, MOCK_INFO);
	expect_value(rpma_info_bind_addr, id, &id);
	will_return(rpma_info_bind_addr, MOCK_ERRNO);
	/* - deconstructing */
	expect_value(rdma_destroy_id, id, &id);
	will_return(rdma_destroy_id, NO_ERROR);
	expect_value(rdma_destroy_event_channel, channel, &evch);

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
	expect_value(rdma_create_id, channel, &evch);
	will_return(rdma_create_id, &id);
	will_return(rpma_info_new, MOCK_INFO);
	expect_value(rpma_info_bind_addr, id, &id);
	will_return(rpma_info_bind_addr, NO_ERROR);
	expect_value(rdma_listen, id, &id);
	will_return(rdma_listen, EAGAIN);
	/* - deconstructing */
	expect_value(rdma_destroy_id, id, &id);
	will_return(rdma_destroy_id, NO_ERROR);
	expect_value(rdma_destroy_event_channel, channel, &evch);

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
	struct rdma_event_channel evch;
	will_return(rdma_create_event_channel, &evch);
	struct rdma_cm_id id;
	expect_value(rdma_create_id, channel, &evch);
	will_return(rdma_create_id, &id);
	will_return(rpma_info_new, MOCK_INFO);
	expect_value(rpma_info_bind_addr, id, &id);
	will_return(rpma_info_bind_addr, NO_ERROR);
	expect_value(rdma_listen, id, &id);
	will_return(rdma_listen, NO_ERROR);
	will_return(__wrap__test_malloc, ENOMEM);
	/* - deconstructing */
	expect_value(rdma_destroy_id, id, &id);
	will_return(rdma_destroy_id, NO_ERROR);
	expect_value(rdma_destroy_event_channel, channel, &evch);

	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_ADDR, MOCK_SERVICE, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(ep);
}

/*
 * ep_test_lifecycle - happy day scenario
 */
static void
ep_test_lifecycle(void **unused)
{
	/* configure mocks: */
	struct rdma_event_channel evch;
	will_return(rdma_create_event_channel, &evch);
	struct rdma_cm_id id;
	expect_value(rdma_create_id, channel, &evch);
	will_return(rdma_create_id, &id);
	will_return(rpma_info_new, MOCK_INFO);
	expect_value(rpma_info_bind_addr, id, &id);
	will_return(rpma_info_bind_addr, NO_ERROR);
	expect_value(rdma_listen, id, &id);
	will_return(rdma_listen, NO_ERROR);
	will_return(__wrap__test_malloc, NO_ERROR);

	/* run test - step 1 */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_ADDR, MOCK_SERVICE, &ep);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
	assert_non_null(ep);

	/* configure mocks: */
	expect_value(rdma_destroy_id, id, &id);
	will_return(rdma_destroy_id, NO_ERROR);
	expect_value(rdma_destroy_event_channel, channel, &evch);

	/* run test - step 2 */
	ret = rpma_ep_shutdown(&ep);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
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
 * rdma_event_channel and rdma_cm_id used from setup / teardown steps
 */
static struct rdma_event_channel Evch;
static struct rdma_cm_id Cmid;

/*
 * ep_setup - prepare a valid rpma_ep object
 */
static int
ep_setup(void **ep_ptr)
{
	/* configure mocks: */
	will_return(rdma_create_event_channel, &Evch);
	expect_value(rdma_create_id, channel, &Evch);
	will_return(rdma_create_id, &Cmid);
	will_return(rpma_info_new, MOCK_INFO);
	expect_value(rpma_info_bind_addr, id, &Cmid);
	will_return(rpma_info_bind_addr, NO_ERROR);
	expect_value(rdma_listen, id, &Cmid);
	will_return(rdma_listen, NO_ERROR);
	will_return(__wrap__test_malloc, NO_ERROR);

	/* prepare an object */
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_ADDR, MOCK_SERVICE,
			(struct rpma_ep **)ep_ptr);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
	assert_non_null(*ep_ptr);

	return 0;
}

/*
 * ep_teardown - delete the rpma_ep object
 */
static int
ep_teardown(void **ep_ptr)
{
	/* configure mocks: */
	expect_value(rdma_destroy_id, id, &Cmid);
	will_return(rdma_destroy_id, NO_ERROR);
	expect_value(rdma_destroy_event_channel, channel, &Evch);

	/* delete the object */
	int ret = rpma_ep_shutdown((struct rpma_ep **)ep_ptr);

	/* verify the results */
	assert_int_equal(ret, NO_ERROR);
	assert_null(*ep_ptr);

	return 0;
}

/*
 * ep_shutdown_test_destroy_id_EAGAIN -- rdma_destroy_id() fails with EAGAIN
 */
static void
ep_shutdown_test_destroy_id_EAGAIN(void **ep_ptr)
{
	/* configure mocks */
	expect_value(rdma_destroy_id, id, &Cmid);
	will_return(rdma_destroy_id, EAGAIN);

	/* run test */
	int ret = rpma_ep_shutdown((struct rpma_ep **)ep_ptr);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), EAGAIN);
	assert_non_null(*ep_ptr);
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

		/* rpma_ep_listen()/_shutdown() lifecycle */
		cmocka_unit_test(ep_test_lifecycle),

		/* rpma_ep_shutdown() unit tests */
		cmocka_unit_test(ep_shutdown_test_ep_ptr_NULL),
		cmocka_unit_test(ep_shutdown_test_ep_NULL),
		cmocka_unit_test_setup_teardown(
			ep_shutdown_test_destroy_id_EAGAIN,
			ep_setup, ep_teardown),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
