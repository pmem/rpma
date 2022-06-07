// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021-2022, Intel Corporation */

/*
 * rpma_peer_cfg_new.c -- rpma_peer_cfg_new multithreaded test
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

/*
 * init -- allocate state
 */
void
init(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct state *st = (struct state *)calloc(1, sizeof(struct state));
	if (!st) {
		MTT_ERR(tr, "calloc", errno);
		return;
	}

	*state_ptr = st;
}

/*
 * thread -- create rpma_peer_cfg
 */
static void
thread(unsigned id, void *prestate, void *state,
		struct mtt_result *result)
{
	struct state *st = (struct state *)state;

	/* create a new peer cfg object */
	int ret = rpma_peer_cfg_new(&st->pcfg);
	if (ret) {
		MTT_RPMA_ERR(result, "rpma_peer_cfg_new", ret);
		return;
	}
}

/*
 * fini -- delete rpma_peer_cfg and free the state
 */
static void
fini(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;
	int ret;

	/* delete the peer_cfg object */
	if ((st->pcfg != NULL) && (ret = rpma_peer_cfg_delete(&st->pcfg)))
		MTT_RPMA_ERR(tr, "rpma_peer_cfg_delete", ret);
	free(st);
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
			NULL,
			init,
			thread,
			fini,
			NULL,
			NULL
	};

	return mtt_run(&test, args.threads_num);
}
