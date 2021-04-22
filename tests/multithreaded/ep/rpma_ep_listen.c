// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_peer_new.c -- rpma_peer_new multithreaded test
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
 * seq_init -- a sequential step of initialization
 *
 * Everything that has be initialized before the test but is not thread-safe
 * should go here.
 */
static void
seq_init(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct state *st = (struct state *)calloc(1, sizeof(struct state));
	if (!st) {
		MTT_ERR(tr, "calloc", errno);
		return;
	}

	/* obtain an IBV context for a remote IP address */
	int ret = rpma_utils_get_ibv_context(pr->addr,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &pr->dev);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_utils_get_ibv_context", ret);
		return;
	}

	/* create a new peer object */
	ret = rpma_peer_new(pr->dev, &pr->peer);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_peer_new", ret);
		return;
	}

	*state_ptr = st;
}

/*
 * thread -- try to start a listening endpoint at addr:port
 * (input port + thread id)
 */
static void
thread(unsigned id, void *prestate, void *state,
		struct mtt_result *result)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct state *st = (struct state *)state;

	/* create a new endpoint object */
	unsigned long portn = strtoul(pr->port, NULL, 10) + id;
	char ports[MTT_PORTDIGITS_MAX] = {0};
	sprintf(ports, "%d", portn);

	int ret = rpma_ep_listen(pr->peer, pr->addr, ports, &st->ep);
	if (ret) {
		MTT_RPMA_ERR(result, "rpma_ep_listen", ret);
		return;
	}
}

/*
 * fini -- a parallel step of cleanup
 *
 * Everything that can be cleaned up after the test in a thread-safe manner
 * should go here.
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

/*
 * seq_fini -- a sequential step of cleanup
 *
 * Everything that has to be cleaned up after the test but is not thread-safe
 * should go here.
 */
static void
seq_fini(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;

	/* delete the peer object */
	int ret = rpma_peer_delete(&pr->peer);
	if (ret)
		MTT_RPMA_ERR(tr, "rpma_peer_delete", ret);
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};
	int ret;

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct prestate prestate = {args.addr, args.port};

	struct mtt_test test = {
			&prestate,
			seq_init,
			NULL,
			thread,
			fini,
			seq_fini
	};

	return mtt_run(&test, args.threads_num);
}
