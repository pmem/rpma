// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * utils-ibv_context_is_odp_capable.c -- a unit test for
 * rpma_utils_ibv_context_is_odp_capable()
 */

#include "cmocka_headers.h"
#include "mocks-ibverbs.h"
#include "librpma.h"
#include "test-common.h"

/*
 * ibvc_odp__ibv_ctx_NULL -- ibv_ctx NULL is invalid
 */
static void
ibvc_odp__ibv_ctx_NULL(void **unused)
{
	/* run test */
	int is_odp_capable;
	int ret = rpma_utils_ibv_context_is_odp_capable(NULL, &is_odp_capable);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * ibvc_odp__cap_NULL -- is_odp_capable NULL is invalid
 */
static void
ibvc_odp__cap_NULL(void **unused)
{
	/* run test */
	int ret = rpma_utils_ibv_context_is_odp_capable(MOCK_VERBS, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * ibvc_odp__ibv_ctx_cap_NULL -- ibv_ctx and is_odp_capable NULL are invalid
 */
static void
ibvc_odp__ibv_ctx_cap_NULL(void **unused)
{
	/* run test */
	int ret = rpma_utils_ibv_context_is_odp_capable(NULL, NULL);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_INVAL);
}

/*
 * ibvc_odp__query_fail -- ibv_query_device_ex() failed
 */
static void
ibvc_odp__query_fail(void **unused)
{
	/* configure mocks */
	will_return(ibv_query_device_ex_mock, NULL);
	will_return(ibv_query_device_ex_mock, MOCK_ERRNO);

	/* run test */
	int is_odp_capable;
	int ret = rpma_utils_ibv_context_is_odp_capable(MOCK_VERBS,
			&is_odp_capable);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * ibvc_odp__general_caps_no -- ibv_odp_caps.general_caps IBV_ODP_SUPPORT bit
 * is not set
 */
static void
ibvc_odp__general_caps_no(void **unused)
{
	/* configure mocks */
	struct ibv_odp_caps caps = {
		.general_caps = 0, /* IBV_ODP_SUPPORT not set */
		.per_transport_caps = {
				IBV_ODP_SUPPORT_WRITE | IBV_ODP_SUPPORT_READ,
				0,
				0
		},
	};
	will_return(ibv_query_device_ex_mock, &caps);

	/* run test */
	int is_odp_capable;
	int ret = rpma_utils_ibv_context_is_odp_capable(MOCK_VERBS,
			&is_odp_capable);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_int_equal(is_odp_capable, 0);
}

/*
 * ibvc_odp__rc_caps_not_all -- ibv_odp_caps.per_transport_caps.rc_odp_caps
 * not all required bits are set
 */
static void
ibvc_odp__rc_caps_not_all(void **unused)
{
	/* configure mocks */
	struct ibv_odp_caps caps = {
		.general_caps = IBV_ODP_SUPPORT,
		.per_transport_caps = {
				/* IBV_ODP_SUPPORT_READ not set */
				IBV_ODP_SUPPORT_WRITE,
				0,
				0
		},
	};
	will_return(ibv_query_device_ex_mock, &caps);

	/* run test */
	int is_odp_capable;
	int ret = rpma_utils_ibv_context_is_odp_capable(MOCK_VERBS,
			&is_odp_capable);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_int_equal(is_odp_capable, 0);
}

/*
 * ibvc_odp__odp_capable -- all required bits are set
 */
static void
ibvc_odp__odp_capable(void **unused)
{
	/* configure mocks */
	struct ibv_odp_caps caps = {
		.general_caps = IBV_ODP_SUPPORT,
		.per_transport_caps = {
				IBV_ODP_SUPPORT_WRITE | IBV_ODP_SUPPORT_READ,
				0,
				0
		},
	};
	will_return(ibv_query_device_ex_mock, &caps);

	/* run test */
	int is_odp_capable;
	int ret = rpma_utils_ibv_context_is_odp_capable(MOCK_VERBS,
			&is_odp_capable);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_int_equal(is_odp_capable, 1);
}

int
main(int argc, char *argv[])
{
	MOCK_VERBS->abi_compat = __VERBS_ABI_IS_EXTENDED;
	Verbs_context.query_device_ex = ibv_query_device_ex_mock;
	Verbs_context.sz = sizeof(struct verbs_context);

	const struct CMUnitTest tests[] = {
		/* rpma_utils_ibv_context_is_odp_capable() unit tests */
		cmocka_unit_test(ibvc_odp__ibv_ctx_NULL),
		cmocka_unit_test(ibvc_odp__cap_NULL),
		cmocka_unit_test(ibvc_odp__ibv_ctx_cap_NULL),
		cmocka_unit_test(ibvc_odp__query_fail),
		cmocka_unit_test(ibvc_odp__general_caps_no),
		cmocka_unit_test(ibvc_odp__rc_caps_not_all),
		cmocka_unit_test(ibvc_odp__odp_capable),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
