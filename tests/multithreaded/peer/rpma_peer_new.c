// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_peer_new.c -- rpma_peer_new multithreaded test
 */

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

	*state_ptr = st;
}

/*
 * thread -- try to create an rpma_peer based on a shared IBV context
 * and On-Demand Paging capability
 */
static void
thread(unsigned id, void *prestate, void *state,
		struct mtt_result *result)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct state *st = (struct state *)state;

	struct rpma_peer *peer = NULL;
	/* create a new peer object */
	int ret = rpma_peer_new(pr->dev, &peer);
	if (ret) {
		MTT_RPMA_ERR(result, "rpma_peer_new", ret);
		return;
	}

	st->peer = peer;
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

	/* delete the peer object */
	int ret = rpma_peer_delete(&st->peer);
	if (ret)
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
			seq_init,
			NULL,
			thread,
			fini,
			NULL
	};

	return mtt_run(&test, args.threads_num);
}
