// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_srq_new.c -- rpma_srq_new multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"

struct prestate {
	char *addr;
	unsigned port;
	struct rpma_peer *peer;
	struct rpma_srq *srq;
};

struct state {
	struct rpma_cq *rcq;
};

/*
 * prestate_init -- obtain an ibv_context for a remote IP address
 * and create a new peer object
 */
static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct ibv_context *ibv_ctx;
	int ret;

	if ((ret = rpma_utils_get_ibv_context(pr->addr,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &ibv_ctx))) {
		MTT_RPMA_ERR(tr, "rpma_utils_get_ibv_context", ret);
		return;
	}

	if ((ret = rpma_peer_new(ibv_ctx, &pr->peer))) {
		MTT_RPMA_ERR(tr, "rpma_peer_new", ret);
		return;
	}

	if ((ret = rpma_srq_new(pr->peer, NULL, &pr->srq)))
		MTT_RPMA_ERR(tr, "rpma_srq_new", ret);

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
 * thread -- create a connection request based on shared peer object
 */
static void
thread(unsigned id, void *prestate, void *state,
		struct mtt_result *result)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct state *st = (struct state *)state;
	int ret;

	MTT_PORT_INIT;
	MTT_PORT_SET(pr->port, id);
	if ((ret = rpma_srq_get_rcq(pr->srq, &st->rcq)))
		MTT_RPMA_ERR(result, "rpma_srq_get_rcq", ret);
}

/*
 * fini -- delete rpma_conn_req and free the state
 */
static void
fini(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;

	free(st);
}

/*
 * prestate_fini -- delete the peer object and the connection
 * configuration object if it's not NULL
 */
static void
prestate_fini(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	int ret;

	if ((ret = rpma_srq_delete(&pr->srq))) {
		MTT_RPMA_ERR(tr, "rpma_srq_delete", ret);
		return;
	}

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
