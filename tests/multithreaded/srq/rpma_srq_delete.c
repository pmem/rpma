// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_srq_delete.c -- rpma_srq_delete multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"

struct prestate {
	char *addr;
	unsigned port;
	struct rpma_peer *peer;
};

struct state {
	struct rpma_srq *srq;
};

/*
 * prestate_init -- obtain an ibv_context for a remote IP address and create a new peer object
 */
static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct ibv_context *ibv_ctx;
	int ret;

	ret = rpma_utils_get_ibv_context(pr->addr, RPMA_UTIL_IBV_CONTEXT_REMOTE, &ibv_ctx);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_utils_get_ibv_context", ret);
		return;
	}

	ret = rpma_peer_new(ibv_ctx, &pr->peer);
	if (ret)
		MTT_RPMA_ERR(tr, "rpma_peer_new", ret);
}

/*
 * seq_init -- allocate a state and create a srq
 */
void
seq_init(unsigned id, void *prestate, void **state_ptr, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;

	struct state *st = (struct state *)calloc(1, sizeof(struct state));
	if (!st) {
		MTT_ERR(tr, "calloc", errno);
		return;
	}

	MTT_PORT_INIT;
	MTT_PORT_SET(pr->port, id);

	int ret = rpma_srq_new(pr->peer, NULL, &st->srq);
	if (ret)
		MTT_RPMA_ERR(tr, "rpma_srq_new", ret);

	*state_ptr = st;
}

/*
 * thread -- delete a srq
 */
static void
thread(unsigned id, void *prestate, void *state, struct mtt_result *result)
{
	struct state *st = (struct state *)state;

	int ret = rpma_srq_delete(&st->srq);
	if (ret)
		MTT_RPMA_ERR(result, "rpma_srq_delete", ret);
}

/*
 * seq_fini -- free the state
 */
static void
seq_fini(unsigned id, void *prestate, void **state_ptr, struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;

	free(st);
}

/*
 * prestate_fini -- delete the peer object
 */
static void
prestate_fini(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;

	int ret = rpma_peer_delete(&pr->peer);
	if (ret)
		MTT_RPMA_ERR(tr, "rpma_peer_delete", ret);
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct prestate prestate = {args.addr, args.port, NULL};

	struct mtt_test test = {
			&prestate,
			prestate_init,
			seq_init,
			NULL,
			thread,
			NULL,
			seq_fini,
			prestate_fini
	};

	return mtt_run(&test, args.threads_num);
}
