// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * mr-advise.c -- rpma_mr_advise() unit tests
 */

#include <infiniband/verbs.h>
#include <stdlib.h>

#include "cmocka_headers.h"
#include "mr.h"
#include "librpma.h"

#include "mr-common.h"
#include "mocks-ibverbs.h"
#include "test-common.h"

/*
 * advise__failed_E_PROVIDER - rpma_mr_advise failed
 * with RPMA_E_PROVIDER when length > ibv_mr->length
 */
static void
advise__failed_E_PROVIDER(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;

	/* configure mocks */
	struct ibv_advise_mr_mock_args args;
	args.pd = MOCK_IBV_PD;
	args.advice = IB_UVERBS_ADVISE_MR_ADVICE_PREFETCH_WRITE;
	args.flags = IB_UVERBS_ADVISE_MR_FLAG_FLUSH;
	args.local_addr = MOCK_LADDR + MOCK_SRC_OFFSET;
	args.length = MOCK_LEN;
	args.lkey = MOCK_LKEY;
	args.num_sge = 1;
	args.ret = RPMA_E_PROVIDER;
	will_return(ibv_advise_mr_mock, &args);

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
	struct ibv_advise_mr_mock_args args;
	args.pd = MOCK_IBV_PD;
	args.advice = IB_UVERBS_ADVISE_MR_ADVICE_PREFETCH_WRITE;
	args.flags = IB_UVERBS_ADVISE_MR_FLAG_FLUSH;
	args.local_addr = MOCK_LADDR + MOCK_SRC_OFFSET;
	args.length = MOCK_LEN;
	args.lkey = MOCK_LKEY;
	args.num_sge = 1;
	args.ret = MOCK_OK;
	will_return(ibv_advise_mr_mock, &args);

	/* run test */
	int ret = rpma_mr_advise(mrs->local, MOCK_SRC_OFFSET, MOCK_LEN,
				IB_UVERBS_ADVISE_MR_ADVICE_PREFETCH_WRITE,
				IB_UVERBS_ADVISE_MR_FLAG_FLUSH);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
}

/*
 * group_setup_mr_advise -- prepare resources for all tests in the group
 */
static int
group_setup_mr_advise(void **unused)
{
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

	return 0;
}

static const struct CMUnitTest test_mr_advise[] = {
	/* rpma_mr_adivse() unit tests */
	cmocka_unit_test_setup_teardown(advise__failed_E_PROVIDER,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test_setup_teardown(advise__success,
			setup__mr_local_and_remote,
			teardown__mr_local_and_remote),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(test_mr_advise,
		group_setup_mr_advise, NULL);
}
