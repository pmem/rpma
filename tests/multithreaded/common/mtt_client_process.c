// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * mtt_client_process.c -- process spawning threads with clients for server-side MT tests
 */

#include <librpma.h>

#include "mtt.h"
#include "mtt_connect.h"

const char data[] = "Hello server!";

struct client_prestate {
	char *addr;
	unsigned port;
};

/*
 * thread -- run a single client
 */
static void
thread(unsigned id, void *prestate, void *state,
		struct mtt_result *result)
{
	struct client_prestate *pr = (struct client_prestate *)prestate;

	struct rpma_peer *peer = NULL;
	struct rpma_conn *conn = NULL;

	if (mtt_client_peer_new(result, pr->addr, &peer))
		return;

	struct rpma_conn_private_data pdata;
	pdata.ptr = (void *)data;
	pdata.len = sizeof(data);

	if (mtt_client_connect(result, pr->addr, pr->port, peer, &conn, &pdata)) {
		mtt_client_peer_delete(result, &peer);
		return;
	}

	mtt_client_disconnect(result, &conn);
	mtt_client_peer_delete(result, &peer);
}

int
client_main(char *addr, unsigned port, unsigned threads_num)
{
	struct client_prestate client_prestate = {addr, port};

	struct mtt_test test = {
			&client_prestate,
			NULL,
			NULL,
			NULL,
			thread,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL
	};

	return mtt_run(&test, threads_num);
}
