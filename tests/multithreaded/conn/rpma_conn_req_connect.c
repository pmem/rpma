// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * rpma_conn_req_connect.c -- rpma_conn_req_connect multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "mtt_connect.h"

/* the server's part */

struct prestate {
	char *addr;
	unsigned port;
	struct rpma_peer *peer;
	struct rpma_ep *ep;
};

struct state {
	struct rpma_conn_req *req;
	struct rpma_conn *conn;
};

/*
 * prestate_init -- listen to the clients
 */
static void
prestate_init(void *prestate, struct mtt_result *result)
{
	struct prestate *pr = (struct prestate *)prestate;

	int ret = mtt_server_listen(pr->addr, pr->port, &pr->peer, &pr->ep);
	if (ret)
		MTT_RPMA_ERR(result, "mtt_server_listen", ret);
}

/*
 * seq_init -- allocate a state for the thread
 */
static void
seq_init(unsigned id, void *prestate, void **state_ptr, struct mtt_result *result)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct state *st = (struct state *)calloc(1, sizeof(struct state));
	if (!st) {
		MTT_ERR(result, "calloc", errno);
		return;
	}

	*state_ptr = st;

	/* receive an incoming connection request */
	int ret = rpma_ep_next_conn_req(pr->ep, NULL, &st->req);
	if (ret)
		MTT_RPMA_ERR(result, "rpma_ep_next_conn_req", ret);
}

/*
 * thread -- run rpma_conn_req_connect()
 */
static void
thread(unsigned id, void *prestate, void *state, struct mtt_result *result)
{
	struct state *st = (struct state *)state;

	/*
	 * connect / accept the connection request and obtain the connection object
	 */
	int ret = rpma_conn_req_connect(&st->req, NULL, &st->conn);
	if (ret) {
		MTT_RPMA_ERR(result, "rpma_conn_req_connect", ret);
		(void) rpma_conn_req_delete(&st->req);
	}
}

/*
 * seq_fini -- delete the connection
 */
static void
seq_fini(unsigned id, void *prestate, void **state_ptr, struct mtt_result *result)
{
	struct state *st = (struct state *)*state_ptr;

	int ret = rpma_conn_delete(&st->conn);
	if (ret)
		MTT_RPMA_ERR(result, "rpma_conn_delete", ret);

	free(st);
	*state_ptr = NULL;
}

/*
 * prestate_fini -- delete the endpoint and the peer object
 */
static void
prestate_fini(void *prestate, struct mtt_result *result)
{
	struct prestate *pr = (struct prestate *)prestate;

	mtt_server_shutdown(&pr->peer, &pr->ep);
}

/* the client's part */

struct client_prestate {
	char *addr;
	unsigned port;
	unsigned threads_num;
};

/*
 * client_main -- the main function of the client
 */
int
client_main(char *addr, unsigned port, unsigned threads_num);

/*
 * client_func -- the client function of this test
 */
int
client_func(void *prestate)
{
	struct client_prestate *pst = prestate;
	return client_main(pst->addr, pst->port, pst->threads_num);
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct prestate server_prestate = {args.addr, args.port};
	struct client_prestate client_prestate = {args.addr, args.port, args.threads_num};

	struct mtt_test test = {
			&server_prestate,
			prestate_init,
			seq_init,
			NULL,
			thread,
			NULL,
			seq_fini,
			prestate_fini,
			client_func,
			&client_prestate,
			MTT_START_CHILD_BEFORE_THREAD_SEQ_INIT_FUNC
	};

	return mtt_run(&test, args.threads_num);
}
