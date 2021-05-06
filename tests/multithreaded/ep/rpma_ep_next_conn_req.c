// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_ep_next_conn_req.c -- rpma_ep_next_conn_req multithreaded test
 */

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <librpma.h>

#include "mtt.h"

#define CHILD_PROCESS_DELAY_US		1000

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

struct child_prestate {
	char *addr;
	unsigned port;
	unsigned threads_num;
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
			RPMA_UTIL_IBV_CONTEXT_LOCAL, &pr->dev)))
		MTT_RPMA_ERR(tr, "rpma_utils_get_ibv_context", ret);

	if ((ret = rpma_peer_new(pr->dev, &pr->peer)))
		MTT_RPMA_ERR(tr, "rpma_peer_new", ret);

	MTT_PORT_INIT;
	MTT_PORT_SET(pr->port, 0);
	if ((ret = rpma_ep_listen(pr->peer, pr->addr, MTT_PORT_STR, &pr->ep)))
		MTT_RPMA_ERR(tr, "rpma_ep_listen", ret);
}

/*
 * init -- allocate state
 */
void
init(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
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
	if (ret) {
		MTT_RPMA_ERR(result, "rpma_ep_next_conn_req", ret);
		return;
	}
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

int
child_process(void *prestate)
{
	struct child_prestate *pr = (struct child_prestate *)prestate;

	/* resources */
	struct ibv_context *dev = NULL;
	struct rpma_peer *peer = NULL;
	struct rpma_conn_req *req = NULL;
	struct rpma_conn *conn = NULL;
	int ret = 0;

	usleep(CHILD_PROCESS_DELAY_US);

	/* obtain an IBV context for a remote IP address */
	if ((ret = rpma_utils_get_ibv_context(pr->addr,
			RPMA_UTIL_IBV_CONTEXT_REMOTE, &dev)))
		return ret;

	/* create a new peer object */
	if ((ret = rpma_peer_new(dev, &peer)))
		return ret;

	MTT_PORT_INIT;
	MTT_PORT_SET(pr->port, 0);
	for (int counts = 0; counts < pr->threads_num; counts++) {
		/* create a connection request */
		if ((ret = rpma_conn_req_new(peer, pr->addr,
				MTT_PORT_STR, NULL, &req)))
			goto err_peer_delete;

		if ((ret = rpma_conn_req_connect(&req, NULL, &conn))) {
			if (req)
				(void) rpma_conn_req_delete(&req);
			goto err_peer_delete;
		}

		usleep(CHILD_PROCESS_DELAY_US);

		/* disconnect the connection */
		if ((ret = rpma_conn_disconnect(conn)))
			goto err_conn_delete;

		/* delete the connection object */
		if ((ret = rpma_conn_delete(&conn)))
			goto err_peer_delete;
	}

	/* delete the peer object */
	if ((ret = rpma_peer_delete(&peer)))
		goto err_exit;

	return 0;

err_conn_delete:
	(void) rpma_conn_delete(&conn);
err_peer_delete:
	(void) rpma_peer_delete(&peer);
err_exit:
	return ret;
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct prestate prestate = {args.addr, args.port, NULL, NULL, NULL};
	struct child_prestate cld_prestate = {args.addr, args.port,
				args.threads_num};

	struct mtt_test test = {
			&prestate,
			prestate_init,
			NULL,
			init,
			thread,
			fini,
			NULL,
			prestate_fini,
			child_process,
			&cld_prestate
	};

	return mtt_run(&test, args.threads_num);
}
