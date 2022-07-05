// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_peer_cfg_set_direct_write_to_pmem.c -- rpma_peer_cfg_set_direct_write_to_pmem
 *						multithreaded test
 */

#include <stdlib.h>

#include <librpma.h>

#include "mtt.h"

struct prestate {
	char *addr;
};

struct state {
	struct rpma_peer_cfg *pcfg;
};

#define DIRECT_WRITE_TO_PMEM	true

/*
 * thread_seq_init -- allocate state and create rpma_peer_cfg
 */
void
thread_seq_init(unsigned id, void *prestate, void **state_ptr, struct mtt_result *tr)
{
	int ret;

	struct state *st = (struct state *)calloc(1, sizeof(struct state));
	if (!st) {
		MTT_ERR(tr, "calloc", errno);
		return;
	}

	if ((ret = rpma_peer_cfg_new(&st->pcfg))) {
		MTT_RPMA_ERR(tr, "rpma_peer_cfg_new", ret);
		return;
	}

	*state_ptr = st;
}

/*
 * thread_seq_fini -- delete rpma_peer_cfg and free the state
 */
void
thread_seq_fini(unsigned id, void *prestate, void **state_ptr, struct mtt_result *tr)
{
	int ret;
	struct state *st = (struct state *)*state_ptr;

	/* delete the peer_cfg object */
	if ((st->pcfg != NULL) && (ret = rpma_peer_cfg_delete(&st->pcfg)))
		MTT_RPMA_ERR(tr, "rpma_peer_cfg_delete", ret);

	free(st);
}

/*
 * thread -- create rpma_peer_cfg
 */
static void
thread(unsigned id, void *prestate, void *state, struct mtt_result *result)
{
	int ret;
	struct state *st = (struct state *)state;

	/* create a new peer cfg object */
	if ((ret = rpma_peer_cfg_set_direct_write_to_pmem(st->pcfg, DIRECT_WRITE_TO_PMEM))) {
		MTT_RPMA_ERR(result, "rpma_peer_cfg_set_direct_write_to_pmem", ret);
		thread_seq_fini(id, prestate, &state, result);
		return;
	}

	bool direct_write_to_pmem = false;
	if ((ret = rpma_peer_cfg_get_direct_write_to_pmem(st->pcfg, &direct_write_to_pmem))) {
		MTT_RPMA_ERR(result, "rpma_peer_cfg_get_direct_write_to_pmem", ret);
		thread_seq_fini(id, prestate, &state, result);
		return;
	}

	if (direct_write_to_pmem != DIRECT_WRITE_TO_PMEM)
		MTT_ERR_MSG(result, "Invalid cq_size: %d instead of %d", -1, direct_write_to_pmem,
				DIRECT_WRITE_TO_PMEM);
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct prestate prestate = {args.addr};

	struct mtt_test test = {
			&prestate,
			NULL,
			thread_seq_init,
			NULL,
			thread,
			NULL,
			thread_seq_fini,
			NULL
	};

	return mtt_run(&test, args.threads_num);
}
