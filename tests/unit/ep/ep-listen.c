// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */
/* Copyright 2021, Fujitsu */

/*
 * ep-listen.c -- the endpoint unit tests
 *
 * APIs covered:
 * - rpma_ep_listen()
 * - rpma_ep_shutdown()
 */

#include "librpma.h"
#include "ep-common.h"
#include "cmocka_headers.h"
#include "test-common.h"

/*
 * listen__peer_NULL - NULL peer is invalid
 */
static void
listen__peer_NULL(void **unused)
{
	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(NULL, MOCK_IP_ADDRESS, MOCK_PORT, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(ep);
}

/*
 * listen__addr_NULL - NULL addr is invalid
 */
static void
listen__addr_NULL(void **unused)
{
	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, NULL, MOCK_PORT, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(ep);
}

/*
 * listen__port_NULL - NULL port is invalid
 */
static void
listen__port_NULL(void **unused)
{
	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_IP_ADDRESS, NULL, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(ep);
}

/*
 * listen__ep_ptr_NULL - NULL ep_ptr is invalid
 */
static void
listen__ep_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * listen__peer_addr_port_ep_ptr_NULL - peer, addr, port
 * and ep_ptr == NULL are invalid
 */
static void
listen__peer_addr_port_ep_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_ep_listen(NULL, NULL, NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * listen__create_evch_ERRNO - rdma_create_event_channel() fails
 * with MOCK_ERRNO
 */
static void
listen__create_evch_ERRNO(void **unused)
{
	/*
	 * configure mocks for:
	 * - constructing:
	 */
	will_return(rdma_create_event_channel, NULL);
	will_return(rdma_create_event_channel, MOCK_ERRNO);
	/* - things which may happen: */
	will_return_maybe(rpma_info_new, MOCK_INFO);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(ep);
}

/*
 * listen__create_id_ERRNO - rdma_create_id() fails with MOCK_ERRNO
 */
static void
listen__create_id_ERRNO(void **unused)
{
	/*
	 * configure mocks:
	 * - constructing
	 */
	struct rdma_event_channel evch;
	will_return(rdma_create_event_channel, &evch);
	will_return(rdma_create_id, NULL);
	will_return(rdma_create_id, MOCK_ERRNO);
	/* - things which may happen: */
	will_return_maybe(rpma_info_new, MOCK_INFO);
	will_return_maybe(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(ep);
}

/*
 * listen__info_new_E_NOMEM - rpma_info_new() returns RPMA_E_NOMEM
 */
static void
listen__info_new_E_NOMEM(void **unused)
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
	will_return_maybe(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(ep);
}

/*
 * listen__info_bind_addr_E_PROVIDER - rpma_info_bind_addr() returns
 * RPMA_E_PROVIDER
 */
static void
listen__info_bind_addr_E_PROVIDER(void **unused)
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
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(ep);
}

/*
 * listen__listen_ERRNO - rdma_listen() fails with MOCK_ERRNO
 */
static void
listen__listen_ERRNO(void **unused)
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
	will_return(rpma_info_bind_addr, MOCK_OK);
	will_return(rdma_listen, MOCK_ERRNO);
	/* - deconstructing */
	will_return(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(ep);
}

/*
 * listen__malloc_ERRNO - malloc() fails with MOCK_ERRNO
 */
static void
listen__malloc_ERRNO(void **unused)
{
	/*
	 * configure mocks for:
	 * - constructing
	 */
	will_return(__wrap__test_malloc, MOCK_ERRNO);
	/* - things which may happen: */
	struct rdma_event_channel evch;
	will_return_maybe(rdma_create_event_channel, &evch);
	struct rdma_cm_id id;
	will_return_maybe(rdma_create_id, &id);
	will_return_maybe(rpma_info_new, MOCK_INFO);
	will_return_maybe(rpma_info_bind_addr, MOCK_OK);
	will_return_maybe(rdma_listen, MOCK_OK);
	will_return_maybe(rdma_destroy_id, MOCK_OK);

	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(ep);
}

/*
 * listen__malloc_ERRNO_destroy_id_ERRNO2 - malloc() fails with MOCK_ERRNO
 * rdma_destroy_id() fails with MOCK_ERRNO2 consequently during the handling
 * of the first error
 *
 * Note: test assumes rdma_create_id() is called before the first failing
 * malloc()
 */
static void
listen__malloc_ERRNO_destroy_id_ERRNO2(void **unused)
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
	will_return(rpma_info_bind_addr, MOCK_OK);
	will_return(rdma_listen, MOCK_OK);
	will_return(__wrap__test_malloc, MOCK_ERRNO); /* first error */
	/* - deconstructing */
	will_return(rdma_destroy_id, MOCK_ERRNO2); /* second error */

	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_listen(MOCK_PEER, MOCK_IP_ADDRESS, MOCK_PORT, &ep);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(ep);
}

/*
 * shutdown__ep_ptr_NULL - NULL ep_ptr is invalid
 */
static void
shutdown__ep_ptr_NULL(void **unused)
{
	/* run test */
	int ret = rpma_ep_shutdown(NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * shutdown__ep_NULL - NULL ep is valid
 */
static void
shutdown__ep_NULL(void **unused)
{
	/* run test */
	struct rpma_ep *ep = NULL;
	int ret = rpma_ep_shutdown(&ep);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_null(ep);
}

/*
 * ep__lifecycle - happy day scenario
 */
static void
ep__lifecycle(void **unused)
{
	/*
	 * The thing is done by setup__ep_listen()
	 * and teardown__ep_shutdown().
	 */
}

/*
 * shutdown__destroy_id_ERRNO -- rdma_destroy_id() fails with MOCK_ERRNO
 */
static void
shutdown__destroy_id_ERRNO(void **estate_ptr)
{
	struct ep_test_state *estate = *estate_ptr;

	/* configure mocks */
	expect_value(rdma_destroy_id, id, &estate->cmid);
	will_return(rdma_destroy_id, MOCK_ERRNO);

	/* run test */
	int ret = rpma_ep_shutdown(&estate->ep);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_non_null(estate->ep);
	assert_int_equal(memcmp(&estate->cmid, &Cmid_zero,
		sizeof(estate->cmid)), 0);
	assert_int_equal(memcmp(&estate->evch, &Evch_zero,
		sizeof(estate->evch)), 0);
}

int
main(int argc, char *argv[])
{
	/* prepare prestates */
	struct ep_test_state prestate_conn_cfg_default;
	prestate_init(&prestate_conn_cfg_default, NULL);

	const struct CMUnitTest tests[] = {
		/* rpma_ep_listen() unit tests */
		cmocka_unit_test(listen__peer_NULL),
		cmocka_unit_test(listen__addr_NULL),
		cmocka_unit_test(listen__port_NULL),
		cmocka_unit_test(listen__ep_ptr_NULL),
		cmocka_unit_test(listen__peer_addr_port_ep_ptr_NULL),
		cmocka_unit_test(listen__create_evch_ERRNO),
		cmocka_unit_test(listen__create_id_ERRNO),
		cmocka_unit_test(listen__info_new_E_NOMEM),
		cmocka_unit_test(listen__info_bind_addr_E_PROVIDER),
		cmocka_unit_test(listen__listen_ERRNO),
		cmocka_unit_test(listen__malloc_ERRNO),
		cmocka_unit_test(
			listen__malloc_ERRNO_destroy_id_ERRNO2),

		/* rpma_ep_listen()/_shutdown() lifecycle */
		cmocka_unit_test_prestate_setup_teardown(ep__lifecycle,
			setup__ep_listen, teardown__ep_shutdown,
			&prestate_conn_cfg_default),

		/* rpma_ep_shutdown() unit tests */
		cmocka_unit_test(shutdown__ep_ptr_NULL),
		cmocka_unit_test(shutdown__ep_NULL),
		cmocka_unit_test_prestate_setup_teardown(
			shutdown__destroy_id_ERRNO,
			setup__ep_listen, teardown__ep_shutdown,
			&prestate_conn_cfg_default),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
