// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_mr_reg.c -- rpma_mr_reg multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "common-conn.h"

#define HELLO_STR "Hello client!"

struct prestate {
	char *addr;
	unsigned port;
	struct ibv_context *dev;
	struct rpma_peer *peer;
	struct rpma_ep *ep;
};

struct state {
	void *mr_ptr;
	size_t mr_size;
	struct rpma_mr_local *mr;
};

/*
 * prestate_init -- obtain an ibv_context for a remote IP address,
 * create a new peer object and start a listening endpoint
 */
static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret;

	if ((ret = rpma_utils_get_ibv_context(pr->addr,
			RPMA_UTIL_IBV_CONTEXT_LOCAL, &pr->dev))) {
		MTT_RPMA_ERR(tr, "rpma_utils_get_ibv_context", ret);
		return;
	}

	if ((ret = rpma_peer_new(pr->dev, &pr->peer))) {
		MTT_RPMA_ERR(tr, "rpma_peer_new", ret);
		return;
	}

	MTT_PORT_INIT;
	MTT_PORT_SET(pr->port, 0);
	if ((ret = rpma_ep_listen(pr->peer, pr->addr, MTT_PORT_STR, &pr->ep))) {
		MTT_RPMA_ERR(tr, "rpma_ep_listen", ret);
		rpma_peer_delete(&pr->peer);
		return;
	}
}

/*
 * init -- allocate state and allocate and fill the memory
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

	st->mr_size = strlen(HELLO_STR) + 1;
	st->mr_ptr = malloc_aligned(st->mr_size);
	if (!st->mr_ptr) {
		MTT_ERR(tr, "malloc_aligned", errno);
		free(st);
		return;
	}

	/* fill the memory with a content */
	memcpy(st->mr_ptr, HELLO_STR, st->mr_size);
	*state_ptr = st;
}

/*
 * thread -- register the memory
 */
static void
thread(unsigned id, void *prestate, void *state,
		struct mtt_result *result)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct state *st = (struct state *)state;

	int ret = rpma_mr_reg(pr->peer, st->mr_ptr, st->mr_size,
				RPMA_MR_USAGE_READ_SRC, &st->mr);
	if (ret) {
		MTT_RPMA_ERR(result, "rpma_mr_reg", ret);
		free(st->mr_ptr);
		st->mr_ptr = NULL;
	}
}

/*
 * fini -- deregister the memory region and free the state
 */
static void
fini(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;

	int ret;
	if (st->mr_ptr) {
		if ((ret = rpma_mr_dereg(&st->mr)))
			MTT_RPMA_ERR(tr, "rpma_mr_dereg", ret);
		free(st->mr_ptr);
	}

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

	if ((ret = rpma_ep_shutdown(&pr->ep)))
		MTT_RPMA_ERR(tr, "rpma_ep_shutdown", ret);

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
			NULL,
			init,
			thread,
			fini,
			NULL,
			prestate_fini
	};

	return mtt_run(&test, args.threads_num);
}
