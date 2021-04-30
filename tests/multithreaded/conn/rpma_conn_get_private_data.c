// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * rpma_conn_get_private_data.c -- rpma_conn_get_private_data multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "common-conn.h"
#include "mtt.h"

struct prestate {
	char *addr;
	unsigned port;
	struct rpma_peer *peer;
	struct rpma_conn *conn;
};

struct state {
	struct rpma_conn_private_data pdata;
	struct common_data dst_data;
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

	if ((ret = rpma_peer_new(dev, &pr->peer))) {
		MTT_RPMA_ERR(tr, "rpma_peer_new", ret);
		return;
	}

	MTT_PORT_INIT;
	MTT_PORT_SET(pr->port, 0);

	/* establish a new connection to a server listening at addr:port */
	ret = client_connect(pr->peer, pr->addr, MTT_PORT_STR, NULL, &pr->conn);
	if (ret) {
		MTT_RPMA_ERR(tr, "client_connect", ret);
		return;
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
 * thread -- get private data
 */
static void
thread(unsigned id, void *prestate, void *state,
		struct mtt_result *result)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct state *st = (struct state *)state;

	/* receive a memory info from the server */
	int ret = rpma_conn_get_private_data(pr->conn, &st->pdata);
	if (ret) {
		MTT_RPMA_ERR(result, "rpma_conn_get_private_data", ret);
		return;
	} else if (st->pdata.ptr == NULL) {
		MTT_ERR_MSG(result,
			"The server has not provided the connection's private data",
			-1);
		return;
	}

	/* read the received private data */
	struct common_data *dst_data = st->pdata.ptr;
	memcpy(&st->dst_data, dst_data, sizeof(st->dst_data));
}

/*
 * fini -- shutdown the endpoint and free the state
 */
static void
fini(unsigned id, void *prestate, void **state_ptr,
		struct mtt_result *tr)
{
	struct state *st = (struct state *)*state_ptr;

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

	if (pr->conn &&
	    (ret = common_disconnect_and_wait_for_conn_close(&pr->conn)))
		MTT_RPMA_ERR(tr, "common_disconnect_and_wait_for_conn_close",
				ret);

	if (pr->peer && (ret = rpma_peer_delete(&pr->peer)))
		MTT_RPMA_ERR(tr, "rpma_peer_delete", ret);
}

/* the server part */

struct server_prestate {
	int argc;
	char **argv;
};

/*
 * server_main -- the main function of the server
 *                of the 02-read-to-volatile example
 *
 * XXX Temporarily and for the simplicity sake
 * the server is taken directly
 * from the 02-read-to-volatile example.
 */
int server_main(int argc, char *argv[]);

/*
 * server_func -- the server function of this test
 */
int
server_func(void *prestate)
{
	struct server_prestate *pst = prestate;
	return server_main(pst->argc, pst->argv);
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	/* skip the 1st argument (<threads_num>) */
	struct server_prestate server_prestate = {argc - 1, argv + 1};

	struct prestate client_prestate = {args.addr, args.port};

	struct mtt_test test = {
			&client_prestate,
			prestate_init,
			NULL,
			init,
			thread,
			fini,
			NULL,
			prestate_fini,
			server_func,
			&server_prestate
	};

	return mtt_run(&test, args.threads_num);
}
