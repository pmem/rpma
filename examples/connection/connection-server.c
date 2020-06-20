/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * connection-server.c -- a server of the connection example
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <librpma.h>

int
main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s <addr> <service>\n", argv[0]);
		abort();
	}

	/* parameters */
	char *addr = argv[1];
	char *service = argv[3];

	/* resources */
	struct ibv_context *dev;
	struct rpma_peer *peer;
	struct rpma_ep *ep;
	struct rpma_conn_req *req;
	struct rpma_conn *conn;
	enum rpma_conn_event conn_event;
	int ret;

	/* listen for a connection and establish one */
	ret = rpma_utils_get_ibv_context(addr, RPMA_UTIL_IBV_CONTEXT_LOCAL,
			&dev);
	assert(ret == 0);

	ret = rpma_peer_new(dev, &peer);
	assert(ret == 0);

	ret = rpma_ep_listen(peer, addr, service, &ep);
	assert(ret == 0);
	ret = rpma_ep_next_conn_req(ep, &req);
	assert(ret == 0);
	ret = rpma_conn_req_connect(&req, NULL, &conn);
	assert(ret == 0);

	/* wait for the connection to establish */
	ret = rpma_conn_next_event(conn, &conn_event);
	assert(ret == 0 && conn_event == RPMA_CONN_ESTABLISHED);

	/* here you can use the newly established connection */

	/* disconnect the connection */
	ret = rpma_conn_disconnect(conn);
	assert(ret == 0);
	ret = rpma_conn_next_event(conn, &conn_event);
	assert(ret == 0 && conn_event == RPMA_CONN_CLOSED);

	ret = rpma_conn_delete(&conn);
	assert(ret == 0);

	ret = rpma_ep_shutdown(&ep);
	assert(ret == 0);

	ret = rpma_peer_delete(&peer);
	assert(ret == 0);

	return ret;
}
