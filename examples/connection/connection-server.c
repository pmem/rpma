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
	struct rpma_ep_event ep_event;
	struct rpma_conn *conn;
	enum rpma_conn_event conn_event;
	int ret;

	/* listen for a connection and establish one */
	ret = rpma_utils_get_ibv_context(addr, &dev);
	assert(ret == 0);

	ret = rpma_peer_new(NULL, dev, &peer);
	assert(ret == 0);

	ret = rpma_ep_listen(peer, addr, service, &ep);
	assert(ret == 0);
	ret = rpma_ep_next_event(ep, &ep_event);
	assert(ret == 0 && ep_event.status == RPMA_INCONN_REQUESTED);

	ret = rpma_conn_accept(ep_event.conn, NULL, NULL);
	assert(ret == 0);
	ret = rpma_ep_next_event(ep, &ep_event);
	assert(ret == 0 && ep_event.status == RPMA_INCONN_ESTABLISHED);

	conn = ep_event.conn;

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
