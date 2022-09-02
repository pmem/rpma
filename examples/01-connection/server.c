// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */

/*
 * server.c -- a server of the connection example
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <librpma.h>

#ifdef TEST_MOCK_MAIN
#define main server_main
#endif

int
main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s <addr> <port>\n", argv[0]);
		return -1;
	}

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	/* parameters */
	char *addr = argv[1];
	char *port = argv[2];

	/* resources */
	struct ibv_context *ibv_ctx = NULL;
	struct rpma_peer *peer = NULL;
	struct rpma_ep *ep = NULL;
	struct rpma_conn_req *req = NULL;
	struct rpma_conn *conn = NULL;
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;
	int ret = 0;

	/* obtain an IBV context for a local IP address */
	ret = rpma_utils_get_ibv_context(addr, RPMA_UTIL_IBV_CONTEXT_LOCAL, &ibv_ctx);
	if (ret)
		return ret;

	/* create a new peer object */
	ret = rpma_peer_new(ibv_ctx, &peer);
	if (ret)
		return ret;

	/* create a new endpoint object */
	ret = rpma_ep_listen(peer, addr, port, &ep);
	if (ret)
		goto err_peer_delete;

	/* obtain an incoming connection request */
	ret = rpma_ep_next_conn_req(ep, NULL, &req);
	if (ret)
		goto err_ep_shutdown;

	/*
	 * connect / accept the connection request and obtain the connection object
	 */
	const char *msg = "Hello client!";
	struct rpma_conn_private_data pdata;
	pdata.ptr = (void *)msg;
	pdata.len = (strlen(msg) + 1) * sizeof(char);
	ret = rpma_conn_req_connect(&req, &pdata, &conn);
	if (ret)
		goto err_ep_shutdown;

	/* wait for the connection to being establish */
	ret = rpma_conn_next_event(conn, &conn_event);
	if (ret) {
		goto err_conn_delete;
	} else if (conn_event != RPMA_CONN_ESTABLISHED) {
		fprintf(stderr, "rpma_conn_next_event returned an unexpected event: %s\n",
			rpma_utils_conn_event_2str(conn_event));
		goto err_conn_delete;
	}

	/* here you can use the newly established connection */
	(void) rpma_conn_get_private_data(conn, &pdata);
	if (pdata.ptr) {
		char *msg = pdata.ptr;
		fprintf(stdout, "Received a message: %s\n", msg);
	} else {
		fprintf(stdout, "No message received\n");
	}

	/* disconnect the connection */
	ret = rpma_conn_disconnect(conn);
	if (ret)
		goto err_conn_delete;

	/* wait for the connection to being closed */
	ret = rpma_conn_next_event(conn, &conn_event);
	if (ret) {
		goto err_conn_delete;
	} else if (conn_event != RPMA_CONN_CLOSED) {
		fprintf(stderr, "rpma_conn_next_event returned an unexpected event: %s\n",
			rpma_utils_conn_event_2str(conn_event));
		goto err_conn_delete;
	}

	/* delete the connection object */
	ret = rpma_conn_delete(&conn);
	if (ret)
		goto err_ep_shutdown;

	/* shutdown the endpoint */
	ret = rpma_ep_shutdown(&ep);
	if (ret)
		goto err_peer_delete;

	/* delete the peer object */
	ret = rpma_peer_delete(&peer);
	if (ret)
		goto err_exit;

	return 0;

err_conn_delete:
	(void) rpma_conn_delete(&conn);
err_ep_shutdown:
	(void) rpma_ep_shutdown(&ep);
err_peer_delete:
	(void) rpma_peer_delete(&peer);

err_exit:
	return ret;
}
