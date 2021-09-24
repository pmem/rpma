// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * mr-advise.c -- rpma_mr_advise() unit tests
 */

#include <infiniband/verbs.h>
#include <stdlib.h>

#include "cmocka_headers.h"
#include "librpma.h"

#include "mr-common.h"
#include "mocks-ibverbs.h"
#include "test-common.h"

#ifdef IBV_ADVISE_MR_SUPPORTED
/*
 * advise__failed_E_NOSUPP - rpma_mr_advise failed
 * with RPMA_E_NOSUPP
 */
static void
advise__failed_E_NOSUPP(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	int error_no[] = {
		EOPNOTSUPP,
		ENOTSUP
	};

	int n_values = sizeof(error_no) / sizeof(error_no[0]);

	for (int i = 0; i < n_values; i++) {
		/* configure mocks */
		expect_value(ibv_advise_mr_mock, pd, MOCK_IBV_PD);
		expect_value(ibv_advise_mr_mock, advice,
			IB_UVERBS_ADVISE_MR_ADVICE_PREFETCH_WRITE);
		expect_value(ibv_advise_mr_mock, flags,
			IB_UVERBS_ADVISE_MR_FLAG_FLUSH);
		expect_value(ibv_advise_mr_mock, sg_list->lkey, MOCK_LKEY);
		expect_value(ibv_advise_mr_mock, sg_list->addr,
			MOCK_LADDR + MOCK_SRC_OFFSET);
		expect_value(ibv_advise_mr_mock, sg_list->length, MOCK_LEN);
		expect_value(ibv_advise_mr_mock, num_sge, 1);
		will_return(ibv_advise_mr_mock, error_no[i]);

		/* run test */
		int ret = rpma_mr_advise(mrs->local, MOCK_SRC_OFFSET, MOCK_LEN,
				IB_UVERBS_ADVISE_MR_ADVICE_PREFETCH_WRITE,
				IB_UVERBS_ADVISE_MR_FLAG_FLUSH);

		/* verify the results */
		assert_int_equal(ret, RPMA_E_NOSUPP);
	}
}

/*
 * advise__failed_E_INVAL - rpma_mr_advise failed
 * with RPMA_E_INVAL
 */
static void
advise__failed_E_INVAL(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	int error_no[] = {
		EFAULT,
		EINVAL
	};

	int n_values = sizeof(error_no) / sizeof(error_no[0]);

	for (int i = 0; i < n_values; i++) {
		/* configure mocks */
		expect_value(ibv_advise_mr_mock, pd, MOCK_IBV_PD);
		expect_value(ibv_advise_mr_mock, advice,
			IB_UVERBS_ADVISE_MR_ADVICE_PREFETCH_WRITE);
		expect_value(ibv_advise_mr_mock, flags,
			IB_UVERBS_ADVISE_MR_FLAG_FLUSH);
		expect_value(ibv_advise_mr_mock, sg_list->lkey, MOCK_LKEY);
		expect_value(ibv_advise_mr_mock, sg_list->addr,
			MOCK_LADDR + MOCK_SRC_OFFSET);
		expect_value(ibv_advise_mr_mock, sg_list->length, MOCK_LEN);
		expect_value(ibv_advise_mr_mock, num_sge, 1);
		will_return(ibv_advise_mr_mock, error_no[i]);

		/* run test */
		int ret = rpma_mr_advise(mrs->local, MOCK_SRC_OFFSET, MOCK_LEN,
				IB_UVERBS_ADVISE_MR_ADVICE_PREFETCH_WRITE,
				IB_UVERBS_ADVISE_MR_FLAG_FLUSH);

		/* verify the results */
		assert_int_equal(ret, RPMA_E_INVAL);
	}
}

/*
 * advise__failed_E_PROVIDER - rpma_mr_advise failed
 * with RPMA_E_PROVIDER
 */
static void
advise__failed_E_PROVIDER(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	expect_value(ibv_advise_mr_mock, pd, MOCK_IBV_PD);
	expect_value(ibv_advise_mr_mock, advice,
		IB_UVERBS_ADVISE_MR_ADVICE_PREFETCH_WRITE);
	expect_value(ibv_advise_mr_mock, flags, IB_UVERBS_ADVISE_MR_FLAG_FLUSH);
	expect_value(ibv_advise_mr_mock, sg_list->lkey, MOCK_LKEY);
	expect_value(ibv_advise_mr_mock, sg_list->addr,
		MOCK_LADDR + MOCK_SRC_OFFSET);
	expect_value(ibv_advise_mr_mock, sg_list->length, MOCK_LEN);
	expect_value(ibv_advise_mr_mock, num_sge, 1);
	will_return(ibv_advise_mr_mock, RPMA_E_PROVIDER);

	/* run test */
	int ret = rpma_mr_advise(mrs->local, MOCK_SRC_OFFSET, MOCK_LEN,
				IB_UVERBS_ADVISE_MR_ADVICE_PREFETCH_WRITE,
				IB_UVERBS_ADVISE_MR_FLAG_FLUSH);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
}

/*
 * read__success - happy day scenario
 */
static void
advise__success(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	expect_value(ibv_advise_mr_mock, pd, MOCK_IBV_PD);
	expect_value(ibv_advise_mr_mock, advice,
		IB_UVERBS_ADVISE_MR_ADVICE_PREFETCH_WRITE);
	expect_value(ibv_advise_mr_mock, flags, IB_UVERBS_ADVISE_MR_FLAG_FLUSH);
	expect_value(ibv_advise_mr_mock, sg_list->lkey, MOCK_LKEY);
	expect_value(ibv_advise_mr_mock, sg_list->addr,
		MOCK_LADDR + MOCK_SRC_OFFSET);
	expect_value(ibv_advise_mr_mock, sg_list->length, MOCK_LEN);
	expect_value(ibv_advise_mr_mock, num_sge, 1);
	will_return(ibv_advise_mr_mock, MOCK_OK);

	/* run test */
	int ret = rpma_mr_advise(mrs->local, MOCK_SRC_OFFSET, MOCK_LEN,
				IB_UVERBS_ADVISE_MR_ADVICE_PREFETCH_WRITE,
				IB_UVERBS_ADVISE_MR_FLAG_FLUSH);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

#else
/*
 * advise__failed_E_NOSUPP - rpma_mr_advise failed
 * with RPMA_E_NOSUPP when the operation is not supported by the system
 */
static void
advise__failed_E_NOSUPP(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* run test */
	int ret = rpma_mr_advise(mrs->local, MOCK_SRC_OFFSET, MOCK_LEN,
				MOCK_ADVICE, MOCK_MR_FLAG);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_NOSUPP);
}
#endif

/*
 * group_setup_mr_advise -- prepare resources for all tests in the group
 */
static int
group_setup_mr_advise(void **unused)
{
#ifdef IBV_ADVISE_MR_SUPPORTED
	/* configure global mocks */

	/*
	 * ibv_advise_mr() is defined as a static inline function
	 * in the included header <infiniband/verbs.h>,
	 * so we cannot define it again. It is defined as:
	 * {
	 *		return (struct verbs_context *)(((uint8_t *)ctx)
	 *		- offsetof(struct verbs_context, context))
	 *		->advise_mr(pd, advice, flags, sg_list, num_sge);
	 * }
	 * so we can set the advise_mr function pointer to our mock function.
	 */
	Verbs_context.advise_mr = ibv_advise_mr_mock;
	Verbs_context.sz = sizeof(Verbs_context);

	Ibv_pd.context = (struct ibv_context *)((uint8_t *)&Verbs_context +
		offsetof(struct verbs_context, context));
	Ibv_pd.context->abi_compat = __VERBS_ABI_IS_EXTENDED;

	Ibv_mr.lkey = MOCK_LKEY;
	Ibv_mr.pd = MOCK_IBV_PD;
#endif

	return 0;
}

#ifdef IBV_ADVISE_MR_SUPPORTED
static const struct CMUnitTest test_mr_advise[] = {
	/* rpma_mr_adivse() unit tests */
	cmocka_unit_test_setup_teardown(advise__failed_E_NOSUPP,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(advise__failed_E_INVAL,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(advise__failed_E_PROVIDER,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(advise__success,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test(NULL)
};

#else
static const struct CMUnitTest test_mr_advise[] = {
	/* rpma_mr_adivse() unit tests */
	cmocka_unit_test_setup_teardown(advise__failed_E_NOSUPP,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test(NULL)
};
#endif

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_mr_advise,
		group_setup_mr_advise, NULL);
}
