/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, Intel Corporation
 */

/*
 * client.c -- a client of the connection example
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <librpma.h>

#ifdef TEST_MOCKS
#define main client_main
#endif

static void
print_error_ex(const char *fname, const int ret)
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
		fprintf(stderr, "usage: %s <addr> <service>\n",
			argv[0]);
		exit(-1);
	}

	/* parameters */
	char *addr = argv[1];
	char *service = argv[2];

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
	if (ret) {
		print_error_ex("rpma_utils_get_ibv_context", ret);
		return -1;
	}

	/* create a new peer object */
	ret = rpma_peer_new(dev, &peer);
	if (ret) {
		print_error_ex("rpma_peer_new", ret);
		return -1;
	}

	/* create a connection request */
	ret = rpma_conn_req_new(peer, addr, service, &req);
	if (ret) {
		print_error_ex("rpma_conn_req_new", ret);
		goto err_peer_delete;
	}

	/* connect the connection request and obtain the connection object */
	const char *msg = "Hello server!";
	struct rpma_conn_private_data pdata;
	pdata.ptr = (void *)msg;
	pdata.len = (strlen(msg) + 1) * sizeof(char);
	ret = rpma_conn_req_connect(&req, &pdata, &conn);
	if (ret) {
		print_error_ex("rpma_conn_req_connect", ret);
		goto err_req_delete;
	}

	/* wait for the connection to establish */
	ret = rpma_conn_next_event(conn, &conn_event);
	if (ret) {
		print_error_ex("rpma_conn_next_event", ret);
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
		print_error_ex("rpma_conn_next_event", ret);
		goto err_conn_disconnect;
	} else if (conn_event != RPMA_CONN_CLOSED) {
		fprintf(stderr, "rpma_conn_next_event returned an unexptected "
				"event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
		goto err_conn_disconnect;
	} else {
		fprintf(stderr, "rpma_conn_next_event returned an event: %s\n",
				rpma_utils_conn_event_2str(conn_event));
	}

	/* disconnect the connection */
	ret = rpma_conn_disconnect(conn);
	if (ret) {
		print_error_ex("rpma_conn_disconnect", ret);
		goto err_conn_delete;
	}

	/* delete the connection object */
	ret = rpma_conn_delete(&conn);
	if (ret) {
		print_error_ex("rpma_conn_delete", ret);
		goto err_peer_delete;
	}

	/* delete the peer object */
	ret = rpma_peer_delete(&peer);
	if (ret) {
		print_error_ex("rpma_peer_delete", ret);
		goto err_exit;
	}

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
	return -1;
}
