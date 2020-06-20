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

static void
print_error(const char *fname, const int ret)
{
	int result = 0;

	if (ret == RPMA_E_PROVIDER) {
		int errnum = rpma_err_get_provider_error();
		const char *errstr = strerror(errnum);
		result = fprintf(stderr, "%s failed: %s (%s)\n", fname,
				rpma_err_2str(ret), errstr);
	} else {
		result = fprintf(stderr, "%s failed: %s\n", fname,
				rpma_err_2str(ret));
	}

	if (result < 0)
		exit(-1);
}

int
main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "usage: %s <addr> <service>\n", argv[0]);
		abort();
	}

	/* parameters */
	char *addr = argv[1];
	char *service = argv[2];

	/* resources */
	struct ibv_context *dev = NULL;
	struct rpma_peer *peer = NULL;
	struct rpma_ep *ep = NULL;
	struct rpma_conn_req *req = NULL;
	struct rpma_conn *conn = NULL;
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;
	int ret = 0;

	/* obtain an IBV context for a local IP address */
	ret = rpma_utils_get_ibv_context(addr, RPMA_UTIL_IBV_CONTEXT_LOCAL,
			&dev);
	if (ret) {
		print_error("rpma_utils_get_ibv_context", ret);
		return -1;
	}

	/* create a new peer object */
	ret = rpma_peer_new(dev, &peer);
	if (ret) {
		print_error("rpma_peer_new", ret);
		return -1;
	}

	/* create a new endpoint object */
	ret = rpma_ep_listen(peer, addr, service, &ep);
	if (ret) {
		print_error("rpma_ep_listen", ret);
		goto err_peer_delete;
	}

	/* obtain an incoming connection request */
	ret = rpma_ep_next_conn_req(ep, &req);
	if (ret) {
		print_error("rpma_ep_next_conn_req", ret);
		goto err_ep_shutdown;
	}

	/*
	 * connect / accept the connection request and obtain the connection
	 * object
	 */
	ret = rpma_conn_req_connect(&req, NULL, &conn);
	if (ret) {
		print_error("rpma_conn_req_connect", ret);
		goto err_req_delete;
	}

	/* wait for the connection to being establish */
	ret = rpma_conn_next_event(conn, &conn_event);
	if (ret) {
		print_error("rpma_conn_next_event", ret);
		goto err_conn_delete;
	} else if (conn_event != RPMA_CONN_ESTABLISHED) {
		fprintf(stderr, "rpma_conn_next_event returned an unexptected "
				"event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
		goto err_conn_delete;
	} else {
		fprintf(stderr, "rpma_conn_next_event returned an event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
	}

	/* here you can use the newly established connection */

	/* disconnect the connection */
	ret = rpma_conn_disconnect(conn);
	if (ret) {
		print_error("rpma_conn_disconnect", ret);
		goto err_conn_delete;
	}

	/* wait for the connection to being closed */
	ret = rpma_conn_next_event(conn, &conn_event);
	if (ret) {
		print_error("rpma_conn_next_event", ret);
		goto err_conn_delete;
	} else if (conn_event != RPMA_CONN_CLOSED) {
		fprintf(stderr, "rpma_conn_next_event returned an unexptected "
				"event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
		goto err_conn_delete;
	} else {
		fprintf(stderr, "rpma_conn_next_event returned an event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
	}

	/* delete the connection object */
	ret = rpma_conn_delete(&conn);
	if (ret) {
		print_error("rpma_conn_delete", ret);
		goto err_ep_shutdown;
	}

	/* shutdown the endpoint */
	ret = rpma_ep_shutdown(&ep);
	if (ret) {
		print_error("rpma_ep_shutdown", ret);
		goto err_peer_delete;
	}

	/* delete the peer object */
	ret = rpma_peer_delete(&peer);
	if (ret) {
		print_error("rpma_peer_delete", ret);
		goto err_exit;
	}

	return 0;

err_conn_delete:
	(void) rpma_conn_delete(&conn);
err_req_delete:
	if (req)
		(void) rpma_conn_req_delete(&req);
err_ep_shutdown:
	(void) rpma_ep_shutdown(&ep);
err_peer_delete:
	(void) rpma_peer_delete(&peer);

err_exit:
	return -1;
}
