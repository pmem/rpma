// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/*
 * mtt_connect.c -- common connection code of multithreaded tests
 */

#include <stdlib.h>
#include <librpma.h>

#include "mtt.h"

/* maximum number of client's connection retries */
#define MAX_CONN_RETRY 10

/*
 * mtt_server_listen -- start a listening endpoint at addr:port
 */
int
mtt_server_listen(char *addr, unsigned port, struct rpma_peer **peer_ptr,
		struct rpma_ep **ep_ptr)
{
	struct ibv_context *ibv_ctx = NULL;
	int ret;

	*peer_ptr = NULL;
	*ep_ptr = NULL;

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	/* lookup an ibv_context via the address */
	ret = rpma_utils_get_ibv_context(addr, RPMA_UTIL_IBV_CONTEXT_LOCAL, &ibv_ctx);
	if (ret) {
		SERVER_RPMA_ERR("rpma_utils_get_ibv_context", ret);
		return ret;
	}

	/* create a new peer object */
	ret = rpma_peer_new(ibv_ctx, peer_ptr);
	if (ret) {
		SERVER_RPMA_ERR("rpma_peer_new", ret);
		return ret;
	}

	MTT_PORT_INIT;
	MTT_PORT_SET(port, 0);

	/* start a listening endpoint at addr:port */
	ret = rpma_ep_listen(*peer_ptr, addr, MTT_PORT_STR, ep_ptr);
	if (ret) {
		SERVER_RPMA_ERR("rpma_ep_listen", ret);
		/* delete the peer object */
		(void) rpma_peer_delete(peer_ptr);
		return ret;
	}

	return 0;
}

/*
 * mtt_server_accept_connection -- wait for an incoming connection request,
 * accept it and wait for its establishment
 */
int
mtt_server_accept_connection(struct rpma_ep *ep,
		struct rpma_conn_private_data *pdata,
		struct rpma_conn **conn_ptr)
{
	struct rpma_conn_req *req = NULL;
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;
	int ret;

	/* receive an incoming connection request */
	ret = rpma_ep_next_conn_req(ep, NULL, &req);
	if (ret) {
		SERVER_RPMA_ERR("rpma_ep_next_conn_req", ret);
		return ret;
	}

	/*
	 * connect / accept the connection request and obtain the connection
	 * object
	 */
	ret = rpma_conn_req_connect(&req, pdata, conn_ptr);
	if (ret) {
		SERVER_RPMA_ERR("rpma_conn_req_connect", ret);
		(void) rpma_conn_req_delete(&req);
		return ret;
	}

	/* wait for the connection to be established */
	ret = rpma_conn_next_event(*conn_ptr, &conn_event);
	if (ret)
		SERVER_RPMA_ERR("rpma_conn_next_event", ret);
	else if (conn_event != RPMA_CONN_ESTABLISHED) {
		SERVER_ERR_MSG("rpma_conn_next_event returned an unexpected event");
		SERVER_ERR_MSG(rpma_utils_conn_event_2str(conn_event));
		ret = -1;
	}

	if (ret)
		(void) rpma_conn_delete(conn_ptr);

	return ret;
}

/*
 * mtt_server_wait_for_conn_close_and_disconnect -- wait for RPMA_CONN_CLOSED,
 * disconnect and delete the connection structure
 */
void
mtt_server_wait_for_conn_close_and_disconnect(struct rpma_conn **conn_ptr)
{
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;
	int ret = 0;

	/* wait for the connection to be closed */
	ret = rpma_conn_next_event(*conn_ptr, &conn_event);
	if (ret)
		SERVER_RPMA_ERR("rpma_conn_next_event", ret);
	else if (conn_event != RPMA_CONN_CLOSED) {
		SERVER_ERR_MSG("rpma_conn_next_event returned an unexpected event");
		SERVER_ERR_MSG(rpma_utils_conn_event_2str(conn_event));
	}

	ret = rpma_conn_disconnect(*conn_ptr);
	if (ret)
		SERVER_RPMA_ERR("rpma_conn_disconnect", ret);

	ret = rpma_conn_delete(conn_ptr);
	if (ret)
		SERVER_RPMA_ERR("rpma_conn_delete", ret);
}

/*
 * mtt_server_shutdown -- shutdown the endpoint and delete the peer object
 */
void
mtt_server_shutdown(struct rpma_peer **peer_ptr, struct rpma_ep **ep_ptr)
{
	/* shutdown the endpoint */
	(void) rpma_ep_shutdown(ep_ptr);

	/* delete the peer object */
	(void) rpma_peer_delete(peer_ptr);
}

/*
 * mtt_client_peer_new -- create a new peer
 */
int
mtt_client_peer_new(struct mtt_result *tr, char *addr, struct rpma_peer **peer_ptr)
{
	struct ibv_context *ibv_ctx;
	int ret;

	ret = rpma_utils_get_ibv_context(addr, RPMA_UTIL_IBV_CONTEXT_REMOTE, &ibv_ctx);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_utils_get_ibv_context", ret);
		return -1;
	}

	ret = rpma_peer_new(ibv_ctx, peer_ptr);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_peer_new", ret);
		return -1;
	}

	return 0;
}

/*
 * mtt_client_peer_delete -- delete the peer
 */
void
mtt_client_peer_delete(struct mtt_result *tr, struct rpma_peer **peer_ptr)
{
	int ret;

	ret = rpma_peer_delete(peer_ptr);
	if (ret)
		MTT_RPMA_ERR(tr, "rpma_peer_delete", ret);
}

/*
 * mtt_client_connect -- connect with the server and get the private data
 */
int
mtt_client_connect(struct mtt_result *tr, char *addr, unsigned port,
	struct rpma_peer *peer, struct rpma_conn **conn_ptr,
	struct rpma_conn_private_data *pdata)
{
	struct rpma_conn_req *req = NULL;
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;

	int ret;

	MTT_PORT_INIT;
	MTT_PORT_SET(port, 0);

	int retry = 0;
	do {
		/* create a connection request */
		ret = rpma_conn_req_new(peer, addr, MTT_PORT_STR,
					NULL, &req);
		if (ret) {
			MTT_RPMA_ERR(tr, "rpma_conn_req_new", ret);
			goto err_conn_req_delete;
		}

		/*
		 * Connect the connection request and obtain
		 * the connection object.
		 */
		ret = rpma_conn_req_connect(&req, NULL, conn_ptr);
		if (ret) {
			(void) rpma_conn_req_delete(&req);
			MTT_RPMA_ERR(tr, "rpma_conn_req_connect", ret);
			goto err_conn_req_delete;
		}

		/* wait for the connection to establish */
		ret = rpma_conn_next_event(*conn_ptr, &conn_event);
		if (ret) {
			MTT_RPMA_ERR(tr, "rpma_conn_next_event", ret);
			goto err_conn_delete;
		}

		if (conn_event == RPMA_CONN_ESTABLISHED)
			break;

		if (conn_event != RPMA_CONN_REJECTED) {
			MTT_ERR_MSG(tr, "rpma_conn_next_event returned an unexpected event: %s",
					-1, rpma_utils_conn_event_2str(conn_event));
			goto err_conn_delete;
		}

		retry++;

		if (retry == MAX_CONN_RETRY) {
			MTT_ERR_MSG(tr, "reached the maximum number of retries (%i), exiting ...",
					-1, MAX_CONN_RETRY);
			goto err_conn_delete;
		}

		/* received the RPMA_CONN_REJECTED event, retrying ... */
		(void) rpma_conn_disconnect(*conn_ptr);
		(void) rpma_conn_delete(conn_ptr);

	} while (retry < MAX_CONN_RETRY);

	/* get the connection private data */
	ret = rpma_conn_get_private_data(*conn_ptr, pdata);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_conn_get_private_data", ret);
		goto err_conn_disconnect;
	} else if (pdata->ptr == NULL) {
		MTT_ERR_MSG(tr, "The server has not provided the connection's private data", -1);
		goto err_conn_disconnect;
	}

	return 0;

err_conn_disconnect:
	(void) rpma_conn_disconnect(*conn_ptr);

err_conn_delete:
	(void) rpma_conn_delete(conn_ptr);

err_conn_req_delete:
	(void) rpma_conn_req_delete(&req);

	return -1;
}

/*
 * mtt_client_err_disconnect -- force disconnect and delete the peer object
 *                          in case of an error
 */
void
mtt_client_err_disconnect(struct rpma_conn **conn_ptr)
{
	(void) rpma_conn_disconnect(*conn_ptr);
	(void) rpma_conn_delete(conn_ptr);
}

/*
 * mtt_client_disconnect -- disconnect and delete the peer object
 */
void
mtt_client_disconnect(struct mtt_result *tr, struct rpma_conn **conn_ptr)
{
	enum rpma_conn_event conn_event = RPMA_CONN_UNDEFINED;
	int ret;

	ret = rpma_conn_disconnect(*conn_ptr);
	if (ret) {
		MTT_RPMA_ERR(tr, "rpma_conn_disconnect", ret);
	} else {
		/* wait for the connection to be closed */
		ret = rpma_conn_next_event(*conn_ptr, &conn_event);
		if (ret)
			MTT_RPMA_ERR(tr, "rpma_conn_next_event", ret);
		else if (conn_event != RPMA_CONN_CLOSED)
			MTT_ERR_MSG(tr, "rpma_conn_next_event returned an unexpected event", -1);
	}

	ret = rpma_conn_delete(conn_ptr);
	if (ret)
		MTT_RPMA_ERR(tr, "rpma_conn_delete", ret);
}
