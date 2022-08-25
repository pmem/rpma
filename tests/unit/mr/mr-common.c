// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * mr-common.c -- the memory region unit tests's common functions
 */

#include "mocks-ibverbs.h"
#include "mocks-rpma-peer.h"
#include "mr-common.h"
#include "test-common.h"

const char Desc_exp_pmem[] = DESC_EXP_PMEM;
const char Desc_exp_dram[] = DESC_EXP_DRAM;

/* common setups & teardowns */

/*
 * setup__mr_local_and_remote -- create a local and a remote
 * memory region structures
 */
int
setup__mr_local_and_remote(void **mrs_ptr)
{
	static struct mrs mrs = {0};
	int ret;

	struct prestate prestate = {MOCK_USAGE, MOCK_ACCESS, NULL};
	struct prestate *pprestate = &prestate;

	/* create a local memory region structure */
	ret = setup__reg_success((void **)&pprestate);
	mrs.local = prestate.mr;

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);

	/* create a remote memory region structure */
	ret = setup__mr_remote((void **)&mrs.remote);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);

	*mrs_ptr = &mrs;

	return 0;
}

/*
 * teardown__mr_local_and_remote -- delete a local and a remote
 * memory region structures
 */
int
teardown__mr_local_and_remote(void **mrs_ptr)
{
	struct mrs *mrs = (struct mrs *)*mrs_ptr;
	int ret;

	struct prestate prestate = {0};
	struct prestate *pprestate = &prestate;
	prestate.mr = mrs->local;

	/* create a local memory region structure */
	ret = teardown__dereg_success((void **)&pprestate);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);

	/* create a remote memory region structure */
	ret = teardown__mr_remote((void **)&mrs->remote);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);

	return 0;
}


/*
 * setup__reg_success -- create a local memory registration object
 */
int
setup__reg_success(void **pprestate)
{
	struct prestate *prestate = *pprestate;

	/* configure mocks */
	struct rpma_peer_setup_mr_reg_args mr_reg_args;
	mr_reg_args.usage = prestate->usage;
	mr_reg_args.access = prestate->access;
	mr_reg_args.mr = MOCK_MR;
	will_return(rpma_peer_setup_mr_reg, &mr_reg_args);
	will_return(__wrap__test_malloc, MOCK_OK);

	/* run test */
	struct rpma_mr_local *mr = NULL;
	int ret = rpma_mr_reg(MOCK_PEER, MOCK_PTR, MOCK_SIZE,
				mr_reg_args.usage, &mr);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(mr);

	/* pass mr to the test */
	prestate->mr = mr;

	return 0;
}

/*
 * teardown__dereg_success -- delete the local memory registration object
 */
int
teardown__dereg_success(void **pprestate)
{
	struct prestate *prestate = *pprestate;
	struct rpma_mr_local *mr = prestate->mr;

	/* configure mocks */
	will_return(ibv_dereg_mr, MOCK_OK);

	int ret = rpma_mr_dereg(&mr);

	/* verify the result */
	assert_int_equal(ret, MOCK_OK);
	assert_null(mr);

	return 0;
}

/*
 * setup__mr_remote -- create a remote memory region structure based
 * on a pre-prepared memory region's descriptor
 */
int
setup__mr_remote(void **mr_ptr)
{
	/* configure mock */
	will_return(__wrap__test_malloc, MOCK_OK);

	/*
	 * create a remote memory structure based on a pre-prepared descriptor
	 */
	struct rpma_mr_remote *mr = NULL;
	int ret = rpma_mr_remote_from_descriptor(Desc_exp_pmem,
			MR_DESC_SIZE, &mr);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_non_null(mr);

	*mr_ptr = mr;

	return 0;
}

/*
 * teardown__mr_remote -- delete the remote memory region's structure
 */
int
teardown__mr_remote(void **mr_ptr)
{
	struct rpma_mr_remote *mr = *mr_ptr;

	/* delete the remote memory region's structure */
	int ret = rpma_mr_remote_delete(&mr);
	assert_int_equal(ret, MOCK_OK);
	assert_null(mr);

	*mr_ptr = NULL;

	return 0;
}
