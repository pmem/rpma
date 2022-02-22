// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * utils-get_ibv_context.c -- a unit test for rpma_utils_get_ibv_context()
 */

#include <stdio.h>
#include <stdlib.h>
#include <rdma/rdma_cma.h>

#include "cmocka_headers.h"
#include "librpma.h"
#include "mocks-ibverbs.h"
#include "info.h"
#include "test-common.h"

#define TYPE_UNKNOWN (enum rpma_util_ibv_context_type)(-1)

/*
 * sanity__type_unknown - TYPE_UNKNOWN != RPMA_UTIL_IBV_CONTEXT_LOCAL &&
 * TYPE_UNKNOWN != RPMA_UTIL_IBV_CONTEXT_REMOTE
 */
static void
sanity__type_unknown(void **unused)
{
	/* run test */
	assert_int_not_equal(TYPE_UNKNOWN, RPMA_UTIL_IBV_CONTEXT_LOCAL);
	assert_int_not_equal(TYPE_UNKNOWN, RPMA_UTIL_IBV_CONTEXT_REMOTE);
}

/*
 * get_ibvc__addr_NULL - test NULL addr parameter
 */
static void
get_ibvc__addr_NULL(void **unused)
{
	/* run test */
	struct ibv_context *ibv_ctx = NULL;
	int ret = rpma_utils_get_ibv_context(NULL, RPMA_UTIL_IBV_CONTEXT_REMOTE,
			&ibv_ctx);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
	assert_null(ibv_ctx);
}

/*
 * get_ibvc__ibv_ctx_NULL - test NULL ibv_ctx parameter
 */
static void
get_ibvc__ibv_ctx_NULL(void **unused)
{
	/* run test */
	int ret = rpma_utils_get_ibv_context(MOCK_IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_ibvc__type_unknown - type == TYPE_UNKNOWN
 */
static void
get_ibvc__type_unknown(void **unused)
{
	/* run test */
	struct ibv_context *ibv_ctx = NULL;
	int ret = rpma_utils_get_ibv_context(MOCK_IP_ADDRESS,
			TYPE_UNKNOWN, &ibv_ctx);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_ibvc__addr_NULL_ibv_ctx_NULL_type_unknown - test NULL addr and
 * ibv_ctx parameter and type == TYPE_UNKNOWN
 */
static void
get_ibvc__addr_NULL_ibv_ctx_NULL_type_unknown(void **unused)
{
	/* run test */
	int ret = rpma_utils_get_ibv_context(NULL, TYPE_UNKNOWN, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * get_ibvc__info_new_failed_E_PROVIDER - rpma_info_new fails with
 * RPMA_E_PROVIDER
 */
static void
get_ibvc__info_new_failed_E_PROVIDER(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, NULL /* info_ptr */);
	will_return(rpma_info_new, RPMA_E_PROVIDER);
	will_return(rpma_info_new, MOCK_ERRNO);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return_maybe(rdma_create_id, &id);
	will_return_maybe(rdma_destroy_id, 0);

	/* run test */
	struct ibv_context *ibv_ctx = NULL;
	int ret = rpma_utils_get_ibv_context(MOCK_IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &ibv_ctx);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(ibv_ctx);
}

/*
 * get_ibvc__info_new_failed_E_NOMEM - rpma_info_new fails with RPMA_E_NOMEM
 */
static void
get_ibvc__info_new_failed_E_NOMEM(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, NULL /* info_ptr */);
	will_return(rpma_info_new, RPMA_E_NOMEM);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return_maybe(rdma_create_id, &id);
	will_return_maybe(rdma_destroy_id, 0);

	/* run test */
	struct ibv_context *ibv_ctx = NULL;
	int ret = rpma_utils_get_ibv_context(MOCK_IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &ibv_ctx);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOMEM);
	assert_null(ibv_ctx);
}

/*
 * get_ibvc__create_id_failed - rdma_create_id fails with MOCK_ERRNO
 */
static void
get_ibvc__create_id_failed(void **unused)
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
	will_return(rdma_create_id, MOCK_ERRNO);

	/* run test */
	struct ibv_context *ibv_ctx = NULL;
	int ret = rpma_utils_get_ibv_context(MOCK_IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_LOCAL, &ibv_ctx);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(ibv_ctx);
}

/*
 * get_ibvc__bind_addr_failed_E_PROVIDER - rpma_info_bind_addr fails
 * with RPMA_E_PROVIDER
 */
static void
get_ibvc__bind_addr_failed_E_PROVIDER(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return(rdma_create_id, &id);

	/* a local address (passive side) */
	expect_value(rpma_info_bind_addr, info, MOCK_INFO);
	expect_value(rpma_info_bind_addr, id, &id);
	will_return(rpma_info_bind_addr, RPMA_E_PROVIDER);
	will_return(rpma_info_bind_addr, MOCK_ERRNO);

	will_return(rdma_destroy_id, 0);

	/* run test */
	struct ibv_context *ibv_ctx = NULL;
	int ret = rpma_utils_get_ibv_context(MOCK_IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_LOCAL, &ibv_ctx);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(ibv_ctx);
}

/*
 * get_ibvc__resolve_addr_failed_E_PROVIDER - rpma_info_resolve_addr fails
 * with RPMA_E_PROVIDER
 */
static void
get_ibvc__resolve_addr_failed_E_PROVIDER(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return(rdma_create_id, &id);

	/* a remote address (active side) */
	expect_value(rpma_info_resolve_addr, info, MOCK_INFO);
	expect_value(rpma_info_resolve_addr, id, &id);
	will_return(rpma_info_resolve_addr, RPMA_E_PROVIDER);
	will_return(rpma_info_resolve_addr, MOCK_ERRNO);

	will_return(rdma_destroy_id, 0);

	/* run test */
	struct ibv_context *ibv_ctx = NULL;
	int ret = rpma_utils_get_ibv_context(MOCK_IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &ibv_ctx);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(ibv_ctx);
}

/*
 * get_ibvc__success_destroy_id_failed_passive - test if
 * rpma_utils_get_ibv_context() succeeds if rdma_destroy_id() fails
 */
static void
get_ibvc__success_destroy_id_failed_passive(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return(rdma_create_id, &id);

	/* a local address (passive side) */
	expect_value(rpma_info_bind_addr, info, MOCK_INFO);
	expect_value(rpma_info_bind_addr, id, &id);
	will_return(rpma_info_bind_addr, 0);

	will_return(rdma_destroy_id, MOCK_ERRNO);

	/* run test */
	struct ibv_context *ibv_ctx = NULL;
	int ret = rpma_utils_get_ibv_context(MOCK_IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_LOCAL, &ibv_ctx);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_ptr_equal(ibv_ctx, MOCK_VERBS);
}

/*
 * get_ibvc__success_destroy_id_failed_active - test if
 * rpma_utils_get_ibv_context() succeeds if rdma_destroy_id() fails
 */
static void
get_ibvc__success_destroy_id_failed_active(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return(rdma_create_id, &id);

	/* a remote address (active side) */
	expect_value(rpma_info_resolve_addr, info, MOCK_INFO);
	expect_value(rpma_info_resolve_addr, id, &id);
	will_return(rpma_info_resolve_addr, 0);

	will_return(rdma_destroy_id, MOCK_ERRNO);

	/* run test */
	struct ibv_context *ibv_ctx = NULL;
	int ret = rpma_utils_get_ibv_context(MOCK_IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &ibv_ctx);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_ptr_equal(ibv_ctx, MOCK_VERBS);
}

/*
 * get_ibvc__success_passive - test the 'all is OK' situation
 */
static void
get_ibvc__success_passive(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return(rdma_create_id, &id);

	expect_value(rpma_info_bind_addr, info, MOCK_INFO);
	expect_value(rpma_info_bind_addr, id, &id);
	will_return(rpma_info_bind_addr, 0);

	will_return(rdma_destroy_id, 0);

	/* run test */
	struct ibv_context *ibv_ctx = NULL;
	int ret = rpma_utils_get_ibv_context(MOCK_IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_LOCAL, &ibv_ctx);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_ptr_equal(ibv_ctx, MOCK_VERBS);
}

/*
 * get_ibvc__success_active - test the 'all is OK' situation
 */
static void
get_ibvc__success_active(void **unused)
{
	/* configure mocks */
	will_return(rpma_info_new, MOCK_INFO);

	struct rdma_cm_id id;
	id.verbs = MOCK_VERBS;
	will_return(rdma_create_id, &id);

	expect_value(rpma_info_resolve_addr, info, MOCK_INFO);
	expect_value(rpma_info_resolve_addr, id, &id);
	will_return(rpma_info_resolve_addr, 0);

	will_return(rdma_destroy_id, 0);

	/* run test */
	struct ibv_context *ibv_ctx = NULL;
	int ret = rpma_utils_get_ibv_context(MOCK_IP_ADDRESS,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &ibv_ctx);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_ptr_equal(ibv_ctx, MOCK_VERBS);
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* sanity */
		cmocka_unit_test(sanity__type_unknown),

		cmocka_unit_test(get_ibvc__addr_NULL),
		cmocka_unit_test(get_ibvc__ibv_ctx_NULL),
		cmocka_unit_test(get_ibvc__type_unknown),
		cmocka_unit_test(get_ibvc__addr_NULL_ibv_ctx_NULL_type_unknown),
		cmocka_unit_test(get_ibvc__info_new_failed_E_PROVIDER),
		cmocka_unit_test(get_ibvc__info_new_failed_E_NOMEM),
		cmocka_unit_test(get_ibvc__create_id_failed),
		cmocka_unit_test(get_ibvc__bind_addr_failed_E_PROVIDER),
		cmocka_unit_test(get_ibvc__resolve_addr_failed_E_PROVIDER),
		cmocka_unit_test(get_ibvc__success_destroy_id_failed_passive),
		cmocka_unit_test(get_ibvc__success_destroy_id_failed_active),
		cmocka_unit_test(get_ibvc__success_passive),
		cmocka_unit_test(get_ibvc__success_active),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
