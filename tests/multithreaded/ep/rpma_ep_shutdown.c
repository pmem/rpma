// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_ep_shutdown.c -- rpma_ep_shutdown multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"

struct prestate {
	char *addr;
	unsigned port;
	struct ibv_context *ibv_ctx;
	struct rpma_peer *peer;
};

struct state {
	struct rpma_ep *ep;
};

/*
 * prestate_init -- obtain an ibv_context for a local IP address
 * and create a new peer object
 */
static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret;

	if ((ret = rpma_utils_get_ibv_context(pr->addr,
			RPMA_UTIL_IBV_CONTEXT_LOCAL, &pr->ibv_ctx))) {
		MTT_RPMA_ERR(tr, "rpma_utils_get_ibv_context", ret);
		return;
	}

	if ((ret = rpma_peer_new(pr->ibv_ctx, &pr->peer)))
		MTT_RPMA_ERR(tr, "rpma_peer_new", ret);
}

/*
 * seq_init -- allocate state and start listening endpoints
 */
void
seq_init(unsigned id, void *prestate, void **state_ptr, struct mtt_result *result)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct state *st = (struct state *)calloc(1, sizeof(struct state));
	if (!st) {
		MTT_ERR(result, "calloc", errno);
		return;
	}

	*state_ptr = st;

	MTT_PORT_INIT;
	MTT_PORT_SET(pr->port, id);

	int ret = rpma_ep_listen(pr->peer, pr->addr, MTT_PORT_STR, &st->ep);
	if (ret)
		MTT_RPMA_ERR(result, "rpma_ep_listen", ret);
}


/*
 * thread -- shutdown a listening endpoint
 */
static void
thread(unsigned id, void *prestate, void *state, struct mtt_result *tr)
{
	struct state *st = (struct state *)state;

	/* shutdown and delete the endpoint */
	int ret = rpma_ep_shutdown(&st->ep);
	if (ret)
		MTT_RPMA_ERR(tr, "rpma_ep_shutdown", ret);
}

/*
 * seq_fini -- free the state
 */
static void
seq_fini(unsigned id, void *prestate, void **state_ptr, struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;

	free(st);
	*state_ptr = NULL;
}

/*
 * prestate_fini -- delete the peer object
 */
static void
prestate_fini(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret;

	if ((ret = rpma_peer_delete(&pr->peer)))
		MTT_RPMA_ERR(tr, "rpma_peer_delete", ret);
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct prestate prestate = {args.addr, args.port, NULL, NULL};

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
