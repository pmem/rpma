// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_ep_listen.c -- rpma_ep_listen multithreaded test
 */

#include <librpma.h>

#include "mtt.h"

struct prestate {
	char *addr;
	char *port;
	struct ibv_context *dev;
	struct rpma_peer *peer;
};

struct state {
	struct rpma_ep *ep;
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
 * thread -- start a listening endpoint
 */
static void
thread(unsigned id, void *prestate, void *state,
		struct mtt_result *result)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct state *st = (struct state *)state;

	unsigned portn = strtoul(pr->port, NULL, 10);
	MTT_PORT_INIT;
	MTT_PORT_SET(portn, id);

	int ret = rpma_ep_listen(pr->peer, pr->addr, MTT_PORT_STR, &st->ep);
	if (ret) {
		MTT_RPMA_ERR(result, "rpma_ep_listen", ret);
		return;
	}
}

/*
 * fini -- shutdown the endpoint and free the state
 */
static void
fini(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;

	/* shutdown the endpoint */
	int ret = rpma_ep_shutdown(&st->ep);
	if (ret)
		MTT_RPMA_ERR(tr, "rpma_ep_shutdown", ret);

	free(st);
	*state_ptr = NULL;
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};
	int ret;

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct prestate prestate = {args.addr, args.port, NULL, NULL};

	/* obtain an IBV context for a remote IP address */
	if ((ret = rpma_utils_get_ibv_context(args.addr,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &prestate.dev))) {
		fprintf(stderr, "rpma_utils_get_ibv_context failed\n");
		return -1;
	}

	/* create a new peer object */
	if ((ret = rpma_peer_new(prestate.dev, &prestate.peer))) {
		fprintf(stderr, "rpma_peer_new failed\n");
		return;
	}

	struct mtt_test test = {
			&prestate,
			NULL,
			init,
			thread,
			fini,
			NULL
	};

	if ((ret = mtt_run(&test, args.threads_num)))
		return ret;

	/* delete the peer object */
	if ((ret = rpma_peer_delete(&prestate.peer)))
		fprintf(stderr, "rpma_peer_delete failed\n");

	return ret;
}
