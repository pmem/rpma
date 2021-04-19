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
	struct ibv_context *dev;
};

struct state {
	struct rpma_peer *peer;
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
 * thread -- create rpma_peer based on shared ibv_context
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
 * fini -- delete rpma_peer and free the state
 */
static void
fini(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;
	int ret;

	/* delete the peer object */
	if (ret = rpma_peer_delete(&st->peer))
		MTT_RPMA_ERR(tr, "rpma_peer_delete", ret);

	free(st);
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	/* obtain an IBV context for a remote IP address */
	struct prestate prestate;
	int ret = rpma_utils_get_ibv_context(args.addr,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &prestate.dev);
	if (ret) {
		fprintf(stderr, "rpma_utils_get_ibv_context() failed\n");
		return -1;
	}

	struct mtt_test test = {
			&prestate,
			init,
			NULL,
			thread,
			fini,
			NULL
	};

	return mtt_run(&test, args.threads_num);
}
