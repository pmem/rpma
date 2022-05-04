// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021-2022, Intel Corporation */

/*
 * rpma_conn_get_private_data.c -- rpma_conn_get_private_data multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "mtt_connect.h"

/* the client's part */

struct prestate {
	char *addr;
	unsigned port;
	struct rpma_peer *peer;
	struct rpma_conn *conn;

	/* the expected value of the private data */
	struct rpma_conn_private_data pdata;
};

/*
 * prestate_init -- connect with the server, get the private data
 * and save it in order to verify it later
 */
static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;

	if (mtt_client_peer_new(tr, pr->addr, &pr->peer))
		return;

	if (mtt_client_connect(tr, pr->addr, pr->port, pr->peer, &pr->conn, &pr->pdata))
		mtt_client_peer_delete(tr, &pr->peer);
}

/*
 * thread -- get and verify the private data
 */
static void
thread(unsigned id, void *prestate, void *state,
		struct mtt_result *result)
{
	struct prestate *pr = (struct prestate *)prestate;
	struct rpma_conn_private_data pdata;

	/* get a connection's private data */
	int ret = rpma_conn_get_private_data(pr->conn, &pdata);
	if (ret) {
		MTT_RPMA_ERR(result, "rpma_conn_get_private_data", ret);
		return;
	} else if (pdata.ptr == NULL) {
		MTT_ERR_MSG(result,
			"The server has not provided the connection's private data",
			-1);
		return;
	}

	/* verify the length of the received private data */
	if (pdata.len != pr->pdata.len) {
		MTT_ERR_MSG(result, "Wrong length of the private data", -1);
		return;
	}

	/* verify the content of the received private data */
	if (memcmp(pdata.ptr, pr->pdata.ptr, pdata.len) != 0)
		MTT_ERR_MSG(result, "Wrong content of the private data", -1);
}

/*
 * prestate_fini -- disconnect and delete the peer object
 */
static void
prestate_fini(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;

	mtt_client_disconnect(tr, &pr->conn);
	mtt_client_peer_delete(tr, &pr->peer);
}

/* the server's part */

struct server_prestate {
	char *addr;
	unsigned port;
};

/*
 * server_main -- the main function of the server
 */
int
server_main(char *addr, unsigned port);

/*
 * server_func -- the server function of this test
 */
int
server_func(void *prestate)
{
	struct server_prestate *pst = prestate;
	return server_main(pst->addr, pst->port);
}

int
main(int argc, char *argv[])
{
	struct mtt_args args = {0};

	if (mtt_parse_args(argc, argv, &args))
		return -1;

	struct prestate client_prestate = {args.addr, args.port};
	struct server_prestate server_prestate = {args.addr, args.port};

	struct mtt_test test = {
			&client_prestate,
			prestate_init,
			NULL,
			NULL,
			thread,
			NULL,
			NULL,
			prestate_fini,
			server_func,
			&server_prestate
	};

	return mtt_run(&test, args.threads_num);
}
