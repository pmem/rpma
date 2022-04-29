// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2021, Intel Corporation */
/* Copyright 2021-2022, Fujitsu */

/*
 * server_rpma_rw_common.c -- common server of rpma_read and rpma_write MT tests
 */

#include <inttypes.h>
#include <librpma.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "mtt.h"
#include "mtt_connect.h"
#include "mtt_common-epoll.h"
#include "rpma_rw_common.h"

#define CLIENT_MAX 64

struct client_res {
	/* RPMA resources */
	struct rpma_conn *conn;
	struct rpma_cq *cq;

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
	void *local_ptr;
	struct rpma_mr_local *mr_local_ptr;

	struct common_data data;
	struct rpma_conn_private_data pdata;

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
	size_t mr_local_size = MAX_STR_LEN;
	svr->local_ptr = mtt_malloc_aligned(mr_local_size, NULL);
	if (svr->local_ptr == NULL) {
		ret = -1;
		goto err_close;
	}

	/* fill the source memory with an initial content for read tests */
	memset(svr->local_ptr, 0, mr_local_size);
	memcpy(svr->local_ptr, STRING_TO_READ, LEN_STRING_TO_READ);

	/* register the memory */
	ret = rpma_mr_reg(peer, svr->local_ptr, mr_local_size,
			RPMA_MR_USAGE_READ_SRC | RPMA_MR_USAGE_WRITE_DST, &svr->mr_local_ptr);
	if (ret)
		goto err_free;


	/* get size of the memory region's descriptor */
	size_t mr_desc_size;
	ret = rpma_mr_get_descriptor_size(svr->mr_local_ptr, &mr_desc_size);
	if (ret)
		goto err_mr_dereg;

	svr->data.mr_desc_size = mr_desc_size;

	/* get the memory region's descriptor */
	ret = rpma_mr_get_descriptor(svr->mr_local_ptr, &svr->data.descriptors[0]);
	if (ret)
		goto err_mr_dereg;

	svr->pdata.ptr = &svr->data;
	svr->pdata.len = sizeof(struct common_data);

	return 0;

err_mr_dereg:
	(void) rpma_mr_dereg(&svr->mr_local_ptr);

err_free:
	free(svr->local_ptr);

err_close:
	close(svr->epoll);

	return ret;
}

/*
 * server_fini -- release server's resources
 */
void
server_fini(struct server_res *svr)
{
	/* deregister the memory region */
	(void) rpma_mr_dereg(&svr->mr_local_ptr);

	/* free the memory */
	free(svr->local_ptr);

	/* close the epoll */
	(void) close(svr->epoll);
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
	ret = epoll_add(epoll, fd, clnt, client_handle_connection_event,
			&clnt->ev_conn_event);
	if (ret)
		return ret;

	/* get the connection's completion fd and add it to epoll */
	ret = rpma_cq_get_fd(clnt->cq, &fd);
	if (ret) {
		epoll_delete(epoll, &clnt->ev_conn_event);
		return ret;
	}
	ret = epoll_add(epoll, fd, clnt, client_handle_completion,
			&clnt->ev_conn_cmpl);
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
 * client_handle_completion -- callback on completion is ready
 */
void
client_handle_completion(struct custom_event *ce)
{
}

/*
 * client_handle_is_ready -- callback on connection is established
 */
void
client_handle_is_ready(struct client_res *clnt)
{
}

/*
 * client_handle_connection_event -- callback on connection's next event
 */
void
client_handle_connection_event(struct custom_event *ce)
{
	struct client_res *clnt = (struct client_res *)ce->arg;

	/* get next connection's event */
	enum rpma_conn_event event;
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
	if (rpma_conn_req_connect(&req, &svr->pdata, &clnt->conn)) {
		(void) rpma_conn_req_delete(&req);
		/*
		 * When rpma_conn_req_connect() fails the connection pointer
		 * remains unchanged (in this case it is NULL) so the server
		 * would choose the same client slot if another client will
		 * come. No additional cleanup needed.
		 */
		return;
	}

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
server_main(char *addr, unsigned port)
{
	int ret;

	/* RPMA resources - general */
	struct rpma_peer *peer = NULL;

	/* server resource */
	struct server_res svr = {0};

	ret = mtt_server_listen(addr, port, &peer, &svr.ep);
	if (ret)
		return ret;

	/* initialize the server's structure */
	ret = server_init(&svr, peer);
	if (ret)
		goto err_server_shutdown;

	/* get the endpoint's event file descriptor and add it to epoll */
	int ep_fd;
	ret = rpma_ep_get_fd(svr.ep, &ep_fd);
	if (ret)
		goto err_server_fini;
	ret = epoll_add(svr.epoll, ep_fd, &svr, server_handle_incoming_client, &svr.ev_incoming);
	if (ret)
		goto err_server_fini;

	/* process epoll's events */
	struct epoll_event event;
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
		SERVER_ERR_MSG("Server timed out");

err_server_fini:
	/* release the server's resources */
	server_fini(&svr);

err_server_shutdown:
	mtt_server_shutdown(&peer, &svr.ep);

	return ret;
}
