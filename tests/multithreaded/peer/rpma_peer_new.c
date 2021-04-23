// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_peer_new.c -- rpma_peer_new multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"

struct prestate {
	char *addr;
	struct ibv_context *dev;
};

struct state {
	struct rpma_peer *peer;
};

/*
 * prestate_init -- obtain an ibv_context for a remote IP address
 */
static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret = rpma_utils_get_ibv_context(pr->addr,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &pr->dev);
	if (ret)
		MTT_RPMA_ERR(tr, "rpma_utils_get_ibv_context", ret);
}

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
 * thread -- create rpma_peer based on shared ibv_context
 */
static void
thread(unsigned id, void *prestate, void *state,
		struct mtt_result *result)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct state *st = (struct state *)state;

	/* create a new peer object */
	int ret = rpma_peer_new(pr->dev, &st->peer);
	if (ret) {
		MTT_RPMA_ERR(result, "rpma_peer_new", ret);
		return;
	}
}

/*
 * fini -- delete rpma_peer and free the state
 */
static void
fini(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;
	int ret;

	/* delete the peer object */
	if ((ret = rpma_peer_delete(&st->peer)))
		MTT_RPMA_ERR(tr, "rpma_peer_delete", ret);

	free(st);
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct prestate prestate = {args.addr, NULL};

	struct mtt_test test = {
			&prestate,
			prestate_init,
			init,
			NULL,
			thread,
			fini,
			NULL,
			NULL
	};

	return mtt_run(&test, args.threads_num);
}
