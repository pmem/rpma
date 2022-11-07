// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2022, Intel Corporation */
/* Copyright 2021-2022, Fujitsu */

/*
 * server.c -- a server of the multiple-connections example
 *
 * Please see README.md for a detailed description of this example.
 */

#include <inttypes.h>
#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>

#define USAGE_STR "usage: %s <server_address> <port>\n"

#include "common-conn.h"
#include "common-epoll.h"
#include "multiple-connections-common.h"

#define CLIENT_MAX 10

struct client_res {
	/* RPMA resources */
	struct rpma_conn *conn;
	struct rpma_cq *cq;

	/* resources - memory regions */
	size_t offset;

	/* events */
	struct custom_event ev_conn_event;
	struct custom_event ev_conn_cmpl;

	/* parent and identifier */
	struct server_res *svr;
	int client_id;
};

struct server_res {
	/* RPMA resources */
	struct rpma_ep *ep;

	/* resources - memory region */
	void *dst_ptr;
	struct rpma_mr_local *dst_mr;

	/* epoll and event */
	int epoll;
	struct custom_event ev_incoming;

	/* client's resources */
	struct client_res clients[CLIENT_MAX];
};

/*
 * server_init -- initialize server's resources
 */
int
server_init(struct server_res *svr, struct rpma_peer *peer)
{
	int ret = 0;

	svr->epoll = epoll_create1(EPOLL_CLOEXEC);
	if (svr->epoll == -1)
		return errno;

	/* allocate a memory */
	size_t dst_size = MAX_NAME_SIZE * CLIENT_MAX;
	svr->dst_ptr = malloc_aligned(dst_size);
	if (svr->dst_ptr == NULL) {
		close(svr->epoll);
		return -1;
	}

	/* register the memory */
	ret = rpma_mr_reg(peer, svr->dst_ptr, dst_size, RPMA_MR_USAGE_READ_DST, &svr->dst_mr);
	if (ret) {
		free(svr->dst_ptr);
		close(svr->epoll);
	}

	return ret;
}

/*
 * server_fini -- release server's resources
 */
int
server_fini(struct server_res *svr)
{
	/* deregister the memory region */
	int ret = rpma_mr_dereg(&svr->dst_mr);

	/* free the memory */
	free(svr->dst_ptr);

	/* close the epoll */
	if (close(svr->epoll)) {
		if (!ret)
			ret = errno;
	}

	return ret;
}

/*
 * client_new -- find a slot for the incoming client
 */
struct client_res *
client_new(struct server_res *svr)
{
	/* find the first free slot */
	struct client_res *clnt = NULL;
	for (int i = 0; i < CLIENT_MAX; ++i) {
		clnt = &svr->clients[i];
		if (clnt->conn != NULL)
			continue;

		clnt->client_id = i;
		clnt->svr = svr;
		clnt->offset = (size_t)MAX_NAME_SIZE * (size_t)i;
		clnt->ev_conn_cmpl.fd = -1;
		clnt->ev_conn_event.fd = -1;
		break;
	}

	return clnt;
}

void client_handle_completion(struct custom_event *ce);
void client_handle_connection_event(struct custom_event *ce);

/*
 * client_add_to_epoll -- add all client's file descriptors to epoll
 */
int
client_add_to_epoll(struct client_res *clnt, int epoll)
{
	/* get the connection's event fd and add it to epoll */
	int fd;
	int ret = rpma_conn_get_event_fd(clnt->conn, &fd);
	if (ret)
		return ret;
	ret = epoll_add(epoll, fd, clnt, client_handle_connection_event, &clnt->ev_conn_event);
	if (ret)
		return ret;

	/* get the connection's completion fd and add it to epoll */
	ret = rpma_cq_get_fd(clnt->cq, &fd);
	if (ret) {
		epoll_delete(epoll, &clnt->ev_conn_event);
		return ret;
	}
	ret = epoll_add(epoll, fd, clnt, client_handle_completion, &clnt->ev_conn_cmpl);
	if (ret)
		epoll_delete(epoll, &clnt->ev_conn_event);

	return ret;
}

/*
 * client_delete -- release client's resources
 */
void
client_delete(struct client_res *clnt)
{
	struct server_res *svr = clnt->svr;

	if (clnt->ev_conn_cmpl.fd != -1)
		epoll_delete(svr->epoll, &clnt->ev_conn_cmpl);

	if (clnt->ev_conn_event.fd != -1)
		epoll_delete(svr->epoll, &clnt->ev_conn_event);

	/* delete the connection and set conn to NULL */
	(void) rpma_conn_delete(&clnt->conn);
}

/*
 * client_handle_name -- print client's name
 */
void
client_handle_name(struct client_res *clnt, void *ptr)
{
	/* print received name of the client */
	char *name = (char *)ptr + clnt->offset;
	printf("- %s\n", name);
}

/*
 * client_handle_completion -- callback on completion is ready
 *
 * The only expected completion in this example is a success of
 * the IBV_WC_RDMA_READ after which the read client's name is printed
 * to the output.
 * No matter what, the disconnection process will be initiated.
 */
void
client_handle_completion(struct custom_event *ce)
{
	struct client_res *clnt = (struct client_res *)ce->arg;
	const struct server_res *svr = clnt->svr;

	/* wait for the completion to be ready */
	int ret = rpma_cq_wait(clnt->cq);
	if (ret) {
		/* no completion is ready - continue */
		if (ret == RPMA_E_NO_COMPLETION)
			return;

		/* another error occurred - disconnect */
		(void) rpma_conn_disconnect(clnt->conn);
		return;
	}

	/* get next completion */
	struct ibv_wc wc;
	ret = rpma_cq_get_wc(clnt->cq, 1, &wc, NULL);
	if (ret) {
		/* no completion is ready - continue */
		if (ret == RPMA_E_NO_COMPLETION)
			return;

		/* another error occurred - disconnect */
		(void) rpma_conn_disconnect(clnt->conn);
		return;
	}

	/* validate received completion */
	if (wc.status != IBV_WC_SUCCESS) {
		(void) fprintf(stderr, "[%d] rpma_read() failed: %s\n", clnt->client_id,
				ibv_wc_status_str(wc.status));
		(void) rpma_conn_disconnect(clnt->conn);
		return;
	}

	if (wc.opcode != IBV_WC_RDMA_READ) {
		(void) fprintf(stderr,
				"[%d] received unexpected wc.opcode value (%d != %d)\n",
				clnt->client_id, wc.opcode, IBV_WC_RDMA_READ);
		(void) rpma_conn_disconnect(clnt->conn);
		return;
	}

	/* print received name of the client */
	client_handle_name(clnt, svr->dst_ptr);

	/* initiate disconnection process */
	(void) rpma_conn_disconnect(clnt->conn);
}

/*
 * client_fetch_name -- read the client's name from the remote memory region
 */
int
client_fetch_name(struct client_res *clnt, struct rpma_mr_local *dst)
{
	/* get connection's private data */
	struct rpma_conn_private_data pdata;
	int ret = rpma_conn_get_private_data(clnt->conn, &pdata);
	if (ret) {
		(void) fprintf(stderr, "rpma_conn_get_private_data() failed\n");
		return -1;
	}
	if (pdata.len < sizeof(struct common_data)) {
		(void) fprintf(stderr,
				"[%d] received connection's private data is too small (%d < %zu)\n",
				clnt->client_id, pdata.len, sizeof(struct common_data));
		return -1;
	}

	/* prepare a remote memory region */
	struct common_data *dst_data = pdata.ptr;

	struct rpma_mr_remote *src_mr;
	ret = rpma_mr_remote_from_descriptor(&dst_data->descriptors[0], dst_data->mr_desc_size,
			&src_mr);
	if (ret)
		return ret;

	/* read client's name from the remote memory region */
	ret = rpma_read(clnt->conn, dst, clnt->offset, src_mr, 0, MAX_NAME_SIZE,
			RPMA_F_COMPLETION_ALWAYS, NULL);
	if (ret) {
		(void) rpma_mr_remote_delete(&src_mr);
		return ret;
	}

	/* delete the remote memory region's object */
	(void) rpma_mr_remote_delete(&src_mr);

	return 0;
}

/*
 * client_handle_is_ready -- callback on connection is established
 *
 * The client should send via the private data the remote memory regions
 * descriptor which allows reading client's name.
 * If any of the required steps fail the client will be disconnected.
 */
void
client_handle_is_ready(struct client_res *clnt)
{
	const struct server_res *svr = clnt->svr;

	if (client_fetch_name(clnt, svr->dst_mr))
		(void) rpma_conn_disconnect(clnt->conn);
}

/*
 * client_handle_connection_event -- callback on connection's next event
 */
void
client_handle_connection_event(struct custom_event *ce)
{
	struct client_res *clnt = (struct client_res *)ce->arg;

	/* get next connection's event */
	enum rpma_conn_event event = RPMA_CONN_UNDEFINED;
	int ret = rpma_conn_next_event(clnt->conn, &event);
	if (ret) {
		if (ret == RPMA_E_NO_EVENT)
			return;

		(void) rpma_conn_disconnect(clnt->conn);
		return;
	}

	/* proceed to the callback specific to the received event */
	switch (event) {
	case RPMA_CONN_ESTABLISHED:
		client_handle_is_ready(clnt);
		break;
	case RPMA_CONN_CLOSED:
	default:
		client_delete(clnt);
		break;
	}
}

/*
 * server_handle_incoming_client -- callback on endpoint's next incoming
 * connection
 *
 * Get the connection request. If there is not free slots reject it. Otherwise,
 * accept the incoming connection, get the event and completion file
 * descriptors, set O_NONBLOCK flag for both of them and add events to
 * the epoll.
 * If error will occur at any of the required steps the client is disconnected.
 */
void
server_handle_incoming_client(struct custom_event *ce)
{
	struct server_res *svr = (struct server_res *)ce->arg;

	/* receive an incoming connection request */
	struct rpma_conn_req *req = NULL;
	if (rpma_ep_next_conn_req(svr->ep, NULL, &req))
		return;

	/* if no free slot is available */
	struct client_res *clnt = NULL;
	if ((clnt = client_new(svr)) == NULL) {
		rpma_conn_req_delete(&req);
		return;
	}

	/* accept the connection request and obtain the connection object */
	if (rpma_conn_req_connect(&req, NULL, &clnt->conn))
		/*
		 * When rpma_conn_req_connect() fails the connection pointer
		 * remains unchanged (in this case it is NULL) so the server
		 * would choose the same client slot if another client will
		 * come. No additional cleanup needed.
		 */
		return;

	/* get the connection's main CQ */
	if (rpma_conn_get_cq(clnt->conn, &clnt->cq)) {
		/* an error occurred - disconnect */
		(void) rpma_conn_disconnect(clnt->conn);
		return;
	}

	if (client_add_to_epoll(clnt, svr->epoll))
		(void) rpma_conn_disconnect(clnt->conn);
}

int
main(int argc, char *argv[])
{
	/* validate parameters */
	if (argc < 3) {
		fprintf(stderr, USAGE_STR, argv[0]);
		exit(-1);
	}

	/* configure logging thresholds to see more details */
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD, RPMA_LOG_LEVEL_INFO);
	rpma_log_set_threshold(RPMA_LOG_THRESHOLD_AUX, RPMA_LOG_LEVEL_INFO);

	/* read common parameters */
	char *addr = argv[1];
	char *port = argv[2];
	int ret, ret2;

	/* RPMA resources - general */
	struct rpma_peer *peer = NULL;

	/* server resource */
	struct server_res svr = {0};

	/*
	 * lookup an ibv_context via the address and create a new peer using it
	 */
	ret = server_peer_via_address(addr, &peer);
	if (ret)
		return ret;

	/* initialize the server's structure */
	ret = server_init(&svr, peer);
	if (ret)
		goto err_peer_delete;

	/* start a listening endpoint at addr:port */
	ret = rpma_ep_listen(peer, addr, port, &svr.ep);
	if (ret)
		goto err_server_fini;

	/* get the endpoint's event file descriptor and add it to epoll */
	int ep_fd;
	ret = rpma_ep_get_fd(svr.ep, &ep_fd);
	if (ret)
		goto err_ep_shutdown;
	ret = epoll_add(svr.epoll, ep_fd, &svr, server_handle_incoming_client, &svr.ev_incoming);
	if (ret)
		goto err_ep_shutdown;

	(void) printf("Today I have got in touch with:\n");

	/* process epoll's events */
	struct epoll_event event = {0};
	struct custom_event *ce;
	while ((ret = epoll_wait(svr.epoll, &event, 1 /* # of events */, TIMEOUT_15S)) == 1) {
		ce = (struct custom_event *)event.data.ptr;
		ce->func(ce);
	}

	/* disconnect all remaining client's */
	for (int i = 0; i < CLIENT_MAX; ++i) {
		if (svr.clients[i].conn == NULL)
			continue;

		(void) rpma_conn_disconnect(svr.clients[i].conn);
		(void) rpma_conn_delete(&svr.clients[i].conn);
	}

	if (ret == 0)
		(void) fprintf(stderr, "Server timed out.\n");

err_ep_shutdown:
	/* shutdown the endpoint */
	ret2 = rpma_ep_shutdown(&svr.ep);
	if (!ret)
		ret = ret2;

err_server_fini:
	/* release the server's resources */
	ret2 = server_fini(&svr);
	if (!ret)
		ret = ret2;

err_peer_delete:
	/* delete the peer object */
	ret2 = rpma_peer_delete(&peer);
	if (!ret)
		ret = ret2;

	return ret;
}
