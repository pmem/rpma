// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * server.c -- a server of the rpma_conn_get_private_data MT test
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <librpma.h>

#include "mtt.h"

const char data[] = "Hello client!";

/*
 * server_accept_connection -- wait for an incoming connection request,
 * accept it and wait for its establishment
 */
int
server_accept_connection(struct rpma_ep *ep,
		struct rpma_conn_private_data *pdata,
		struct rpma_conn **conn_ptr)
{
	struct rpma_conn_req *req = NULL;
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;
	int ret;

	/* receive an incoming connection request */
	if ((ret = rpma_ep_next_conn_req(ep, NULL, &req))) {
		SERVER_RPMA_ERR("rpma_ep_next_conn_req", ret);
		return ret;
	}

	/*
	 * connect / accept the connection request and obtain the connection
	 * object
	 */
	if ((ret = rpma_conn_req_connect(&req, pdata, conn_ptr))) {
		SERVER_RPMA_ERR("rpma_conn_req_connect", ret);
		(void) rpma_conn_req_delete(&req);
		return ret;
	}

	/* wait for the connection to be established */
	if ((ret = rpma_conn_next_event(*conn_ptr, &conn_event)))
		SERVER_RPMA_ERR("rpma_conn_next_event", ret);
	else if (conn_event != RPMA_CONN_ESTABLISHED) {
		SERVER_ERR_MSG(
			"rpma_conn_next_event returned an unexpected event");
		SERVER_ERR_MSG(rpma_utils_conn_event_2str(conn_event));
		ret = -1;
	}

	if (ret)
		(void) rpma_conn_delete(conn_ptr);

	return ret;
}

/*
 * common_wait_for_conn_close_and_disconnect -- wait for RPMA_CONN_CLOSED,
 * disconnect and delete the connection structure
 */
void
common_wait_for_conn_close_and_disconnect(struct rpma_conn **conn_ptr)
{
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;
	int ret = 0;

	/* wait for the connection to be closed */
	if ((ret = rpma_conn_next_event(*conn_ptr, &conn_event)))
		SERVER_RPMA_ERR("rpma_conn_next_event", ret);
	else if (conn_event != RPMA_CONN_CLOSED) {
		SERVER_ERR_MSG(
			"rpma_conn_next_event returned an unexpected event");
		SERVER_ERR_MSG(rpma_utils_conn_event_2str(conn_event));
	}

	if ((ret = rpma_conn_disconnect(*conn_ptr)))
		SERVER_RPMA_ERR("rpma_conn_disconnect", ret);

	if ((ret = rpma_conn_delete(conn_ptr)))
		SERVER_RPMA_ERR("rpma_conn_delete", ret);
}

int
server_main(char *addr, unsigned port)
{
	struct ibv_context *ibv_ctx = NULL;
	struct rpma_peer *peer = NULL;
	struct rpma_ep *ep = NULL;
	struct rpma_conn *conn = NULL;
	int ret;

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	/* lookup an ibv_context via the address */
	if ((ret = rpma_utils_get_ibv_context(addr,
			RPMA_UTIL_IBV_CONTEXT_LOCAL, &ibv_ctx))) {
		SERVER_RPMA_ERR("rpma_utils_get_ibv_context", ret);
		return ret;
	}

	/* create a new peer object */
	if ((ret = rpma_peer_new(ibv_ctx, &peer))) {
		SERVER_RPMA_ERR("rpma_peer_new", ret);
		return ret;
	}

	MTT_PORT_INIT;
	MTT_PORT_SET(port, 0);

	/* start a listening endpoint at addr:port */
	if ((ret = rpma_ep_listen(peer, addr, MTT_PORT_STR, &ep))) {
		SERVER_RPMA_ERR("rpma_ep_listen", ret);
		goto err_peer_delete;
	}

	struct rpma_conn_private_data pdata;
	pdata.ptr = (void *)data;
	pdata.len = sizeof(data);

	/*
	 * Wait for an incoming connection request, accept it and wait for its
	 * establishment.
	 */
	ret = server_accept_connection(ep, &pdata, &conn);
	if (ret)
		goto err_ep_shutdown;

	/*
	 * Wait for RPMA_CONN_CLOSED, disconnect and delete the connection
	 * structure.
	 */
	common_wait_for_conn_close_and_disconnect(&conn);

err_ep_shutdown:
	/* shutdown the endpoint */
	(void) rpma_ep_shutdown(&ep);

err_peer_delete:
	/* delete the peer object */
	(void) rpma_peer_delete(&peer);

	return ret;
}
