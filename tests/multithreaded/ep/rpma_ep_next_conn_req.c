// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_ep_next_conn_req.c -- rpma_ep_next_conn_req multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"

struct prestate {
	char *addr;
	unsigned port;
	struct ibv_context *dev;
	struct rpma_peer *peer;
	struct rpma_ep *ep;
};

struct state {
	struct rpma_conn_req *req;
};

/*
 * prestate_init -- obtain an ibv_context for a local IP address,
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
		(void) rpma_peer_delete(&pr->peer);
	}
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
 * thread -- obtain an incoming connection request
 */
static void
thread(unsigned id, void *prestate, void *state,
		struct mtt_result *result)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct state *st = (struct state *)state;

	int ret = rpma_ep_next_conn_req(pr->ep, NULL, &st->req);
	if (ret)
		MTT_RPMA_ERR(result, "rpma_ep_next_conn_req", ret);
}

/*
 * fini -- delete the connection request and free the state
 */
static void
fini(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;

	if (st->req)
		(void) rpma_conn_req_delete(&st->req);

	free(st);
	*state_ptr = NULL;
}

/*
 * prestate_fini -- shutdown the endpoint and delete the peer object
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

/* the client's part */

struct client_prestate {
	int argc;
	char **argv;
};

/*
 * client_main -- the main function of the client
 *                of the 01-connection example
 */
int client_main(int argc, char *argv[]);

/*
 * client_func -- the client function of this test
 *
 * XXX Temporarily and for the simplicity sake
 * the client is taken directly
 * from the 01-connection example.
 */
int
client_func(void *prestate)
{
	struct client_prestate *pst = prestate;
	int threads_num = strtoul(pst->argv[1], NULL, 10);

	for (int i = 0; i < threads_num; i++)
		client_main(pst->argc - 1, pst->argv + 1);

	return 0;
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct client_prestate client_prestate = {argc, argv};

	struct prestate prestate = {args.addr, args.port, NULL};

	struct mtt_test test = {
			&prestate,
			prestate_init,
			NULL,
			init,
			thread,
			fini,
			NULL,
			prestate_fini,
			client_func,
			&client_prestate
	};

	return mtt_run(&test, args.threads_num);
}
