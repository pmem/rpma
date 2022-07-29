// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_srq_get_rcq.c -- rpma_srq_get_rcq multithreaded test
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

/*
 * prestate_init -- obtain an ibv_context for a remote IP address, create a new peer object
 * and create a srq
 */
static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct ibv_context *ibv_ctx;
	int ret;

	if ((ret = rpma_utils_get_ibv_context(pr->addr, RPMA_UTIL_IBV_CONTEXT_REMOTE, &ibv_ctx))) {
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
 * thread -- get the receive CQ from the shared RQ object
 */
static void
thread(unsigned id, void *prestate, void *state, struct mtt_result *result)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct rpma_cq *rcq;
	int ret;

	MTT_PORT_INIT;
	MTT_PORT_SET(pr->port, id);

	if ((ret = rpma_srq_get_rcq(pr->srq, &rcq)))
		MTT_RPMA_ERR(result, "rpma_srq_get_rcq", ret);
}

/*
 * prestate_fini -- delete the srq object and the peer object
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
			NULL,
			thread,
			NULL,
			NULL,
			prestate_fini
	};

	return mtt_run(&test, args.threads_num);
}
