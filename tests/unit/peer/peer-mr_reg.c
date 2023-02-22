// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */
/* Copyright (c) 2021-2023, Fujitsu Limited */

/*
 * peer-mr_reg.c -- a peer unit test
 *
 * API covered:
 * - rpma_peer_setup_mr_reg()
 */

#include <infiniband/verbs.h>

#include "cmocka_headers.h"
#include "conn_req.h"
#include "mocks-ibverbs.h"
#include "mocks-rpma-utils.h"
#include "peer.h"
#include "peer-common.h"
#include "test-common.h"

static struct prestate prestates[] = {
	/* 0-1) non-iWARP and iWARP are the same */
	{IBV_TRANSPORT_IB,
		RPMA_MR_USAGE_READ_SRC,
			IBV_ACCESS_REMOTE_READ,
				MOCK_ODP_CAPABLE},
	{IBV_TRANSPORT_IWARP,
		RPMA_MR_USAGE_READ_SRC,
			IBV_ACCESS_REMOTE_READ,
				MOCK_ODP_CAPABLE},
	/* 2-3) non-iWARP and iWARP differs */
	{IBV_TRANSPORT_IB,
		RPMA_MR_USAGE_READ_DST,
			IBV_ACCESS_LOCAL_WRITE,
				MOCK_ODP_CAPABLE},
	{IBV_TRANSPORT_IWARP,
		RPMA_MR_USAGE_READ_DST,
			IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE,
				MOCK_ODP_CAPABLE},
	/* 4-5) non-iWARP and iWARP are the same */
	{IBV_TRANSPORT_IB,
		RPMA_MR_USAGE_WRITE_SRC,
			IBV_ACCESS_LOCAL_WRITE,
				MOCK_ODP_CAPABLE},
	{IBV_TRANSPORT_IWARP,
		RPMA_MR_USAGE_WRITE_SRC,
			IBV_ACCESS_LOCAL_WRITE,
				MOCK_ODP_CAPABLE},
	/* 6-7) non-iWARP and iWARP are the same */
	{IBV_TRANSPORT_IB,
		RPMA_MR_USAGE_WRITE_DST,
			IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_LOCAL_WRITE,
				MOCK_ODP_CAPABLE},
	{IBV_TRANSPORT_IWARP,
		RPMA_MR_USAGE_WRITE_DST,
			IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_LOCAL_WRITE,
				MOCK_ODP_CAPABLE},
	/* 8-9) non-iWARP and iWARP are the same */
	{IBV_TRANSPORT_IB,
		RPMA_MR_USAGE_RECV,
			IBV_ACCESS_LOCAL_WRITE,
				MOCK_ODP_CAPABLE},
	{IBV_TRANSPORT_IWARP,
		RPMA_MR_USAGE_RECV,
			IBV_ACCESS_LOCAL_WRITE,
				MOCK_ODP_CAPABLE},
	/* 10-11) non-iWARP and iWARP are the same */
	{IBV_TRANSPORT_IB,
		(RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY | RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT),
		IBV_ACCESS_REMOTE_READ,
			MOCK_ODP_CAPABLE, 0, MOCK_FLUSH_INCAPABLE},
	{IBV_TRANSPORT_IWARP,
		(RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY | RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT),
			IBV_ACCESS_REMOTE_READ,
				MOCK_ODP_CAPABLE, 0, MOCK_FLUSH_INCAPABLE},
#ifdef NATIVE_FLUSH_SUPPORTED
	/* 12-13) non-iWARP and iWARP are the same */
	{IBV_TRANSPORT_IB,
		RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY,
			IBV_ACCESS_FLUSH_GLOBAL,
				MOCK_ODP_CAPABLE, 0, MOCK_FLUSH_CAPABLE},
	{IBV_TRANSPORT_IWARP,
		RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY,
			IBV_ACCESS_FLUSH_GLOBAL,
				MOCK_ODP_CAPABLE, 0, MOCK_FLUSH_CAPABLE},
	/* 14-15) non-iWARP and iWARP are the same */
	{IBV_TRANSPORT_IB,
		RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT,
			IBV_ACCESS_FLUSH_PERSISTENT,
				MOCK_ODP_CAPABLE, 0, MOCK_FLUSH_CAPABLE},
	{IBV_TRANSPORT_IWARP,
		RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT,
			IBV_ACCESS_FLUSH_PERSISTENT,
				MOCK_ODP_CAPABLE, 0, MOCK_FLUSH_CAPABLE},
#endif
};

/*
 * mr_reg__reg_mr_ERRNO -- ibv_reg_mr() fails with MOCK_ERRNO
 */
static void
mr_reg__reg_mr_ERRNO(void **pprestate)
{
	struct prestate *prestate = *pprestate;
	prestate->access = MOCK_ACCESS;

#ifdef NATIVE_FLUSH_SUPPORTED
	if (prestate->is_flush_capable)
		prestate->access |= (IBV_ACCESS_FLUSH_GLOBAL | IBV_ACCESS_FLUSH_PERSISTENT);
#endif

	/* configure mocks */
	expect_value(ibv_reg_mr, pd, MOCK_IBV_PD);
	expect_value(ibv_reg_mr, addr, MOCK_ADDR);
	expect_value(ibv_reg_mr, length, MOCK_LEN);
	expect_value(ibv_reg_mr, access, prestate->access);
	will_return(ibv_reg_mr, NULL);
	will_return(ibv_reg_mr, MOCK_ERRNO);

	/* run test */
	struct ibv_mr *mr = NULL;
	int ret = rpma_peer_setup_mr_reg(prestate->peer, &mr, MOCK_ADDR,
				MOCK_LEN, MOCK_USAGE);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(mr);
}

/*
 * mr_reg__reg_mr_EOPNOTSUPP_no_odp -- ibv_reg_mr() fails with EOPNOTSUPP
 */
static void
mr_reg__reg_mr_EOPNOTSUPP_no_odp(void **pprestate)
{
	struct prestate *prestate = *pprestate;
	prestate->access = MOCK_ACCESS;

#ifdef NATIVE_FLUSH_SUPPORTED
	if (prestate->is_flush_capable)
		prestate->access |= (IBV_ACCESS_FLUSH_GLOBAL | IBV_ACCESS_FLUSH_PERSISTENT);
#endif

	/* configure mocks */
	expect_value(ibv_reg_mr, pd, MOCK_IBV_PD);
	expect_value(ibv_reg_mr, addr, MOCK_ADDR);
	expect_value(ibv_reg_mr, length, MOCK_LEN);
	expect_value(ibv_reg_mr, access, prestate->access);
	will_return(ibv_reg_mr, NULL);
	will_return(ibv_reg_mr, EOPNOTSUPP);

	/* run test */
	struct ibv_mr *mr = NULL;
	int ret = rpma_peer_setup_mr_reg(prestate->peer, &mr, MOCK_ADDR,
				MOCK_LEN, MOCK_USAGE);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(mr);
}

/*
 * mr_reg__reg_mr_EOPNOTSUPP_ERRNO -- the first ibv_reg_mr() fails with
 * EOPNOTSUPP whereas the second one fails with MOCK_ERRNO
 */
static void
mr_reg__reg_mr_EOPNOTSUPP_ERRNO(void **pprestate)
{
	struct prestate *prestate = *pprestate;
	prestate->access = MOCK_ACCESS;

#ifdef NATIVE_FLUSH_SUPPORTED
	if (prestate->is_flush_capable)
		prestate->access |= (IBV_ACCESS_FLUSH_GLOBAL | IBV_ACCESS_FLUSH_PERSISTENT);
#endif

	/* configure mocks */
	expect_value(ibv_reg_mr, pd, MOCK_IBV_PD);
	expect_value(ibv_reg_mr, addr, MOCK_ADDR);
	expect_value(ibv_reg_mr, length, MOCK_LEN);
	expect_value(ibv_reg_mr, access, prestate->access);
	will_return(ibv_reg_mr, NULL);
	will_return(ibv_reg_mr, EOPNOTSUPP);

#ifdef ON_DEMAND_PAGING_SUPPORTED
	prestate->access |= IBV_ACCESS_ON_DEMAND;
	expect_value(ibv_reg_mr, pd, MOCK_IBV_PD);
	expect_value(ibv_reg_mr, addr, MOCK_ADDR);
	expect_value(ibv_reg_mr, length, MOCK_LEN);
	expect_value(ibv_reg_mr, access, prestate->access);
	will_return(ibv_reg_mr, NULL);
	will_return(ibv_reg_mr, MOCK_ERRNO);
#endif

	/* run test */
	struct ibv_mr *mr = NULL;
	int ret = rpma_peer_setup_mr_reg(prestate->peer, &mr, MOCK_ADDR,
				MOCK_LEN, MOCK_USAGE);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(mr);
}

/*
 * mr_reg__success -- happy day scenario
 */
static void
mr_reg__success(void **pprestate)
{
	struct prestate *prestate = *pprestate;

	struct rpma_peer *peer = prestate->peer;
	struct ibv_pd *mock_ibv_pd = MOCK_IBV_PD;

	/* configure mocks */
	mock_ibv_pd->context->device->transport_type = prestate->transport_type;
	expect_value(ibv_reg_mr, pd, mock_ibv_pd);
	expect_value(ibv_reg_mr, addr, MOCK_ADDR);
	expect_value(ibv_reg_mr, length, MOCK_LEN);
	expect_value(ibv_reg_mr, access, prestate->access);
	will_return(ibv_reg_mr, MOCK_MR);

	/* run test */
	struct ibv_mr *mr;
	int ret = rpma_peer_setup_mr_reg(peer, &mr, MOCK_ADDR,
				MOCK_LEN, prestate->usage);

	/* verify the results */
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(mr, MOCK_MR);
}

/*
 * mr_reg__success_odp -- happy day scenario ODP style
 */
static void
mr_reg__success_odp(void **pprestate)
{
	struct prestate *prestate = *pprestate;
	prestate->access = MOCK_ACCESS;

#ifdef NATIVE_FLUSH_SUPPORTED
	if (prestate->is_flush_capable)
		prestate->access |= (IBV_ACCESS_FLUSH_GLOBAL | IBV_ACCESS_FLUSH_PERSISTENT);
#endif

	/* configure mocks */
	expect_value(ibv_reg_mr, pd, MOCK_IBV_PD);
	expect_value(ibv_reg_mr, addr, MOCK_ADDR);
	expect_value(ibv_reg_mr, length, MOCK_LEN);
	expect_value(ibv_reg_mr, access, prestate->access);
	will_return(ibv_reg_mr, NULL);
	will_return(ibv_reg_mr, EOPNOTSUPP);

#ifdef ON_DEMAND_PAGING_SUPPORTED
	prestate->access |= IBV_ACCESS_ON_DEMAND;
	expect_value(ibv_reg_mr, pd, MOCK_IBV_PD);
	expect_value(ibv_reg_mr, addr, MOCK_ADDR);
	expect_value(ibv_reg_mr, length, MOCK_LEN);
	expect_value(ibv_reg_mr, access, prestate->access);
	will_return(ibv_reg_mr, MOCK_MR);
#endif

	/* run test */
	struct ibv_mr *mr;
	int ret = rpma_peer_setup_mr_reg(prestate->peer, &mr, MOCK_ADDR,
				MOCK_LEN, MOCK_USAGE);

	/* verify the results */
#ifdef ON_DEMAND_PAGING_SUPPORTED
	assert_int_equal(ret, MOCK_OK);
	assert_ptr_equal(mr, MOCK_MR);
#else
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(mr);
#endif
}

int
main(int argc, char *argv[])
{
	const struct CMUnitTest tests[] = {
		/* rpma_peer_setup_mr_reg() unit tests */
		{ "mr_reg__reg_mr_ERRNO_no_odp", mr_reg__reg_mr_ERRNO,
				setup__peer, teardown__peer, &prestate_Incapable},
		{ "mr_reg__reg_mr_ERRNO_odp", mr_reg__reg_mr_ERRNO,
				setup__peer, teardown__peer, &prestate_Capable},
		cmocka_unit_test_prestate_setup_teardown(
				mr_reg__reg_mr_EOPNOTSUPP_no_odp,
				setup__peer, teardown__peer, &prestate_Incapable),
		cmocka_unit_test_prestate_setup_teardown(
				mr_reg__reg_mr_EOPNOTSUPP_ERRNO,
				setup__peer, teardown__peer, &prestate_Capable),
		{ "mr_reg__USAGE_READ_SRC_IB", mr_reg__success,
				setup__peer, teardown__peer, prestates + 0},
		{ "mr_reg__USAGE_READ_SRC_iWARP", mr_reg__success,
				setup__peer, teardown__peer, prestates + 1},
		{ "mr_reg__USAGE_READ_DST_IB", mr_reg__success,
				setup__peer, teardown__peer, prestates + 2},
		{ "mr_reg__USAGE_READ_DST_iWARP", mr_reg__success,
				setup__peer, teardown__peer, prestates + 3},
		{ "mr_reg__USAGE_WRITE_SRC_IB", mr_reg__success,
				setup__peer, teardown__peer, prestates + 4},
		{ "mr_reg__USAGE_WRITE_SRC_iWARP", mr_reg__success,
				setup__peer, teardown__peer, prestates + 5},
		{ "mr_reg__USAGE_WRITE_DST_IB", mr_reg__success,
				setup__peer, teardown__peer, prestates + 6},
		{ "mr_reg__USAGE_WRITE_DST_iWARP", mr_reg__success,
				setup__peer, teardown__peer, prestates + 7},
		{ "mr_reg__USAGE_RECV_IB", mr_reg__success,
				setup__peer, teardown__peer, prestates + 8},
		{ "mr_reg__USAGE_RECV_iWARP", mr_reg__success,
				setup__peer, teardown__peer, prestates + 9},
		{ "mr_reg__USAGE_FLUSH_TYPE_VISIBILITY_OR_PERSISTENT_IB", mr_reg__success,
				setup__peer, teardown__peer, prestates + 10},
		{ "mr_reg__USAGE_FLUSH_TYPE_VISIBILITY_OR_PERSISTENT_iWARP", mr_reg__success,
				setup__peer, teardown__peer, prestates + 11},
#ifdef NATIVE_FLUSH_SUPPORTED
		{ "mr_reg__USAGE_NATIVE_FLUSH_TYPE_VISIBILITY_IB", mr_reg__success,
				setup__peer, teardown__peer, prestates + 12},
		{ "mr_reg__USAGE_NATIVE_FLUSH_TYPE_VISIBILITY_iWARP", mr_reg__success,
				setup__peer, teardown__peer, prestates + 13},
		{ "mr_reg__USAGE_NATIVE_FLUSH_TYPE_PERSISTENT_IB", mr_reg__success,
				setup__peer, teardown__peer, prestates + 14},
		{ "mr_reg__USAGE_NATIVE_FLUSH_TYPE_PERSISTENT_iWARP", mr_reg__success,
				setup__peer, teardown__peer, prestates + 15},
#endif
		cmocka_unit_test_prestate_setup_teardown(
				mr_reg__success_odp,
				setup__peer, teardown__peer, &prestate_Capable),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
