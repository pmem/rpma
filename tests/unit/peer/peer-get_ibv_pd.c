// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Fujitsu */

/*
 * peer-get_ibv_pd.c -- the rpma_peer_get_ibv_pd() unit tests
 *
 * API covered:
 * - rpma_peer_get_ibv_pd()
 */

#include "librpma.h"
#include "peer.h"
#include "cmocka_headers.h"
#include "mocks-ibverbs.h"
#include "peer-common.h"

/*
 * get_ibv_pd__success -- happy day scenario
 */
static void
get_ibv_pd__success(void **pprestate)
{
	struct prestate *prestate = *pprestate;

	/* run test */
	struct ibv_pd *pd = rpma_peer_get_ibv_pd(prestate->peer);

	/* verify the results */
	assert_int_equal(pd, MOCK_IBV_PD);
}

static const struct CMUnitTest tests_get_ibv_pd[] = {
	/* rpma_cq_get_fd() unit tests */
	cmocka_unit_test_prestate_setup_teardown(
		get_ibv_pd__success, setup__peer, teardown__peer,
		&prestate_OdpCapable),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_get_ibv_pd, NULL, NULL);
}
