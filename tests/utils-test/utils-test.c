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

#define IP_ADDRESS	"127.0.0.1"
#define MOCK_VERBS	((struct ibv_context *)0xABC0) /* any not-NULL value */
#define MOCK_SRC_ADDR	((struct sockaddr *)0xABC1) /* any not-NULL value */
#define MOCK_DST_ADDR	((struct sockaddr *)0xABC2) /* any not-NULL value */

/* mock errno values */
#define MOCK_ERRNO_0	0xFE00
#define MOCK_ERRNO_1	0xFE01
#define MOCK_ERRNO_2	0xFE02
#define MOCK_ERRNO_3	0xFE03
#define MOCK_ERRNO_4	0xFE04

/*
 * rdma_getaddrinfo -- mock of rdma_getaddrinfo
 */
int
rdma_getaddrinfo(const char *node, const char *service,
			const struct rdma_addrinfo *hints,
			struct rdma_addrinfo **res)
{
	assert_non_null(res);

	/* Either node, service, or hints must be provided. */
	if (!(node || service || hints)) {
		errno = MOCK_ERRNO_0;
		return -1;
	}

	/*
	 * If node is not given, rdma_getaddrinfo will attempt to resolve
	 * the RDMA addressing information based on the hints.ai_src_addr,
	 * hints.ai_dst_addr, or hints.ai_route.
	 */
	if ((!node) && hints) {
		if (!(hints->ai_src_addr || hints->ai_dst_addr ||
		    hints->ai_route)) {
			errno = MOCK_ERRNO_1;
			return -1;
		}
	}

	/* allocate (struct rdma_addrinfo *) */
	*res = mock_type(struct rdma_addrinfo *);
	if (*res == NULL) {
		errno = mock_type(int);
		return -1;
	}

	if (hints) {
		check_expected(hints->ai_flags);
		(*res)->ai_flags = hints->ai_flags;
	}

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
	check_expected(ps);

	/* allocate (struct rdma_cm_id *) */
	*id = mock_type(struct rdma_cm_id *);
	if (*id == NULL) {
		errno = mock_type(int);
		return -1;
	}

	return 0;
}

/*
 * rdma_bind_addr -- mock of rdma_bind_addr
 */
int
rdma_bind_addr(struct rdma_cm_id *id, struct sockaddr *addr)
{
	check_expected(id);
	check_expected(addr);

	int ret = mock_type(int);
	if (ret)
		errno = mock_type(int);
	return ret;
}

int
rdma_resolve_addr(struct rdma_cm_id *id, struct sockaddr *src_addr,
			struct sockaddr *dst_addr, int timeout_ms)
{
	check_expected(id);
	check_expected(src_addr);
	check_expected(dst_addr);

	int ret = mock_type(int);
	if (ret)
		errno = mock_type(int);
	return ret;
}

/*
 * rdma_destroy_id -- mock of rdma_destroy_id
 */
int
rdma_destroy_id(struct rdma_cm_id *id)
{
	check_expected(id);

	int ret = mock_type(int);
	if (ret)
		errno = mock_type(int);
	return ret;
}

/*
 * rdma_freeaddrinfo -- mock of rdma_freeaddrinfo
 */
void
rdma_freeaddrinfo(struct rdma_addrinfo *res)
{
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
 * test_addr_NULL - test NULL addr and dev parameter
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
 * test_getaddrinfo_failed - memory allocation in rdma_getaddrinfo fails
 */
static void
test_getaddrinfo_failed(void **unused)
{
	/* configure mocks */
	will_return(rdma_getaddrinfo, NULL);
	will_return(rdma_getaddrinfo, ENOMEM);
	will_return(rdma_getaddrinfo, NULL);
	will_return(rdma_getaddrinfo, ENOMEM);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS, &dev);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), ENOMEM);
	assert_null(dev);
}

/*
 * test_create_id_failed_passive - memory allocation in rdma_create_id fails
 */
static void
test_create_id_failed_passive(void **unused)
{
	/* configure mocks */
	struct rdma_addrinfo rai;
	will_return(rdma_getaddrinfo, &rai);
	expect_value(rdma_getaddrinfo, hints->ai_flags, RAI_PASSIVE);
	expect_value(rdma_create_id, ps, RDMA_PS_TCP);
	will_return(rdma_create_id, NULL);
	will_return(rdma_create_id, ENOMEM);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS, &dev);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), ENOMEM);
	assert_null(dev);
}

/*
 * test_create_id_failed_active - rdma_getaddrinfo fails for passive
 *                           and succeeds for active
 */
static void
test_create_id_failed_active(void **unused)
{
	/* configure mocks */
	struct rdma_addrinfo rai;
	will_return(rdma_getaddrinfo, NULL);
	will_return(rdma_getaddrinfo, MOCK_ERRNO_4);
	will_return(rdma_getaddrinfo, &rai);
	expect_value(rdma_getaddrinfo, hints->ai_flags, 0); /* active */
	expect_value(rdma_create_id, ps, RDMA_PS_TCP);
	will_return(rdma_create_id, NULL);
	will_return(rdma_create_id, ENOMEM);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS, &dev);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), ENOMEM);
	assert_null(dev);
}

/*
 * test_bind_addr_failed - rdma_bind_addr fails
 */
static void
test_bind_addr_failed(void **unused)
{
	/* configure mocks */
	struct rdma_addrinfo rai;
	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	rai.ai_src_addr = MOCK_SRC_ADDR;
	will_return(rdma_getaddrinfo, &rai);
	expect_value(rdma_getaddrinfo, hints->ai_flags, RAI_PASSIVE);
	expect_value(rdma_create_id, ps, RDMA_PS_TCP);
	will_return(rdma_create_id, &id);
	expect_value(rdma_bind_addr, id, &id);
	expect_value(rdma_bind_addr, addr, MOCK_SRC_ADDR);
	will_return(rdma_bind_addr, -1);
	will_return(rdma_bind_addr, MOCK_ERRNO_2);
	expect_value(rdma_destroy_id, id, &id);
	will_return(rdma_destroy_id, 0);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS, &dev);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), MOCK_ERRNO_2);
	assert_null(dev);
}

/*
 * test_resolve_addr_failed - rdma_resolve_addr fails
 */
static void
test_resolve_addr_failed(void **unused)
{
	/* configure mocks */
	struct rdma_addrinfo rai;
	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	rai.ai_src_addr = MOCK_SRC_ADDR;
	rai.ai_dst_addr = MOCK_DST_ADDR;
	will_return(rdma_getaddrinfo, NULL);
	will_return(rdma_getaddrinfo, MOCK_ERRNO_4);
	will_return(rdma_getaddrinfo, &rai);
	expect_value(rdma_getaddrinfo, hints->ai_flags, 0); /* active */
	expect_value(rdma_create_id, ps, RDMA_PS_TCP);
	will_return(rdma_create_id, &id);
	expect_value(rdma_resolve_addr, id, &id);
	expect_value(rdma_resolve_addr, src_addr, MOCK_SRC_ADDR);
	expect_value(rdma_resolve_addr, dst_addr, MOCK_DST_ADDR);
	will_return(rdma_resolve_addr, -1);
	will_return(rdma_resolve_addr, MOCK_ERRNO_2);
	expect_value(rdma_destroy_id, id, &id);
	will_return(rdma_destroy_id, 0);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS, &dev);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_int_equal(rpma_err_get_provider_error(), MOCK_ERRNO_2);
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
	struct rdma_addrinfo rai;
	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	rai.ai_src_addr = MOCK_SRC_ADDR;
	will_return(rdma_getaddrinfo, &rai);
	expect_value(rdma_getaddrinfo, hints->ai_flags, RAI_PASSIVE);
	expect_value(rdma_create_id, ps, RDMA_PS_TCP);
	will_return(rdma_create_id, &id);
	expect_value(rdma_bind_addr, id, &id);
	expect_value(rdma_bind_addr, addr, MOCK_SRC_ADDR);
	will_return(rdma_bind_addr, 0);
	expect_value(rdma_destroy_id, id, &id);
	will_return(rdma_destroy_id, -1);
	will_return(rdma_destroy_id, MOCK_ERRNO_3);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS, &dev);

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
	struct rdma_addrinfo rai;
	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	rai.ai_src_addr = MOCK_SRC_ADDR;
	will_return(rdma_getaddrinfo, &rai);
	expect_value(rdma_getaddrinfo, hints->ai_flags, RAI_PASSIVE);
	expect_value(rdma_create_id, ps, RDMA_PS_TCP);
	will_return(rdma_create_id, &id);
	expect_value(rdma_bind_addr, id, &id);
	expect_value(rdma_bind_addr, addr, MOCK_SRC_ADDR);
	will_return(rdma_bind_addr, 0);
	expect_value(rdma_destroy_id, id, &id);
	will_return(rdma_destroy_id, 0);

	/* run test */
	struct ibv_context *dev = NULL;
	int ret = rpma_utils_get_ibv_context(IP_ADDRESS, &dev);

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
	struct rdma_addrinfo rai;
	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	rai.ai_src_addr = MOCK_SRC_ADDR;
	rai.ai_dst_addr = MOCK_DST_ADDR;
	will_return(rdma_getaddrinfo, NULL);
	will_return(rdma_getaddrinfo, MOCK_ERRNO_4);
	will_return(rdma_getaddrinfo, &rai);
	expect_value(rdma_getaddrinfo, hints->ai_flags, 0); /* active */
	expect_value(rdma_create_id, ps, RDMA_PS_TCP);
	will_return(rdma_create_id, &id);
	expect_value(rdma_resolve_addr, id, &id);
	expect_value(rdma_resolve_addr, src_addr, MOCK_SRC_ADDR);
	expect_value(rdma_resolve_addr, dst_addr, MOCK_DST_ADDR);
	will_return(rdma_resolve_addr, 0);
	expect_value(rdma_destroy_id, id, &id);
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
		cmocka_unit_test(test_getaddrinfo_failed),
		cmocka_unit_test(test_create_id_failed_passive),
		cmocka_unit_test(test_create_id_failed_active),
		cmocka_unit_test(test_bind_addr_failed),
		cmocka_unit_test(test_resolve_addr_failed),
		cmocka_unit_test(test_success_destroy_id_failed),
		cmocka_unit_test(test_success_passive),
		cmocka_unit_test(test_success_active),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
