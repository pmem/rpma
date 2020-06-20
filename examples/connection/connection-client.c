/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * connection-client.c -- a client of the connection example
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <librpma.h>

int
main(int argc, char *argv[])
{
	if (argc < 4) {
		fprintf(stderr, "usage: %s <addr_local> <addr> <service>\n",
			argv[0]);
		abort();
	}

	/* parameters */
	char *addr_local = argv[1];
	char *addr = argv[2];
	char *service = argv[3];

	/* resources */
	struct ibv_context *dev;
	struct rpma_peer *peer;
	struct rpma_conn_req *req;
	struct rpma_conn *conn;
	enum rpma_conn_event conn_event;
	int ret;

	/* request for a connection and wait for its establishment */
	ret = rpma_utils_get_ibv_context(addr_local, RPMA_INFO_ACTIVE, &dev);
	assert(ret == 0);

	ret = rpma_peer_new(dev, &peer);
	assert(ret == 0);

	ret = rpma_conn_req_new(peer, addr, service, &req);
	assert(ret == 0);
	ret = rpma_conn_req_connect(&req, NULL, &conn);
	assert(ret == 0);

	/* wait for the connection to establish */
	ret = rpma_conn_next_event(conn, &conn_event);
	assert(ret == 0 && conn_event == RPMA_CONN_ESTABLISHED);

	/* here you can use the newly established connection */

	/* disconnect the connection */
	ret = rpma_conn_next_event(conn, &conn_event);
	assert(ret == 0 && conn_event == RPMA_CONN_CLOSED);
	ret = rpma_conn_disconnect(conn);
	assert(ret == 0);

	ret = rpma_conn_delete(&conn);
	assert(ret == 0);

	ret = rpma_peer_delete(&peer);
	assert(ret == 0);

	return 0;
}
