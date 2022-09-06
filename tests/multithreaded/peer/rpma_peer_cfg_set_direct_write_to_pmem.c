// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_peer_cfg_set_direct_write_to_pmem.c -- rpma_peer_cfg_set_direct_write_to_pmem MT test
 */

#include <stdlib.h>

#include <librpma.h>

#include "mtt.h"

#define DIRECT_WRITE_TO_PMEM	true

struct prestate {
	struct rpma_peer_cfg *pcfg;
};

struct prestate prestate = {NULL};

/*
 * prestate_init -- create rpma_peer_cfg
 */
static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;

	int ret = rpma_peer_cfg_new(&pr->pcfg);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_peer_cfg_new", ret);
		return;
	}
}

/*
 * thread -- set peer configuration direct_write_to_pmem value and check if its is as expected
 */
static void
thread(unsigned id, void *prestate, void *state, struct mtt_result *result)
{
	int ret;
	struct prestate *pr = (struct prestate *)prestate;

	/* create a new peer cfg object */
	ret = rpma_peer_cfg_set_direct_write_to_pmem(pr->pcfg, DIRECT_WRITE_TO_PMEM);
	if (ret) {
		MTT_RPMA_ERR(result, "rpma_peer_cfg_set_direct_write_to_pmem", ret);
		return;
	}

	bool direct_write_to_pmem = false;
	ret = rpma_peer_cfg_get_direct_write_to_pmem(pr->pcfg, &direct_write_to_pmem);
	if (ret) {
		MTT_RPMA_ERR(result, "rpma_peer_cfg_get_direct_write_to_pmem", ret);
		return;
	}

	if (direct_write_to_pmem != DIRECT_WRITE_TO_PMEM)
		MTT_ERR_MSG(result, "Invalid cq_size: %d instead of %d", -1, direct_write_to_pmem,
				DIRECT_WRITE_TO_PMEM);
}

/*
 * prestate_fini -- delete rpma_peer_cfg
 */
static void
prestate_fini(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;

	/* delete the peer_cfg object */
	if (pr->pcfg != NULL) {
		int ret = rpma_peer_cfg_delete(&pr->pcfg);
		if (ret)
			MTT_RPMA_ERR(tr, "rpma_peer_cfg_delete", ret);
	}
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct mtt_test test = {
			&prestate,
			prestate_init,
			NULL,
			NULL,
			thread,
			NULL,
			NULL,
			prestate_fini
	};

	return mtt_run(&test, args.threads_num);
}
