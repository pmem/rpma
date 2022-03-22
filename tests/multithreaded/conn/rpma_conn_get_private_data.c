// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021-2022, Intel Corporation */

/*
 * rpma_conn_get_private_data.c -- rpma_conn_get_private_data multithreaded test
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"
#include "utils.h"

/* the client's part */

/*
 * prestate_init -- connect with the server, get the private date
 * and save it in order to verify it later.
 */
static void
prestate_init(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;

	get_private_data(pr, tr);
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
	if (pdata.len != pr->pdata_exp.len) {
		MTT_ERR_MSG(result, "Wrong length of the private data", -1);
		return;
	}

	/* verify the content of the received private data */
	if (memcmp(pdata.ptr, pr->pdata_exp.ptr, pdata.len) != 0)
		MTT_ERR_MSG(result, "Wrong content of the private data", -1);
}

/*
 * prestate_fini -- disconnect and delete the peer object
 */
static void
prestate_fini(void *prestate, struct mtt_result *tr)
{
	struct prestate *pr = (struct prestate *)prestate;
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;
	int ret;

	if ((ret = rpma_conn_disconnect(pr->conn))) {
		MTT_RPMA_ERR(tr, "rpma_conn_disconnect", ret);
	} else {
		/* wait for the connection to be closed */
		if ((ret = rpma_conn_next_event(pr->conn, &conn_event)))
			MTT_RPMA_ERR(tr, "rpma_conn_next_event", ret);
		else if (conn_event != RPMA_CONN_CLOSED)
			MTT_ERR_MSG(tr,
				"rpma_conn_next_event returned an unexpected event",
				-1);
	}

	if ((ret = rpma_conn_delete(&pr->conn)))
		MTT_RPMA_ERR(tr, "rpma_conn_delete", ret);

	if ((ret = rpma_peer_delete(&pr->peer)))
		MTT_RPMA_ERR(tr, "rpma_peer_delete", ret);
}

/* the server's part */

struct server_prestate {
	char *addr;
	unsigned port;
};

/*
 * server_main -- the main function of the server
 */
int server_main(char *addr, unsigned port);

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
