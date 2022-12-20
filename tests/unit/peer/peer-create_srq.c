// SPDX-License-Identifier: BSD-3-Clause
/* Copyright (c) 2022, Fujitsu Limited */

/*
 * peer-create_srq.c -- the rpma_peer_create_srq() unit tests
 *
 * API covered:
 * - rpma_peer_create_srq()
 */

#include "librpma.h"
#include "peer.h"
#include "cmocka_headers.h"
#include "mocks-ibverbs.h"
#include "mocks-rpma-srq.h"
#include "mocks-rpma-srq_cfg.h"
#include "peer-common.h"
#include "test-common.h"

static struct srq_cfg_get_mock_args Create_srq_cfg_default = {
	.cfg = MOCK_SRQ_CFG_DEFAULT,
	.rq_size = MOCK_SRQ_SIZE_DEFAULT,
	.rcq_size = MOCK_SRQ_RCQ_SIZE_DEFAULT,
};

static struct srq_cfg_get_mock_args Create_srq_cfg_custom = {
	.cfg = MOCK_SRQ_CFG_CUSTOM,
	.rq_size = MOCK_SRQ_SIZE_CUSTOM,
	.rcq_size = MOCK_SRQ_RCQ_SIZE_CUSTOM,
};

static struct srq_cfg_get_mock_args *cfgs[] = {
	&Create_srq_cfg_default,
	&Create_srq_cfg_custom,
};

static int num_cfgs = sizeof(cfgs) / sizeof(cfgs[0]);

/*
 * create_srq__ibv_create_srq_ERRNO -- ibv_create_srq() fails with MOCK_ERRNO
 */
static void
create_srq__ibv_create_srq_ERRNO(void **pprestate)
{
	struct prestate *prestate = *pprestate;

	/* configure mocks */
	will_return(rpma_srq_cfg_get_rq_size, &Create_srq_cfg_default);
	expect_value(ibv_create_srq, srq_init_attr->attr.max_wr, Create_srq_cfg_default.rq_size);
	will_return(ibv_create_srq, NULL);
	will_return(ibv_create_srq, MOCK_ERRNO);

	/* run test */
	struct ibv_srq *ibv_srq = NULL;
	struct rpma_cq *rcq = NULL;
	int ret = rpma_peer_create_srq(prestate->peer, Create_srq_cfg_default.cfg, &ibv_srq, &rcq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(ibv_srq);
	assert_null(rcq);
}

/*
 * create_srq__rpma_cq_new_ERRNO -- rpma_cq_new() fails with MOCK_ERRNO
 */
static void
create_srq__rpma_cq_new_ERRNO(void **pprestate)
{
	struct prestate *prestate = *pprestate;

	/* configure mocks */
	will_return(rpma_srq_cfg_get_rq_size, &Create_srq_cfg_default);
	expect_value(ibv_create_srq, srq_init_attr->attr.max_wr, Create_srq_cfg_default.rq_size);
	will_return(ibv_create_srq, MOCK_IBV_SRQ);
	will_return(rpma_srq_cfg_get_rcqe, &Create_srq_cfg_default);
	expect_value(rpma_cq_new, cqe, Create_srq_cfg_default.rcq_size);
	expect_value(rpma_cq_new, shared_channel, NULL);
	will_return(rpma_cq_new, NULL);
	will_return(rpma_cq_new, RPMA_E_PROVIDER);
	will_return(rpma_cq_new, MOCK_ERRNO);
	expect_value(ibv_destroy_srq, srq, MOCK_IBV_SRQ);
	will_return(ibv_destroy_srq, MOCK_OK);

	/* run test */
	struct ibv_srq *ibv_srq = NULL;
	struct rpma_cq *rcq = NULL;
	int ret = rpma_peer_create_srq(prestate->peer, Create_srq_cfg_default.cfg, &ibv_srq, &rcq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(ibv_srq);
	assert_null(rcq);
}

/*
 * create_srq__rpma_cq_new_ERRNO_subsequent_ERRNO2 -- ibv_destroy_srq()
 * fails with MOCK_ERRNO2 after rpma_cq_new() failed with MOCK_ERRNO
 */
static void
create_srq__rpma_cq_new_ERRNO_subsequent_ERRNO2(void **pprestate)
{
	struct prestate *prestate = *pprestate;

	/* configure mocks */
	will_return(rpma_srq_cfg_get_rq_size, &Create_srq_cfg_default);
	expect_value(ibv_create_srq, srq_init_attr->attr.max_wr, Create_srq_cfg_default.rq_size);
	will_return(ibv_create_srq, MOCK_IBV_SRQ);
	will_return(rpma_srq_cfg_get_rcqe, &Create_srq_cfg_default);
	expect_value(rpma_cq_new, cqe, Create_srq_cfg_default.rcq_size);
	expect_value(rpma_cq_new, shared_channel, NULL);
	will_return(rpma_cq_new, NULL);
	will_return(rpma_cq_new, RPMA_E_PROVIDER);
	will_return(rpma_cq_new, MOCK_ERRNO);
	expect_value(ibv_destroy_srq, srq, MOCK_IBV_SRQ);
	will_return(ibv_destroy_srq, MOCK_ERRNO2);

	/* run test */
	struct ibv_srq *ibv_srq = NULL;
	struct rpma_cq *rcq = NULL;
	int ret = rpma_peer_create_srq(prestate->peer, Create_srq_cfg_default.cfg, &ibv_srq, &rcq);

	/* verify the result */
	assert_int_equal(ret, RPMA_E_PROVIDER);
	assert_null(ibv_srq);
	assert_null(rcq);
}

/*
 * create_srq__success -- happy day scenario
 */
static void
create_srq__success(void **pprestate)
{
	struct prestate *prestate = *pprestate;

	for (int i = 0; i < num_cfgs; i++) {
		/* configure mocks */
		will_return(rpma_srq_cfg_get_rq_size, cfgs[i]);
		expect_value(ibv_create_srq, srq_init_attr->attr.max_wr, cfgs[i]->rq_size);
		will_return(ibv_create_srq, MOCK_IBV_SRQ);
		will_return(rpma_srq_cfg_get_rcqe, cfgs[i]);
		if (cfgs[i]->rcq_size) {
			expect_value(rpma_cq_new, cqe, cfgs[i]->rcq_size);
			expect_value(rpma_cq_new, shared_channel, NULL);
			will_return(rpma_cq_new, MOCK_RPMA_SRQ_RCQ);
		}

		/* run test */
		struct ibv_srq *ibv_srq = NULL;
		struct rpma_cq *rcq = NULL;
		int ret = rpma_peer_create_srq(prestate->peer, cfgs[i]->cfg, &ibv_srq, &rcq);

		/* verify the result */
		assert_int_equal(ret, MOCK_OK);
		assert_ptr_equal(ibv_srq, MOCK_IBV_SRQ);
		assert_ptr_equal(rcq, cfgs[i]->rcq_size ? MOCK_RPMA_SRQ_RCQ : NULL);
	}
}

static const struct CMUnitTest tests_create_srq[] = {
	/* rpma_peer_create_srq() unit tests */
	cmocka_unit_test_prestate_setup_teardown(
		create_srq__ibv_create_srq_ERRNO, setup__peer, teardown__peer,
		&prestate_Capable),
	cmocka_unit_test_prestate_setup_teardown(
		create_srq__rpma_cq_new_ERRNO, setup__peer, teardown__peer, &prestate_Capable),
	cmocka_unit_test_prestate_setup_teardown(
		create_srq__rpma_cq_new_ERRNO_subsequent_ERRNO2, setup__peer, teardown__peer,
		&prestate_Capable),
	cmocka_unit_test_prestate_setup_teardown(
		create_srq__success, setup__peer, teardown__peer, &prestate_Capable),
	cmocka_unit_test(NULL)
};

int
main(int argc, char *argv[])
{
	return cmocka_run_group_tests(tests_create_srq, NULL, NULL);
}
