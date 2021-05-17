// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_conn_req_new.c -- rpma_conn_req_new multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"

struct prestate {
	char *addr;
	unsigned port;
	struct rpma_peer *peer;
};

struct prestate2 {
	char *addr;
	unsigned port;
	struct rpma_peer *peer;
	struct rpma_conn_cfg *cfg;
};

struct state {
	struct rpma_conn_req *req;
};

/*
 * prestate_init -- obtain an ibv_context for a remote IP address
 * and create a new peer object
 */
static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct ibv_context *dev;
	int ret;

	if ((ret = rpma_utils_get_ibv_context(pr->addr,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &dev))) {
		MTT_RPMA_ERR(tr, "rpma_utils_get_ibv_context", ret);
		return;
	}

	if ((ret = rpma_peer_new(dev, &pr->peer)))
		MTT_RPMA_ERR(tr, "rpma_peer_new", ret);
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
	if ((ret = rpma_conn_req_new(pr->peer, pr->addr,
				MTT_PORT_STR, NULL, &st->req)))
		MTT_RPMA_ERR(result, "rpma_conn_req_new", ret);
}

/*
 * fini -- delete rpma_conn_req and free the state
 */
static void
fini(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;
	int ret;

	if (st->req && (ret = rpma_conn_req_delete(&st->req)))
		MTT_RPMA_ERR(tr, "rpma_conn_req_delete", ret);

	free(st);
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

/*
 * prestate_init2 -- obtain an ibv_context for a remote IP address,
 * create a new peer object and a new connection configuration object
 */
static void
prestate_init2(void *prestate, struct mtt_result *tr)
{
	struct prestate2 *pr = (struct prestate2 *)prestate;
	struct ibv_context *dev;
	int ret;

	if ((ret = rpma_utils_get_ibv_context(pr->addr,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &dev))) {
		MTT_RPMA_ERR(tr, "rpma_utils_get_ibv_context", ret);
		return;
	}

	if ((ret = rpma_peer_new(dev, &pr->peer))) {
		MTT_RPMA_ERR(tr, "rpma_peer_new", ret);
		return;
	}

	if ((ret = rpma_conn_cfg_new(&pr->cfg))) {
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_new", ret);
		(void) rpma_peer_delete(&pr->peer);
	}
}

/*
 * thread2 -- create a connection request based on shared peer object
 * and connection configuration object
 */
static void
thread2(unsigned id, void *prestate, void *state,
		struct mtt_result *result)
{
	struct prestate2 *pr = (struct prestate2 *)prestate;
	struct state *st = (struct state *)state;
	int ret;

	MTT_PORT_INIT;
	MTT_PORT_SET(pr->port, id);
	if ((ret = rpma_conn_req_new(pr->peer, pr->addr,
				MTT_PORT_STR, pr->cfg, &st->req)))
		MTT_RPMA_ERR(result, "rpma_conn_req_new", ret);
}

/*
 * prestate_fini2 -- delete the peer object
 */
static void
prestate_fini2(void *prestate, struct mtt_result *tr)
{
	struct prestate2 *pr = (struct prestate2 *)prestate;
	int ret;

	if ((ret = rpma_peer_delete(&pr->peer)))
		MTT_RPMA_ERR(tr, "rpma_peer_delete", ret);

	if ((ret = rpma_conn_cfg_delete(&pr->cfg)))
		MTT_RPMA_ERR(tr, "rpma_conn_cfg_delete", ret);
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};
	int ret;

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct prestate prestate = {args.addr, args.port, NULL};
	struct prestate2 prestate2 = {args.addr, args.port, NULL};

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

	struct mtt_test test2 = {
			&prestate2,
			prestate_init2,
			NULL,
			init,
			thread2,
			fini,
			NULL,
			prestate_fini2
	};

	if ((ret = mtt_run(&test, args.threads_num)))
		return ret;
	return mtt_run(&test2, args.threads_num);
}
