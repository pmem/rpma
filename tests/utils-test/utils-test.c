/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * utils-test.c -- a unit test for rpma_utils_get_ibv_context()
 */

#include <stdio.h>
#include <stdlib.h>
#include <rdma/rdma_cma.h>

#include "cmocka_headers.h"
#include "librpma.h"
#include "info.h"
#include "rpma_err.h"

#define IP_ADDRESS	"127.0.0.1"

/* any not-NULL values */
#define MOCK_INFO	((struct rpma_info *)0xABC0)
#define MOCK_VERBS	((struct ibv_context *)0xABC1)

#define TYPE_UNKNOWN (enum rpma_util_ibv_context_type)(-1)

#define ANY_ERRNO	0xFE00 /* mock errno value */

/*
 * rpma_info_new -- mock of rpma_info_new
 */
int
rpma_info_new(const char *addr, const char *service, enum rpma_info_side side,
		struct rpma_info **info_ptr)
{
	/*
	 * rpma_info_new() and rdma_create_id() may be called in any order.
	 * If the first one fails, then the second one won't be called,
	 * so we cannot add cmocka's expects here.
	 * Otherwise, unconsumed expects would cause a test failure.
	 */
	assert_string_equal(addr, IP_ADDRESS);
	assert_null(service);
	assert_true(side == RPMA_INFO_PASSIVE || side == RPMA_INFO_ACTIVE);

	*info_ptr = mock_type(struct rpma_info *);
	if (*info_ptr == NULL) {
		int result = mock_type(int);
		if (result == RPMA_E_PROVIDER)
			Rpma_provider_error = mock_type(int);

		return result;
	}

	expect_value(rpma_info_delete, *info_ptr, *info_ptr);

	return 0;
}

/*
 * rdma_create_id -- mock of rdma_create_id
 */
int
rdma_create_id(struct rdma_event_channel *channel,
		struct rdma_cm_id **id, void *context,
		enum rdma_port_space ps)
{
	/*
	 * rpma_info_new() and rdma_create_id() may be called in any order.
	 * If the first one fails, then the second one won't be called,
	 * so we cannot add cmocka's expects here.
	 * Otherwise, unconsumed expects would cause a test failure.
	 */
	assert_non_null(id);
	assert_null(context);
	assert_int_equal(ps, RDMA_PS_TCP);

	/* allocate (struct rdma_cm_id *) */
	*id = mock_type(struct rdma_cm_id *);
	if (*id == NULL) {
		errno = mock_type(int);
		return -1;
	}

	expect_value(rdma_destroy_id, id, *id);

	return 0;
}

/*
 * rpma_info_assign_addr -- mock of rpma_info_asign_addr
 */
int
rpma_info_assign_addr(const struct rpma_info *info, struct rdma_cm_id *id)
{
	check_expected(info);
	check_expected(id);

	int ret = mock_type(int);
	if (ret)
		Rpma_provider_error = mock_type(int);

	return ret;
}

/*
 * rpma_info_assign_addr -- mock of rpma_info_assign_addr
 */
#if 0
int
rpma_info_assign_addr(const struct rpma_info *info, struct rdma_cm_id *id)
{
	check_expected(info);
	check_expected(id);

	int ret = mock_type(int);
	if (ret)
		Rpma_provider_error = mock_type(int);

	return ret;
}

/*
 * rpma_info_assign_addr -- mock of rpma_info_assign_addr
 */
int
rpma_info_assign_addr(const struct rpma_info *info, struct rdma_cm_id *id)
{
	check_expected(info);
	check_expected(id);

	int ret = mock_type(int);
	if (ret)
		Rpma_provider_error = mock_type(int);

	return ret;
}
#endif
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
 * rpma_info_delete -- mock of rpma_info_delete
 */
int
rpma_info_delete(struct rpma_info **info_ptr)
{
	if (info_ptr == NULL)
		return RPMA_E_INVAL;

	check_expected(*info_ptr);

	return 0;
}

/*
 * sanity_test_type_unknown - TYPE_UNKNOWN != RPMA_UTIL_IBV_CONTEXT_LOCAL &&
 * TYPE_UNKNOWN != RPMA_UTIL_IBV_CONTEXT_REMOTE
 */
static void
sanity_test_type_unknown(void **unused)
{
	/* run test */
	assert_int_not_equal(TYPE_UNKNOWN, RPMA_UTIL_IBV_CONTEXT_LOCAL);
	assert_int_not_equal(TYPE_UNKNOWN, RPMA_UTIL_IBV_CONTEXT_REMOTE);
}

/*
 * test_addr_NULL - test NULL addr parameter
 */
static void
test_addr_NULL(void **unused)
{
	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(NULL, RPMA_UTIL_IBV_CONTEXT_REMOTE,
			&dev);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(dev);
}

/*
 * test_dev_NULL - test NULL dev parameter
 */
static void
test_dev_NULL(void **unused)
{
	/* run test */
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_type_unknown - type == TYPE_UNKNOWN
 */
static void
test_type_unknown(void **unused)
{
	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS, TYPE_UNKNOWN, &dev);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_addr_NULL_dev_NULL_type_unknown - test NULL addr and dev parameter and
 * type == TYPE_UNKNOWN
 */
static void
test_addr_NULL_dev_NULL_type_unknown(void **unused)
{
	/* run test */
	int ret = rpma_utils_get_ibv_context(NULL, TYPE_UNKNOWN, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_info_new_failed_E_PROVIDER - rpma_info_new fails with RPMA_E_PROVIDER
 */
static void
test_info_new_failed_E_PROVIDER(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, NULL /* info_ptr */);
	will_return(rpma_info_new, RPMA_E_PROVIDER);
	will_return(rpma_info_new, ANY_ERRNO);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return_maybe(rdma_create_id, &id);
	will_return_maybe(rdma_destroy_id, 0);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &dev);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), ANY_ERRNO);
	assert_null(dev);
}

/*
 * test_info_new_failed_E_NOMEM - rpma_info_new fails with RPMA_E_NOMEM
 */
static void
test_info_new_failed_E_NOMEM(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, NULL /* info_ptr */);
	will_return(rpma_info_new, RPMA_E_NOMEM);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return_maybe(rdma_create_id, &id);
	will_return_maybe(rdma_destroy_id, 0);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &dev);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(dev);
}

/*
 * test_create_id_failed - rdma_create_id fails with ANY_ERRNO
 */
static void
test_create_id_failed(void **unused)
{
	/*
	 * Configure mocks.
	 * We assume it is not important which call of rpma_info_new()
	 * succeeds (active or passive), since failing rdma_create_id()
	 * should look and behaves the same.
	 * Here we assume that if rpma_info_new() is called, it will succeed
	 * for a local address (passive side).
	 */
	will_return_maybe(rpma_info_new, MOCK_INFO);
	will_return(rdma_create_id, NULL);
	will_return(rdma_create_id, ANY_ERRNO);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_LOCAL, &dev);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), ANY_ERRNO);
	assert_null(dev);
}

/*
 * test_bind_addr_failed_E_PROVIDER - rpma_info_assign_addr fails
 *                                    with RPMA_E_PROVIDER
 */
static void
test_bind_addr_failed_E_PROVIDER(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return(rdma_create_id, &id);

	/* a local address (passive side) */
	expect_value(rpma_info_assign_addr, info, MOCK_INFO);
	expect_value(rpma_info_assign_addr, id, &id);
	will_return(rpma_info_assign_addr, RPMA_E_PROVIDER);
	will_return(rpma_info_assign_addr, ANY_ERRNO);

	will_return(rdma_destroy_id, 0);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_LOCAL, &dev);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), ANY_ERRNO);
	assert_null(dev);
}

/*
 * test_resolve_addr_failed_E_PROVIDER - rpma_info_assign_addr fails
 *                                       with RPMA_E_PROVIDER
 */
static void
test_resolve_addr_failed_E_PROVIDER(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return(rdma_create_id, &id);

	/* a remote address (active side) */
	expect_value(rpma_info_assign_addr, info, MOCK_INFO);
	expect_value(rpma_info_assign_addr, id, &id);
	will_return(rpma_info_assign_addr, RPMA_E_PROVIDER);
	will_return(rpma_info_assign_addr, ANY_ERRNO);

	will_return(rdma_destroy_id, 0);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &dev);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), ANY_ERRNO);
	assert_null(dev);
}

/*
 * test_success_destroy_id_failed_passive - test if rpma_utils_get_ibv_context()
 *                                          succeeds if rdma_destroy_id() fails
 */
static void
test_success_destroy_id_failed_passive(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return(rdma_create_id, &id);

	/* a local address (passive side) */
	expect_value(rpma_info_assign_addr, info, MOCK_INFO);
	expect_value(rpma_info_assign_addr, id, &id);
	will_return(rpma_info_assign_addr, 0);

	will_return(rdma_destroy_id, ANY_ERRNO);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_LOCAL, &dev);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_ptr_equal(dev, MOCK_VERBS);
}

/*
 * test_success_destroy_id_failed_active - test if rpma_utils_get_ibv_context()
 *                                         succeeds if rdma_destroy_id() fails
 */
static void
test_success_destroy_id_failed_active(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return(rdma_create_id, &id);

	/* a remote address (active side) */
	expect_value(rpma_info_assign_addr, info, MOCK_INFO);
	expect_value(rpma_info_assign_addr, id, &id);
	will_return(rpma_info_assign_addr, 0);

	will_return(rdma_destroy_id, ANY_ERRNO);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &dev);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_ptr_equal(dev, MOCK_VERBS);
}

/*
 * test_success_passive - test the 'all is OK' situation
 */
static void
test_success_passive(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return(rdma_create_id, &id);

	expect_value(rpma_info_assign_addr, info, MOCK_INFO);
	expect_value(rpma_info_assign_addr, id, &id);
	will_return(rpma_info_assign_addr, 0);

	will_return(rdma_destroy_id, 0);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_LOCAL, &dev);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_ptr_equal(dev, MOCK_VERBS);
}

/*
 * test_success_active - test the 'all is OK' situation
 */
static void
test_success_active(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return(rdma_create_id, &id);

	expect_value(rpma_info_assign_addr, info, MOCK_INFO);
	expect_value(rpma_info_assign_addr, id, &id);
	will_return(rpma_info_assign_addr, 0);

	will_return(rdma_destroy_id, 0);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &dev);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_ptr_equal(dev, MOCK_VERBS);
}

/*
 * event_2str_test_CONN_UNDEFINED - sanity test for
 * rpma_utils_conn_event_2str()
 */
static void
event_2str_test_CONN_UNDEFINED(void **unused)
{
	assert_string_equal(rpma_utils_conn_event_2str(RPMA_CONN_UNDEFINED),
	"Undefined connection event");
}

/*
 * event_2str_test_CONN_ESTABLISHED - sanity test for
 * rpma_utils_conn_event_2str()
 */
static void
event_2str_test_CONN_ESTABLISHED(void **unused)
{
	assert_string_equal(rpma_utils_conn_event_2str(RPMA_CONN_ESTABLISHED),
	"Connection established");
}

/*
 * event_2str_test_CONN_CLOSED - sanity test for
 * rpma_utils_conn_event_2str()
 */
static void
event_2str_test_CONN_CLOSED(void **unused)
{
	assert_string_equal(rpma_utils_conn_event_2str(RPMA_CONN_CLOSED),
	"Connection closed");
}

/*
 * event_2str_test_CONN_LOST - sanity test for
 * rpma_utils_conn_event_2str()
 */
static void
event_2str_test_CONN_LOST(void **unused)
{
	assert_string_equal(rpma_utils_conn_event_2str(RPMA_CONN_LOST),
	"Connection lost");
}

/*
 * event_2str_test_CONN_UNKOWN - sanity test for
 * rpma_utils_conn_event_2str()
 */
static void
event_2str_test_CONN_UNKNOWN(void **unused)
{
	assert_string_equal(rpma_utils_conn_event_2str(RPMA_E_UNKNOWN),
	"Unknown connection event");
}
int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* sanity */
		cmocka_unit_test(sanity_test_type_unknown),

		cmocka_unit_test(test_addr_NULL),
		cmocka_unit_test(test_dev_NULL),
		cmocka_unit_test(test_type_unknown),
		cmocka_unit_test(test_addr_NULL_dev_NULL_type_unknown),
		cmocka_unit_test(test_info_new_failed_E_PROVIDER),
		cmocka_unit_test(test_info_new_failed_E_NOMEM),
		cmocka_unit_test(test_create_id_failed),
		cmocka_unit_test(test_bind_addr_failed_E_PROVIDER),
		cmocka_unit_test(test_resolve_addr_failed_E_PROVIDER),
		cmocka_unit_test(test_success_destroy_id_failed_passive),
		cmocka_unit_test(test_success_destroy_id_failed_active),
		cmocka_unit_test(test_success_passive),
		cmocka_unit_test(test_success_active),

		/* rpma_utils_conn_event_2str() unit tests */
		cmocka_unit_test(event_2str_test_CONN_UNDEFINED),
		cmocka_unit_test(event_2str_test_CONN_ESTABLISHED),
		cmocka_unit_test(event_2str_test_CONN_CLOSED),
		cmocka_unit_test(event_2str_test_CONN_LOST),
		cmocka_unit_test(event_2str_test_CONN_UNKNOWN),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
