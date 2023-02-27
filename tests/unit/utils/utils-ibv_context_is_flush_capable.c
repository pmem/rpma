// SPDX-License-Identifier: BSD-3-Clause
/* Copyright (c) 2023 Fujitsu Limited */

/*
 * utils-ibv_context_is_flush_capable.c -- a unit test
 * for rpma_utils_ibv_context_is_flush_capable()
 */

#include "cmocka_headers.h"
#include "mocks-ibverbs.h"
#include "librpma.h"
#include "test-common.h"
#include "utils.h"

/*
 * ibvc_flush__cap_no_without_both_attributes -- both of flush attributes are not set
 * in attr.device_cap_flags_ex
 */
static void
ibvc_flush__cap_no_without_both_attributes(void **unused)
{
#ifdef NATIVE_FLUSH_SUPPORTED
	/* configure mocks */
	struct ibv_device_attr_ex attr = {
		/* both of flush attributes are not set */
		.device_cap_flags_ex = 0,
	};
	will_return(ibv_query_device_ex_mock, &attr);
#endif

	/* run test */
	int is_flush_capable;
	int ret = rpma_utils_ibv_context_is_flush_capable(MOCK_VERBS, &is_flush_capable);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_int_equal(is_flush_capable, 0);
}

#ifdef NATIVE_FLUSH_SUPPORTED
/*
 * ibvc_flush__query_fail -- ibv_query_device_ex() failed
 */
static void
ibvc_flush__query_fail(void **unused)
{
	/* configure mocks */
	will_return(ibv_query_device_ex_mock, NULL);
	will_return(ibv_query_device_ex_mock, MOCK_ERRNO);

	/* run test */
	int is_flush_capable;
	int ret = rpma_utils_ibv_context_is_flush_capable(MOCK_VERBS, &is_flush_capable);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * ibvc_flush__cap_no_with_one_attribute -- only one flush attribute is set
 * in attr.device_cap_flags_ex
 */
static void
ibvc_flush__cap_no_with_one_attribute(void **unused)
{
	/* configure mocks */
	struct ibv_device_attr_ex attr;
	uint64_t flush_attrs[2] = {IB_UVERBS_DEVICE_FLUSH_GLOBAL,
			IB_UVERBS_DEVICE_FLUSH_PERSISTENT};
	int num = sizeof(flush_attrs) / sizeof(flush_attrs[0]);

	for (int i = 0; i < num; i++) {
		/* only one flush attribute is set */
		attr.device_cap_flags_ex = flush_attrs[i];

		will_return(ibv_query_device_ex_mock, &attr);

		/* run test */
		int is_flush_capable;
		int ret = rpma_utils_ibv_context_is_flush_capable(MOCK_VERBS, &is_flush_capable);

		/* verify the results */
		assert_int_equal(ret, 0);
		assert_int_equal(is_flush_capable, 0);
	}
}

/*
 * ibvc_flush__cap_yes_with_both_attributes -- both of flush attributes are set
 * in attr.device_cap_flags_ex
 */
static void
ibvc_flush__cap_yes_with_both_attributes(void **unused)
{
	/* configure mocks */
	struct ibv_device_attr_ex attr = {
		/* both of flush attributes are set */
		.device_cap_flags_ex = IB_UVERBS_DEVICE_FLUSH_GLOBAL |
				IB_UVERBS_DEVICE_FLUSH_PERSISTENT,
	};
	will_return(ibv_query_device_ex_mock, &attr);

	/* run test */
	int is_flush_capable;
	int ret = rpma_utils_ibv_context_is_flush_capable(MOCK_VERBS, &is_flush_capable);

	/* verify the results */
	assert_int_equal(ret, 0);
	assert_int_equal(is_flush_capable, 1);
}
#endif

int
main(int argc, char *argv[])
{
	MOCK_VERBS->abi_compat = __VERBS_ABI_IS_EXTENDED;
	Verbs_context.query_device_ex = ibv_query_device_ex_mock;
	Verbs_context.sz = sizeof(struct verbs_context);

	const struct CMUnitTest tests[] = {
		/* rpma_utils_ibv_context_is_flush_capable() unit tests */
		cmocka_unit_test(ibvc_flush__cap_no_without_both_attributes),
#ifdef NATIVE_FLUSH_SUPPORTED
		cmocka_unit_test(ibvc_flush__query_fail),
		cmocka_unit_test(ibvc_flush__cap_no_with_one_attribute),
		cmocka_unit_test(ibvc_flush__cap_yes_with_both_attributes),
#endif
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
