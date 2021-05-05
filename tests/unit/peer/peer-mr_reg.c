// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */

/*
 * peer-mr_reg.c -- a peer unit test
 *
 * API covered:
 * - rpma_peer_mr_reg()
 */

#include <infiniband/verbs.h>

#include "cmocka_headers.h"
#include "conn_req.h"
#include "mocks-ibverbs.h"
#include "mocks-rpma-utils.h"
#include "peer.h"
#include "peer-common.h"
#include "test-common.h"

struct usage2access_s {
	enum ibv_transport_type transport_type;
	int usage;
	unsigned access;
};

static struct usage2access_s usage2access[] = {
	/* 1) non-iWARP and iWARP are the same */
	{IBV_TRANSPORT_IB,
		(RPMA_MR_USAGE_READ_SRC |
		RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY |
		RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT),
			IBV_ACCESS_REMOTE_READ},
	{IBV_TRANSPORT_IWARP,
		(RPMA_MR_USAGE_READ_SRC |
		RPMA_MR_USAGE_FLUSH_TYPE_VISIBILITY |
		RPMA_MR_USAGE_FLUSH_TYPE_PERSISTENT),
			IBV_ACCESS_REMOTE_READ},
	/* 2) non-iWARP and iWARP differs */
	{IBV_TRANSPORT_IB,
		RPMA_MR_USAGE_READ_DST,
			IBV_ACCESS_LOCAL_WRITE},
	{IBV_TRANSPORT_IWARP,
		RPMA_MR_USAGE_READ_DST,
			IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE},
	/* 3) non-iWARP and iWARP are the same */
	{IBV_TRANSPORT_IB,
		RPMA_MR_USAGE_WRITE_SRC,
			IBV_ACCESS_LOCAL_WRITE},
	{IBV_TRANSPORT_IWARP,
		RPMA_MR_USAGE_WRITE_SRC,
			IBV_ACCESS_LOCAL_WRITE},
	/* 4) non-iWARP and iWARP are the same */
	{IBV_TRANSPORT_IB,
		RPMA_MR_USAGE_WRITE_DST,
			IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_LOCAL_WRITE},
	{IBV_TRANSPORT_IWARP,
		RPMA_MR_USAGE_WRITE_DST,
			IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_LOCAL_WRITE},
	/* 5) non-iWARP and iWARP are the same */
	{IBV_TRANSPORT_IB,
		RPMA_MR_USAGE_RECV,
			IBV_ACCESS_LOCAL_WRITE},
	{IBV_TRANSPORT_IWARP,
		RPMA_MR_USAGE_RECV,
			IBV_ACCESS_LOCAL_WRITE},
};

/*
 * mr_reg__fail_ENOMEM -- ibv_reg_mr() failed with ENOMEM
 */
static void
mr_reg__fail_ENOMEM(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;

	/* configure mocks */
	expect_value(ibv_reg_mr, pd, MOCK_IBV_PD);
	expect_value(ibv_reg_mr, addr, MOCK_ADDR);
	expect_value(ibv_reg_mr, length, MOCK_LEN);
	expect_value(ibv_reg_mr, access, MOCK_ACCESS);
	will_return(ibv_reg_mr, NULL);
	will_return(ibv_reg_mr, ENOMEM);

	/* run test */
	struct ibv_mr *mr = NULL;
	int ret = rpma_peer_mr_reg(peer, &mr, MOCK_ADDR,
				MOCK_LEN, MOCK_USAGE);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(mr);
}

/*
 * mr_reg__fail_EOPNOTSUPP_no_odp -- ibv_reg_mr() failed with EOPNOTSUPP
 */
static void
mr_reg__fail_EOPNOTSUPP_no_odp(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;

	/* configure mocks */
	expect_value(ibv_reg_mr, pd, MOCK_IBV_PD);
	expect_value(ibv_reg_mr, addr, MOCK_ADDR);
	expect_value(ibv_reg_mr, length, MOCK_LEN);
	expect_value(ibv_reg_mr, access, MOCK_ACCESS);
	will_return(ibv_reg_mr, NULL);
	will_return(ibv_reg_mr, EOPNOTSUPP);

	/* run test */
	struct ibv_mr *mr = NULL;
	int ret = rpma_peer_mr_reg(peer, &mr, MOCK_ADDR,
				MOCK_LEN, MOCK_USAGE);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(mr);
}

/*
 * mr_reg__fail_EOPNOTSUPP_EAGAIN -- the first ibv_reg_mr() fails with
 * EOPNOTSUPP whereas the second one fails with EAGAIN
 */
static void
mr_reg__fail_EOPNOTSUPP_EAGAIN(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;

	/* configure mocks */
	expect_value(ibv_reg_mr, pd, MOCK_IBV_PD);
	expect_value(ibv_reg_mr, addr, MOCK_ADDR);
	expect_value(ibv_reg_mr, length, MOCK_LEN);
	expect_value(ibv_reg_mr, access, MOCK_ACCESS);
	will_return(ibv_reg_mr, NULL);
	will_return(ibv_reg_mr, EOPNOTSUPP);

#ifdef ON_DEMAND_PAGING_SUPPORTED
	expect_value(ibv_reg_mr, pd, MOCK_IBV_PD);
	expect_value(ibv_reg_mr, addr, MOCK_ADDR);
	expect_value(ibv_reg_mr, length, MOCK_LEN);
	expect_value(ibv_reg_mr, access, MOCK_ACCESS | IBV_ACCESS_ON_DEMAND);
	will_return(ibv_reg_mr, NULL);
	will_return(ibv_reg_mr, EAGAIN);
#endif

	/* run test */
	struct ibv_mr *mr = NULL;
	int ret = rpma_peer_mr_reg(peer, &mr, MOCK_ADDR,
				MOCK_LEN, MOCK_USAGE);

	/* verify the results */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(mr);
}

/*
 * mr_reg__success -- happy day scenario
 */
static void
mr_reg__success(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;
	struct ibv_pd *mock_ibv_pd = MOCK_IBV_PD;

	int n_iters = sizeof(usage2access) / sizeof(struct usage2access_s);
	int i;

	/* iterate over different access/usage values */
	for (i = 0; i < n_iters; i++) {
		/* configure mocks */
		mock_ibv_pd->context->device->transport_type =
			usage2access[i].transport_type;
		expect_value(ibv_reg_mr, pd, mock_ibv_pd);
		expect_value(ibv_reg_mr, addr, MOCK_ADDR);
		expect_value(ibv_reg_mr, length, MOCK_LEN);
		expect_value(ibv_reg_mr, access, usage2access[i].access);
		will_return(ibv_reg_mr, MOCK_MR);

		/* run test */
		struct ibv_mr *mr;
		int ret = rpma_peer_mr_reg(peer, &mr, MOCK_ADDR,
					MOCK_LEN, usage2access[i].usage);

		/* verify the results */
		assert_int_equal(ret, MOCK_OK);
		assert_ptr_equal(mr, MOCK_MR);
	}
}

/*
 * mr_reg__success_odp -- happy day scenario ODP style
 */
static void
mr_reg__success_odp(void **peer_ptr)
{
	struct rpma_peer *peer = *peer_ptr;

	/* configure mocks */
	expect_value(ibv_reg_mr, pd, MOCK_IBV_PD);
	expect_value(ibv_reg_mr, addr, MOCK_ADDR);
	expect_value(ibv_reg_mr, length, MOCK_LEN);
	expect_value(ibv_reg_mr, access, MOCK_ACCESS);
	will_return(ibv_reg_mr, NULL);
	will_return(ibv_reg_mr, EOPNOTSUPP);

#ifdef ON_DEMAND_PAGING_SUPPORTED
	expect_value(ibv_reg_mr, pd, MOCK_IBV_PD);
	expect_value(ibv_reg_mr, addr, MOCK_ADDR);
	expect_value(ibv_reg_mr, length, MOCK_LEN);
	expect_value(ibv_reg_mr, access, MOCK_ACCESS | IBV_ACCESS_ON_DEMAND);
	will_return(ibv_reg_mr, MOCK_MR);
#endif

	/* run test */
	struct ibv_mr *mr;
	int ret = rpma_peer_mr_reg(peer, &mr, MOCK_ADDR,
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
		/* rpma_peer_mr_reg() unit tests */
		{ "mr_reg__fail_ENOMEM_no_odp", mr_reg__fail_ENOMEM,
				setup__peer, teardown__peer, &OdpIncapable},
		{ "mr_reg__fail_ENOMEM_odp", mr_reg__fail_ENOMEM,
				setup__peer, teardown__peer, &OdpCapable},
		cmocka_unit_test_prestate_setup_teardown(
				mr_reg__fail_EOPNOTSUPP_no_odp,
				setup__peer, teardown__peer, &OdpIncapable),
		cmocka_unit_test_prestate_setup_teardown(
				mr_reg__fail_EOPNOTSUPP_EAGAIN,
				setup__peer, teardown__peer, &OdpCapable),
		cmocka_unit_test_prestate_setup_teardown(
				mr_reg__success,
				setup__peer, teardown__peer, &OdpCapable),
		cmocka_unit_test_prestate_setup_teardown(
				mr_reg__success_odp,
				setup__peer, teardown__peer, &OdpCapable),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
