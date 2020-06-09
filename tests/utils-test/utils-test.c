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

/* mock error values */
#define ANY_RPMA_ERROR	0xFE00
#define ANY_ERRNO	0xFE01

/*
 * rpma_info_new -- mock of rpma_info_new
 */
int
rpma_info_new(const char *addr, const char *service, enum rpma_info_side side,
		struct rpma_info **info_ptr)
{
	assert_string_equal(addr, IP_ADDRESS);
	assert_null(service);
	assert_int_equal(side, RPMA_INFO_PASSIVE);

	int ret = mock_type(int);
	if (ret) {
		Rpma_provider_error = mock_type(int);
		return ret;
	}

	*info_ptr = mock_type(struct rpma_info *);

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
 * rpma_info_bind_addr -- mock of rpma_info_bind_addr
 */
int
rpma_info_bind_addr(const struct rpma_info *info, struct rdma_cm_id *id)
{
	check_expected(info);
	check_expected(id);

	int ret = mock_type(int);
	if (ret)
		Rpma_provider_error = mock_type(int);
	return ret;
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
 * test_addr_NULL - test NULL addr parameter
 */
static void
test_addr_NULL(void **unused)
{
	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(NULL, &dev);

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
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_addr_NULL_dev_NULL - test NULL addr and dev parameter
 */
static void
test_addr_NULL_dev_NULL(void **unused)
{
	/* run test */
	int ret = rpma_utils_get_ibv_context(NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * test_info_new_failed - rpma_info_new fails
 */
static void
test_info_new_failed(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, ANY_RPMA_ERROR);
	will_return(rpma_info_new, ANY_ERRNO);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return_maybe(rdma_create_id, &id);
	will_return_maybe(rdma_destroy_id, 0);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS, &dev);

	/* verify the results */
	assert_int_equal(ret, ANY_RPMA_ERROR);
	assert_int_equal(rpma_err_get_provider_error(), ANY_ERRNO);
	assert_null(dev);
}

/*
 * test_create_id_failed - memory allocation in rdma_create_id fails
 */
static void
test_create_id_failed(void **unused)
{
	/* configure mocks */
	will_return_maybe(rpma_info_new, 0);
	will_return(rdma_create_id, NULL);
	will_return(rdma_create_id, ANY_ERRNO);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS, &dev);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), ANY_ERRNO);
	assert_null(dev);
}

/*
 * test_bind_addr_failed - rpma_info_bind_addr fails
 */
static void
test_bind_addr_failed(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, 0);
	will_return(rpma_info_new, MOCK_INFO);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return(rdma_create_id, &id);

	expect_value(rpma_info_bind_addr, info, MOCK_INFO);
	expect_value(rpma_info_bind_addr, id, &id);
	will_return(rpma_info_bind_addr, ANY_RPMA_ERROR);
	will_return(rpma_info_bind_addr, ANY_ERRNO);

	will_return(rdma_destroy_id, 0);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS, &dev);

	/* verify the results */
	assert_int_equal(ret, ANY_RPMA_ERROR);
	assert_int_equal(rpma_err_get_provider_error(), ANY_ERRNO);
	assert_null(dev);
}

/*
 * test_success_destroy_id_failed - test if rpma_utils_get_ibv_context()
 *                                  succeeds if rdma_destroy_id() fails
 */
static void
test_success_destroy_id_failed(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, 0);
	will_return(rpma_info_new, MOCK_INFO);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return(rdma_create_id, &id);

	expect_value(rpma_info_bind_addr, info, MOCK_INFO);
	expect_value(rpma_info_bind_addr, id, &id);
	will_return(rpma_info_bind_addr, 0);

	will_return(rdma_destroy_id, ANY_ERRNO);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS, &dev);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_ptr_equal(dev, MOCK_VERBS);
}

/*
 * test_success - test the 'all is OK' situation
 */
static void
test_success(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, 0);
	will_return(rpma_info_new, MOCK_INFO);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return(rdma_create_id, &id);

	expect_value(rpma_info_bind_addr, info, MOCK_INFO);
	expect_value(rpma_info_bind_addr, id, &id);
	will_return(rpma_info_bind_addr, 0);

	will_return(rdma_destroy_id, 0);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS, &dev);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_ptr_equal(dev, MOCK_VERBS);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_addr_NULL),
		cmocka_unit_test(test_dev_NULL),
		cmocka_unit_test(test_addr_NULL_dev_NULL),
		cmocka_unit_test(test_info_new_failed),
		cmocka_unit_test(test_create_id_failed),
		cmocka_unit_test(test_bind_addr_failed),
		cmocka_unit_test(test_success_destroy_id_failed),
		cmocka_unit_test(test_success),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
