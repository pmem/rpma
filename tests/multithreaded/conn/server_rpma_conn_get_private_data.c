// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021-2022, Intel Corporation */

/*
 * server.c -- a server of the rpma_conn_get_private_data MT test
 */

#include <librpma.h>

#include "mtt.h"
#include "mtt_connect.h"

const char data[] = "Hello client!";

int
server_main(char *addr, unsigned port)
{
	struct rpma_peer *peer = NULL;
	struct rpma_ep *ep = NULL;
	struct rpma_conn *conn = NULL;
	int ret;

	ret = mtt_server_listen(addr, port, &peer, &ep);
	if (ret)
		return ret;

	struct rpma_conn_private_data pdata;
	pdata.ptr = (void *)data;
	pdata.len = sizeof(data);

	/*
	 * Wait for an incoming connection request, accept it and wait for its
	 * establishment.
	 */
	ret = mtt_server_accept_connection(ep, &pdata, &conn);
	if (ret)
		goto err_shutdown;

	/*
	 * Wait for RPMA_CONN_CLOSED, disconnect and delete the connection
	 * structure.
	 */
	mtt_server_wait_for_conn_close_and_disconnect(&conn);

err_shutdown:
	/* shutdown the endpoint and delete the peer object */
	mtt_server_shutdown(&peer, &ep);

	return ret;
}
