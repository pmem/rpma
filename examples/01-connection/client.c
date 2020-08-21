// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * client.c -- a client of the connection example
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <librpma.h>
#include <librpma_log.h>

#ifdef TEST_MOCK_MAIN
#define main client_main
#endif

int
main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s <addr> <port>\n",
			argv[0]);
		exit(-1);
	}

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	/* parameters */
	char *addr = argv[1];
	char *port = argv[2];

	/* resources */
	struct ibv_context *dev = NULL;
	struct rpma_peer *peer = NULL;
	struct rpma_conn_req *req = NULL;
	struct rpma_conn *conn = NULL;
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;
	int ret = 0;

	/* obtain an IBV context for a remote IP address */
	ret = rpma_utils_get_ibv_context(addr, RPMA_UTIL_IBV_CONTEXT_REMOTE,
			&dev);
	if (ret)
		return ret;

	/* create a new peer object */
	ret = rpma_peer_new(dev, &peer);
	if (ret)
		return ret;

	/* create a connection request */
	ret = rpma_conn_req_new(peer, addr, port, &req);
	if (ret)
		goto err_peer_delete;

	/* connect the connection request and obtain the connection object */
	const char *msg = "Hello server!";
	struct rpma_conn_private_data pdata;
	pdata.ptr = (void *)msg;
	pdata.len = (strlen(msg) + 1) * sizeof(char);
	ret = rpma_conn_req_connect(&req, &pdata, &conn);
	if (ret)
		goto err_req_delete;

	/* wait for the connection to establish */
	ret = rpma_conn_next_event(conn, &conn_event);
	if (ret) {
		goto err_conn_delete;
	} else if (conn_event != RPMA_CONN_ESTABLISHED) {
		fprintf(stderr,
				"rpma_conn_next_event returned an unexpected event\n");
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

	/* wait for the connection to being closed */
	ret = rpma_conn_next_event(conn, &conn_event);
	if (ret) {
		goto err_conn_disconnect;
	} else if (conn_event != RPMA_CONN_CLOSED) {
		fprintf(stderr,
				"rpma_conn_next_event returned an unexpected event\n");
		goto err_conn_disconnect;
	}

	/* disconnect the connection */
	ret = rpma_conn_disconnect(conn);
	if (ret)
		goto err_conn_delete;

	/* delete the connection object */
	ret = rpma_conn_delete(&conn);
	if (ret)
		goto err_peer_delete;

	/* delete the peer object */
	ret = rpma_peer_delete(&peer);
	if (ret)
		goto err_exit;

	return 0;

err_conn_disconnect:
	(void) rpma_conn_disconnect(conn);
err_conn_delete:
	(void) rpma_conn_delete(&conn);
err_req_delete:
	if (req)
		(void) rpma_conn_req_delete(&req);
err_peer_delete:
	(void) rpma_peer_delete(&peer);

err_exit:
	return ret;
}
